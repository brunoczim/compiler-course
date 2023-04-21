#include "x86_64_opt.h"
#include "symboltable.h"
#include "panic.h"

enum deduplifier_status {
    DEDUP_NOT_STARTED,
    DEDUP_MOV_FIRST,
    DEDUP_MOV_LAST
};

struct deduplifier {
    struct x86_64_asm_unit *unit;
    size_t back_cursor;
    size_t first_selected;
    size_t front_cursor;
    enum deduplifier_status status;
};

static struct deduplifier deduplifier_init(struct x86_64_asm_unit *unit);

static int deduplifier_next_raw(
    struct deduplifier *dedup,
    size_t *curr_index
);

static int deduplifier_next(
    struct deduplifier *dedup,
    size_t *curr_index
);

static int deduplifier_transition(struct deduplifier *dedup);

static void deduplifier_remove(
    struct deduplifier *dedup,
    size_t start,
    size_t end
);

static void deduplifier_trans_not_started(
    struct deduplifier *dedup,
    size_t index
);

static void deduplifier_trans_started(
    struct deduplifier *dedup,
    size_t index
);

static void deduplifier_movs(struct x86_64_asm_unit *unit);

static void use_inc_decs(struct x86_64_asm_unit *unit);

void x86_64_opt(struct x86_64_asm_unit *unit, x86_64_opt_flags_type flags)
{
    if (flags & X86_64_OPT_DEDUP_MOVS) {
        deduplifier_movs(unit);
    }
    if (flags & X86_64_OPT_INC_DECS) {
        use_inc_decs(unit);
    }
}

static struct deduplifier deduplifier_init(struct x86_64_asm_unit *unit)
{
    struct deduplifier dedup;
    dedup.unit = unit;
    dedup.back_cursor = 0;
    dedup.first_selected = 0;
    dedup.front_cursor = 0;
    dedup.status = DEDUP_NOT_STARTED;
    return dedup;
}

static void deduplifier_movs(struct x86_64_asm_unit *unit)
{
    struct deduplifier dedup = deduplifier_init(unit);
    while (deduplifier_transition(&dedup)) {}
}

static int deduplifier_next_raw(
    struct deduplifier *dedup,
    size_t *curr_index
)
{
    if (dedup->front_cursor > dedup->unit->length) {
        if (dedup->back_cursor > dedup->unit->length) {
            return 0;
        }
        dedup->back_cursor++;
        dedup->front_cursor = dedup->back_cursor;
        dedup->status = DEDUP_NOT_STARTED;
    }
    if (curr_index != NULL) {
        *curr_index = dedup->front_cursor;
    }
    dedup->front_cursor++;
    return 1;
}

static int deduplifier_next(
    struct deduplifier *dedup,
    size_t *curr_index
)
{
    size_t local_curr;
    int retry;
    int success;
    x86_64_operand_flags_type operand_flags;

    do {
        success = deduplifier_next_raw(dedup, &local_curr);
        retry = 0;
        if (success) {
            retry = dedup->unit->statements[local_curr].tag !=
                X86_64_INSTRUCTION;
        }
        if (success && !retry) {
            operand_flags = x86_64_operand_flags(
                dedup->unit->statements[local_curr].data.instruction.opcode
            );
            retry = operand_flags & X86_64_OPERAND_RIP_DEST;
            if (retry) {
                dedup->status = DEDUP_NOT_STARTED;
            } else if (curr_index != NULL) {
                *curr_index = local_curr;
            }
        }
    } while (retry);

    return success;
}

static int deduplifier_transition(struct deduplifier *dedup)
{
    size_t index;
    if (!deduplifier_next(dedup, &index)) {
        return 0;
    }

    if (dedup->status == DEDUP_NOT_STARTED) {
        deduplifier_trans_not_started(dedup,  index);
    } else {
        deduplifier_trans_started(dedup,  index);
    }

    return 1;
}

static void deduplifier_trans_not_started(
    struct deduplifier *dedup,
    size_t index
)
{
    struct x86_64_instruction *instruction;
    x86_64_operand_flags_type operand_flags;

    instruction = &dedup->unit->statements[index].data.instruction;
    operand_flags = x86_64_operand_flags(instruction->opcode);

    if (
        operand_flags & X86_64_OPERAND_0_DEST
        && operand_flags & X86_64_OPERAND_1_SRC
        && instruction->opcode != X86_64_MOVQ
        && instruction->operand_count == 2
        && instruction->operands[0].tag == X86_64_OPERAND_DIRECT
    ) {
        if (instruction->opcode == X86_64_MOV) {
            dedup->status = DEDUP_MOV_FIRST;
        } else {
            dedup->status = DEDUP_MOV_LAST;
        }
        dedup->first_selected = index;
    }
}

static void deduplifier_trans_started(
    struct deduplifier *dedup,
    size_t index
)
{
    int first_rax, first_rdx, first_rsp, curr_rax, curr_rdx, curr_rsp;
    int uses_reg;
    size_t i;
    struct x86_64_instruction *first_instr;
    struct x86_64_instruction *curr_instr;
    x86_64_operand_flags_type first_flags, curr_flags;

    first_instr =
        &dedup->unit->statements[dedup->first_selected].data.instruction;
    curr_instr = &dedup->unit->statements[index].data.instruction;

    first_flags = x86_64_operand_flags(first_instr->opcode);
    curr_flags = x86_64_operand_flags(curr_instr->opcode);

    first_rax = first_flags & X86_64_OPERAND_RAX_DEST
        || first_instr->operands[0].data.direct == X86_64_RAX;

    curr_rax = curr_flags & X86_64_OPERAND_RAX_SRC
        || (
            curr_flags & X86_64_OPERAND_1_SRC
            && curr_instr->operands[1].tag == X86_64_OPERAND_DIRECT
            && curr_instr->operands[1].data.direct== X86_64_RAX
        );

    first_rdx = first_flags & X86_64_OPERAND_RDX_DEST
        || first_instr->operands[0].data.direct == X86_64_RDX;

    curr_rdx = curr_flags & X86_64_OPERAND_RDX_SRC
        || (
            curr_flags & X86_64_OPERAND_1_SRC
            && curr_instr->operands[1].tag == X86_64_OPERAND_DIRECT
            && curr_instr->operands[1].data.direct== X86_64_RDX
        );

    first_rsp = first_flags & X86_64_OPERAND_RSP_DEST
        || first_instr->operands[0].data.direct == X86_64_RSP;

    curr_rsp = curr_flags & X86_64_OPERAND_RSP_SRC
        || (
            curr_flags & X86_64_OPERAND_1_SRC
            && curr_instr->operands[1].tag == X86_64_OPERAND_DIRECT
            && curr_instr->operands[1].data.direct== X86_64_RSP
        );

    if (
        (first_rax && curr_rax)
        || (first_rdx && curr_rdx)
        || (first_rsp && curr_rsp)
    ) {
        dedup->status = DEDUP_NOT_STARTED;
    } else if (
        curr_flags & X86_64_OPERAND_0_DEST
        && curr_instr->operands[0].tag == X86_64_OPERAND_DIRECT
        && x86_64_reg_unsized_eq(
            first_instr->operands[0].data.direct,
            curr_instr->operands[0].data.direct
        )
    ) {
        dedup->status = DEDUP_NOT_STARTED;
    } else if (
        curr_flags & X86_64_OPERAND_1_DEST
        && curr_instr->operands[1].tag == X86_64_OPERAND_DIRECT
        && x86_64_reg_unsized_eq(
            first_instr->operands[0].data.direct,
            curr_instr->operands[1].data.direct
        )
    ) {
        dedup->status = DEDUP_NOT_STARTED;
    } else if (
        curr_flags & X86_64_OPERAND_0_DEST
        && curr_flags & X86_64_OPERAND_1_SRC
        && curr_instr->opcode != X86_64_MOVQ
        && curr_instr->operand_count == 2
        && curr_instr->operands[1].tag == X86_64_OPERAND_DIRECT
        && first_instr->operands[0].data.direct
            == curr_instr->operands[1].data.direct
        && (
            !x86_64_is_operand_memory(first_instr->operands[1].tag)
            || !x86_64_is_operand_memory(curr_instr->operands[0].tag)
        )
    ) {
        if ((first_flags & X86_64_OPERAND_0_SRC) != 0) {
            dedup->status = DEDUP_NOT_STARTED;
        } else if (
            first_instr->opcode == X86_64_MOVABS
            && !x86_64_is_operand_memory(curr_instr->operands[0].tag)
        ) {
            dedup->status = DEDUP_NOT_STARTED;
        } else if (
            first_instr->operands[1].tag == X86_64_OPERAND_IMMEDIATE
            && x86_64_operand_data_size(first_instr->operands[1])
                >
                x86_64_operand_data_size(curr_instr->operands[0])
        ) {
            dedup->status = DEDUP_NOT_STARTED;
        }
        switch (dedup->status) {
            case DEDUP_MOV_FIRST:
                curr_instr->operands[1] = first_instr->operands[1];
                deduplifier_remove(
                    dedup,
                    dedup->first_selected,
                    dedup->first_selected + 1
                );
                dedup->status = DEDUP_NOT_STARTED;
                break;
            case DEDUP_MOV_LAST:
                if (curr_instr->opcode == X86_64_MOV) {
                    curr_instr->opcode = first_instr->opcode;
                    curr_instr->operands[1] = first_instr->operands[1];
                    deduplifier_remove(
                        dedup,
                        dedup->first_selected,
                        dedup->first_selected + 1
                    );
                    dedup->status = DEDUP_NOT_STARTED;
                }
                break;
             case DEDUP_NOT_STARTED:
                break;
        }
    }

    if (
        (
            curr_flags & X86_64_OPERAND_0_DEST
            &&
            x86_64_operand_cmp(
                first_instr->operands[0],
                curr_instr->operands[0]
            ) == 0
        )
        ||
        (
            curr_flags & X86_64_OPERAND_1_DEST
            &&
            x86_64_operand_cmp(
                first_instr->operands[0],
                curr_instr->operands[1]
            ) == 0
        )
    ) {
        dedup->status = DEDUP_NOT_STARTED;
    } else {
        uses_reg = 0;
        i = 0;
        while (!uses_reg && i < curr_instr->operand_count) {
            uses_reg = x86_64_operand_uses_reg(
                curr_instr->operands[i],
                first_instr->operands[0].data.direct
            );
            i++;
        }
        if (uses_reg) {
            dedup->status = DEDUP_NOT_STARTED;
        }
    }
}

static void deduplifier_remove(
    struct deduplifier *dedup,
    size_t start,
    size_t end 
)
{
    x86_64_asm_unit_splice(
        dedup->unit,
        start,
        end,
        NULL,
        0,
        NULL
    );

    if (dedup->front_cursor > start) {
        dedup->front_cursor -= end - start;
    }
}

static void use_inc_decs(struct x86_64_asm_unit *unit)
{
    size_t i;
    struct x86_64_instruction *instruction;
    int immediate;

    for (i = 0; i < unit->length; i++) {
        if (unit->statements[i].tag == X86_64_INSTRUCTION) {
            instruction = &unit->statements[i].data.instruction;
            switch (instruction->opcode) {
                case X86_64_ADD:
                case X86_64_SUB:
                    if (
                        instruction->operands[1].tag == X86_64_OPERAND_IMMEDIATE
                        && (
                            instruction
                                ->operands[1]
                                .data.immediate->data.parsed_int == 1
                            || instruction
                                ->operands[1]
                                .data.immediate->data.parsed_int == -1
                        )
                    ) {
                        instruction->operand_count = 1;
                        immediate =
                            instruction
                            ->operands[1].data.immediate->data.parsed_int;
                        if (
                            (
                                instruction->opcode == X86_64_ADD
                                && immediate == 1
                            )
                            || (
                                instruction->opcode == X86_64_SUB
                                && immediate == -1
                            )
                        ) {
                            instruction->opcode = X86_64_INC;
                        } else {
                            instruction->opcode = X86_64_DEC;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
