#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include "symboltable.h"
#include "x86_64_asm.h"
#include "vector.h"
#include "panic.h"

static void write_indent(struct x86_64_render_params params);

static int opcode_needs_size_suffix(enum x86_64_opcode opcode);

static char const *size_suffix(enum x86_64_register_size size);

static void render_symbol_operand(
    struct symbol *symbol,
    struct x86_64_render_params params
);

static char const *directive_name(
    enum x86_64_directive_name directive_name,
    struct x86_64_render_params params
);

static void render_operand(
    struct x86_64_operand operand,
    struct x86_64_render_params params
);

static void render_register(
    enum x86_64_register reg,
    struct x86_64_render_params params
);

static void render_label(
    struct symbol *symbol,
    struct x86_64_render_params params
);

static void render_instruction(
    struct x86_64_instruction instruction,
    struct x86_64_render_params params
);

static void render_directive(
    struct x86_64_directive directive,
    struct x86_64_render_params params
);

enum x86_64_syntax x86_64_assembler_syntax(enum x86_64_assembler assembler)
{
    switch (assembler) {
        case X86_64_GAS: return X86_64_AT_T_SYNTAX;
    }

    panic("assembler %i syntax function not implemented", assembler);
}

struct x86_64_asm_unit x86_64_asm_unit_empty(void)
{
    struct x86_64_asm_unit unit;
    unit.length = 0;
    unit.statements = NULL;
    return unit;
}

void x86_64_asm_unit_push(
    struct x86_64_asm_unit *unit,
    struct x86_64_asm_stmt statement
)
{
    unit->statements = vector_push(
        unit->statements,
        sizeof(statement),
        &unit->length,
        &statement
    );
}

void x86_64_asm_unit_splice(
    struct x86_64_asm_unit *unit,
    size_t start,
    size_t end,
    struct x86_64_asm_stmt *replacement,
    size_t repl_length,
    struct x86_64_asm_stmt *old
)
{
    unit->statements = vector_splice(
        unit->statements,
        sizeof(*unit->statements),
        &unit->length,
        start,
        end,
        replacement,
        repl_length,
        old
    );
}

void x86_64_asm_unit_free(struct x86_64_asm_unit unit)
{
    free(unit.statements);
}

struct x86_64_asm_unit x86_64_asm_unit_vjoin(size_t count, va_list vargs)
{
    size_t i;
    struct x86_64_asm_unit argument;
    struct x86_64_asm_unit result = x86_64_asm_unit_empty();

    for (i = 0; i < count; i++) {
        argument = va_arg(vargs, struct x86_64_asm_unit);
        if (result.length == 0) {
            result = argument;
        } else if (argument.length > 0) {
            result.statements = vector_append(
                result.statements,
                argument.statements,
                sizeof(result.statements[0]),
                &result.length,
                argument.length
            );
        }
    }

    return result;
}

struct x86_64_asm_unit x86_64_asm_unit_join(size_t count, ...)
{
    struct x86_64_asm_unit unit;
    va_list vargs;

    va_start(vargs, count);
    unit = x86_64_asm_unit_vjoin(count, vargs);
    va_end(vargs);

    return unit;
}

void x86_64_render_asm_stmt(
    struct x86_64_asm_stmt statement,
    struct x86_64_render_params params
)
{
    switch (statement.tag) {
        case X86_64_INSTRUCTION:
            render_instruction(statement.data.instruction, params);
            return;
        case X86_64_LABEL:
            render_label(statement.data.label, params);
            return;
        case X86_64_DIRECTIVE:
            render_directive(statement.data.directive, params);
            return;
    }
    panic(
        "statement tag %i unimplemented for rendering",
        statement.tag
    );
}

void x86_64_render(
    struct x86_64_asm_unit unit,
    struct x86_64_render_params params
)
{
    size_t i;

    for (i = 0; i < unit.length; i++) {
        x86_64_render_asm_stmt(unit.statements[i], params);
    }
}

enum x86_64_register x86_64_make_register_b(enum x86_64_register reg)
{
    switch (reg) {
        case X86_64_AL:
        case X86_64_AX:
        case X86_64_EAX:
        case X86_64_RAX:
            return X86_64_AL;
        case X86_64_BL:
        case X86_64_BX:
        case X86_64_EBX:
        case X86_64_RBX:
            return X86_64_BL;
        case X86_64_CL:
        case X86_64_CX:
        case X86_64_ECX:
        case X86_64_RCX:
            return X86_64_CL;
        case X86_64_DL:
        case X86_64_DX:
        case X86_64_EDX:
        case X86_64_RDX:
            return X86_64_DL;
        case X86_64_SIL:
        case X86_64_SI:
        case X86_64_ESI:
        case X86_64_RSI:
            return X86_64_SIL;
        case X86_64_DIL:
        case X86_64_DI:
        case X86_64_EDI:
        case X86_64_RDI:
            return X86_64_DIL;
        case X86_64_SPL:
        case X86_64_SP:
        case X86_64_ESP:
        case X86_64_RSP:
            return X86_64_SPL;
        case X86_64_BPL:
        case X86_64_BP:
        case X86_64_EBP:
        case X86_64_RBP:
            return X86_64_BPL;
        case X86_64_R8B:
        case X86_64_R8W:
        case X86_64_R8D:
        case X86_64_R8:
            return X86_64_R8B;
        case X86_64_R9B:
        case X86_64_R9W:
        case X86_64_R9D:
        case X86_64_R9:
            return X86_64_R9B;
        case X86_64_R10B:
        case X86_64_R10W:
        case X86_64_R10D:
        case X86_64_R10:
            return X86_64_R10B;
        case X86_64_R11B:
        case X86_64_R11W:
        case X86_64_R11D:
        case X86_64_R11:
            return X86_64_R11B;
        case X86_64_R12B:
        case X86_64_R12W:
        case X86_64_R12D:
        case X86_64_R12:
            return X86_64_R12B;
        case X86_64_R13B:
        case X86_64_R13W:
        case X86_64_R13D:
        case X86_64_R13:
            return X86_64_R13B;
        case X86_64_R14B:
        case X86_64_R14W:
        case X86_64_R14D:
        case X86_64_R14:
            return X86_64_R14B;
        case X86_64_R15B:
        case X86_64_R15W:
        case X86_64_R15D:
        case X86_64_R15:
            return X86_64_R15B;
        case X86_64_RIP:
            panic("RIP register is not conversible");
        case X86_64_AH:
        case X86_64_BH:
        case X86_64_CH:
        case X86_64_DH:
            panic("high 8 bit registers are not conversible");
        case X86_64_XMM0:
        case X86_64_XMM1:
        case X86_64_XMM2:
        case X86_64_XMM3:
        case X86_64_XMM4:
        case X86_64_XMM5:
        case X86_64_XMM6:
        case X86_64_XMM7:
        case X86_64_XMM8:
        case X86_64_XMM9:
        case X86_64_XMM10:
        case X86_64_XMM11:
        case X86_64_XMM12:
        case X86_64_XMM13:
        case X86_64_XMM14:
        case X86_64_XMM15:
            panic("128 bit XMM registers are not conversible");
    }
    panic("register %i unimplemented for make b", reg);
}

enum x86_64_register x86_64_make_register_w(enum x86_64_register reg)
{
    switch (reg) {
        case X86_64_AL:
        case X86_64_AX:
        case X86_64_EAX:
        case X86_64_RAX:
            return X86_64_AX;
        case X86_64_BL:
        case X86_64_BX:
        case X86_64_EBX:
        case X86_64_RBX:
            return X86_64_BX;
        case X86_64_CL:
        case X86_64_CX:
        case X86_64_ECX:
        case X86_64_RCX:
            return X86_64_CX;
        case X86_64_DL:
        case X86_64_DX:
        case X86_64_EDX:
        case X86_64_RDX:
            return X86_64_DX;
        case X86_64_SIL:
        case X86_64_SI:
        case X86_64_ESI:
        case X86_64_RSI:
            return X86_64_SI;
        case X86_64_DIL:
        case X86_64_DI:
        case X86_64_EDI:
        case X86_64_RDI:
            return X86_64_DI;
        case X86_64_SPL:
        case X86_64_SP:
        case X86_64_ESP:
        case X86_64_RSP:
            return X86_64_SP;
        case X86_64_BPL:
        case X86_64_BP:
        case X86_64_EBP:
        case X86_64_RBP:
            return X86_64_BP;
        case X86_64_R8B:
        case X86_64_R8W:
        case X86_64_R8D:
        case X86_64_R8:
            return X86_64_R8W;
        case X86_64_R9B:
        case X86_64_R9W:
        case X86_64_R9D:
        case X86_64_R9:
            return X86_64_R9W;
        case X86_64_R10B:
        case X86_64_R10W:
        case X86_64_R10D:
        case X86_64_R10:
            return X86_64_R10W;
        case X86_64_R11B:
        case X86_64_R11W:
        case X86_64_R11D:
        case X86_64_R11:
            return X86_64_R11W;
        case X86_64_R12B:
        case X86_64_R12W:
        case X86_64_R12D:
        case X86_64_R12:
            return X86_64_R12W;
        case X86_64_R13B:
        case X86_64_R13W:
        case X86_64_R13D:
        case X86_64_R13:
            return X86_64_R13W;
        case X86_64_R14B:
        case X86_64_R14W:
        case X86_64_R14D:
        case X86_64_R14:
            return X86_64_R14W;
        case X86_64_R15B:
        case X86_64_R15W:
        case X86_64_R15D:
        case X86_64_R15:
            return X86_64_R15W;
        case X86_64_RIP:
            panic("RIP register is not conversible");
        case X86_64_AH:
        case X86_64_BH:
        case X86_64_CH:
        case X86_64_DH:
            panic( "high 8 bit registers are not conversible");
        case X86_64_XMM0:
        case X86_64_XMM1:
        case X86_64_XMM2:
        case X86_64_XMM3:
        case X86_64_XMM4:
        case X86_64_XMM5:
        case X86_64_XMM6:
        case X86_64_XMM7:
        case X86_64_XMM8:
        case X86_64_XMM9:
        case X86_64_XMM10:
        case X86_64_XMM11:
        case X86_64_XMM12:
        case X86_64_XMM13:
        case X86_64_XMM14:
        case X86_64_XMM15:
            panic("128 bit XMM registers are not conversible");
    }
    panic("register %i unimplemented for make w", reg);
}

enum x86_64_register x86_64_make_register_dw(enum x86_64_register reg)
{
    switch (reg) {
        case X86_64_AL:
        case X86_64_AX:
        case X86_64_EAX:
        case X86_64_RAX:
            return X86_64_EAX;
        case X86_64_BL:
        case X86_64_BX:
        case X86_64_EBX:
        case X86_64_RBX:
            return X86_64_EBX;
        case X86_64_CL:
        case X86_64_CX:
        case X86_64_ECX:
        case X86_64_RCX:
            return X86_64_ECX;
        case X86_64_DL:
        case X86_64_DX:
        case X86_64_EDX:
        case X86_64_RDX:
            return X86_64_EDX;
        case X86_64_SIL:
        case X86_64_SI:
        case X86_64_ESI:
        case X86_64_RSI:
            return X86_64_ESI;
        case X86_64_DIL:
        case X86_64_DI:
        case X86_64_EDI:
        case X86_64_RDI:
            return X86_64_EDI;
        case X86_64_SPL:
        case X86_64_SP:
        case X86_64_ESP:
        case X86_64_RSP:
            return X86_64_ESP;
        case X86_64_BPL:
        case X86_64_BP:
        case X86_64_EBP:
        case X86_64_RBP:
            return X86_64_EBP;
        case X86_64_R8B:
        case X86_64_R8W:
        case X86_64_R8D:
        case X86_64_R8:
            return X86_64_R8D;
        case X86_64_R9B:
        case X86_64_R9W:
        case X86_64_R9D:
        case X86_64_R9:
            return X86_64_R9D;
        case X86_64_R10B:
        case X86_64_R10W:
        case X86_64_R10D:
        case X86_64_R10:
            return X86_64_R10D;
        case X86_64_R11B:
        case X86_64_R11W:
        case X86_64_R11D:
        case X86_64_R11:
            return X86_64_R11D;
        case X86_64_R12B:
        case X86_64_R12W:
        case X86_64_R12D:
        case X86_64_R12:
            return X86_64_R12D;
        case X86_64_R13B:
        case X86_64_R13W:
        case X86_64_R13D:
        case X86_64_R13:
            return X86_64_R13D;
        case X86_64_R14B:
        case X86_64_R14W:
        case X86_64_R14D:
        case X86_64_R14:
            return X86_64_R14D;
        case X86_64_R15B:
        case X86_64_R15W:
        case X86_64_R15D:
        case X86_64_R15:
            return X86_64_R15D;
        case X86_64_RIP:
            panic("RIP register is not conversible");
        case X86_64_AH:
        case X86_64_BH:
        case X86_64_CH:
        case X86_64_DH:
            panic("high 8 bit registers are not conversible");
        case X86_64_XMM0:
        case X86_64_XMM1:
        case X86_64_XMM2:
        case X86_64_XMM3:
        case X86_64_XMM4:
        case X86_64_XMM5:
        case X86_64_XMM6:
        case X86_64_XMM7:
        case X86_64_XMM8:
        case X86_64_XMM9:
        case X86_64_XMM10:
        case X86_64_XMM11:
        case X86_64_XMM12:
        case X86_64_XMM13:
        case X86_64_XMM14:
        case X86_64_XMM15:
            panic("128 bit XMM registers are not conversible");
    }
    panic("register %i unimplemented for make dw", reg);
}

enum x86_64_register x86_64_make_register_qw(enum x86_64_register reg)
{
    switch (reg) {
        case X86_64_AL:
        case X86_64_AX:
        case X86_64_EAX:
        case X86_64_RAX:
            return X86_64_RAX;
        case X86_64_BL:
        case X86_64_BX:
        case X86_64_EBX:
        case X86_64_RBX:
            return X86_64_RBX;
        case X86_64_CL:
        case X86_64_CX:
        case X86_64_ECX:
        case X86_64_RCX:
            return X86_64_RCX;
        case X86_64_DL:
        case X86_64_DX:
        case X86_64_EDX:
        case X86_64_RDX:
            return X86_64_RDX;
        case X86_64_SIL:
        case X86_64_SI:
        case X86_64_ESI:
        case X86_64_RSI:
            return X86_64_RSI;
        case X86_64_DIL:
        case X86_64_DI:
        case X86_64_EDI:
        case X86_64_RDI:
            return X86_64_RDI;
        case X86_64_SPL:
        case X86_64_SP:
        case X86_64_ESP:
        case X86_64_RSP:
            return X86_64_RSP;
        case X86_64_BPL:
        case X86_64_BP:
        case X86_64_EBP:
        case X86_64_RBP:
            return X86_64_RBP;
        case X86_64_R8B:
        case X86_64_R8W:
        case X86_64_R8D:
        case X86_64_R8:
            return X86_64_R8;
        case X86_64_R9B:
        case X86_64_R9W:
        case X86_64_R9D:
        case X86_64_R9:
            return X86_64_R9;
        case X86_64_R10B:
        case X86_64_R10W:
        case X86_64_R10D:
        case X86_64_R10:
            return X86_64_R10;
        case X86_64_R11B:
        case X86_64_R11W:
        case X86_64_R11D:
        case X86_64_R11:
            return X86_64_R11;
        case X86_64_R12B:
        case X86_64_R12W:
        case X86_64_R12D:
        case X86_64_R12:
            return X86_64_R12;
        case X86_64_R13B:
        case X86_64_R13W:
        case X86_64_R13D:
        case X86_64_R13:
            return X86_64_R13;
        case X86_64_R14B:
        case X86_64_R14W:
        case X86_64_R14D:
        case X86_64_R14:
            return X86_64_R14;
        case X86_64_R15B:
        case X86_64_R15W:
        case X86_64_R15D:
        case X86_64_R15:
            return X86_64_R15;
        case X86_64_RIP:
            panic("RIP register is not conversible");
        case X86_64_AH:
        case X86_64_BH:
        case X86_64_CH:
        case X86_64_DH:
            panic("high 8 bit registers are not conversible");
        case X86_64_XMM0:
        case X86_64_XMM1:
        case X86_64_XMM2:
        case X86_64_XMM3:
        case X86_64_XMM4:
        case X86_64_XMM5:
        case X86_64_XMM6:
        case X86_64_XMM7:
        case X86_64_XMM8:
        case X86_64_XMM9:
        case X86_64_XMM10:
        case X86_64_XMM11:
        case X86_64_XMM12:
        case X86_64_XMM13:
        case X86_64_XMM14:
        case X86_64_XMM15:
            panic("128 bit XMM registers are not conversible");
    }
    panic("register %i unimplemented for make qw", reg);
}

static void render_register(
    enum x86_64_register reg,
    struct x86_64_render_params params
)
{
    switch (x86_64_assembler_syntax(params.assembler)) {
        case X86_64_AT_T_SYNTAX:
            fprintf(params.output, "%%%s", x86_64_register_name(reg));
            return;
    }
    panic(
        "assembler syntax %i's render register not implemented",
        x86_64_assembler_syntax(params.assembler)
    );
}

static void render_label(
    struct symbol *symbol,
    struct x86_64_render_params params
)
{
    switch (x86_64_assembler_syntax(params.assembler)) {
        case X86_64_AT_T_SYNTAX:
            fprintf(params.output, "\"%s\":\n", symbol->content);
            return;
    }
    panic(
        "assembler syntax %i's render label not implemented",
        x86_64_assembler_syntax(params.assembler)
    );
}

int x86_64_instruction_data_size(struct x86_64_instruction instruction)
{
    size_t i;
    int size, alt_size;
    int has_direct_operand;

    size = 8;
    has_direct_operand = 0;
    i = 0;

    while (i < instruction.operand_count && !has_direct_operand) {
        alt_size = X86_64_QWORD;
        switch (instruction.operands[i].tag) {
            case X86_64_OPERAND_DIRECT:
                has_direct_operand = 1;
                size = x86_64_register_size(
                    instruction.operands[i].data.direct
                );
                break;
            case X86_64_OPERAND_DISPLACED:
            case X86_64_OPERAND_DISPLACED_PLT:
                alt_size = x86_64_symbol_data_size(
                    instruction
                        .operands[i].data.displaced.displacement
                );
                break;
            case X86_64_OPERAND_INDEXED:
                alt_size = x86_64_symbol_data_size(
                    instruction
                        .operands[i].data.indexed.displacement
                );
                break;
            case X86_64_OPERAND_SCALED:
                alt_size = x86_64_symbol_data_size(
                    instruction
                        .operands[i].data.scaled.displacement
                );
                break;
            case X86_64_OPERAND_IMMEDIATE:
                break;
            case X86_64_OPERAND_ADDRESS:
            case X86_64_OPERAND_PLT:
                alt_size = x86_64_symbol_data_size(
                    instruction.operands[i].data.address
                );
                break;
            default:
                panic(
                    "operand tag %i's not considered in instruction data size",
                    instruction.operands[i].tag
                );
        }
        if (!has_direct_operand && alt_size < size) {
            size = alt_size;
        }
        i++;
    }

    return size;
}

static void render_instruction(
    struct x86_64_instruction instruction,
    struct x86_64_render_params params
)
{
    size_t i;
    int is_first = 1;
    int size;

    write_indent(params);

    switch (x86_64_assembler_syntax(params.assembler)) {
        case X86_64_AT_T_SYNTAX:
            fputs(x86_64_opcode_mnemonic(instruction.opcode), params.output);
            if (opcode_needs_size_suffix(instruction.opcode)) {
                size = x86_64_instruction_data_size(instruction);
                fputs(size_suffix(size), params.output);
            }
            for (i = instruction.operand_count; i > 0; i--) {
                if (is_first) {
                    fputc(' ', params.output);
                } else {
                    fputs(", ", params.output);
                }
                is_first = 0;
                render_operand(instruction.operands[i - 1], params);
            }
            break;
        default:
            panic(
                "assembler syntax %i's render instruction not implemented",
                x86_64_assembler_syntax(params.assembler)
            );
    }

    fputc('\n', params.output);
}

static void render_directive(
    struct x86_64_directive directive,
    struct x86_64_render_params params
)
{
    size_t i;
    int is_first = 1;

    write_indent(params);

    switch (params.assembler) {
        case X86_64_GAS:
            fputs(directive_name(directive.name, params), params.output);
            for (i = 0; i < directive.operand_count; i++) {
                if (is_first) {
                    fputc(' ', params.output);
                    is_first = 0;
                } else {
                    fputs(", ", params.output);
                }
                is_first = 0;
                fputs(directive.operands[i]->content, params.output);
            }
            break;
        default:
            panic(
                "assembler syntax %i's render directive not implemented",
                x86_64_assembler_syntax(params.assembler)
            );
    }

    fputc('\n', params.output);
}

static void render_operand(
    struct x86_64_operand operand,
    struct x86_64_render_params params
)
{
    switch (x86_64_assembler_syntax(params.assembler)) {
        case X86_64_AT_T_SYNTAX:
            switch (operand.tag) {
                case X86_64_OPERAND_DIRECT:
                    render_register(operand.data.direct, params);
                    break;
                case X86_64_OPERAND_INDEXED:
                    render_symbol_operand(
                        operand.data.indexed.displacement,
                        params
                    );
                    fputc('(', params.output);
                    render_register(operand.data.indexed.base, params);
                    fputs(", ",  params.output);
                    render_register(operand.data.indexed.index, params);
                    fprintf(
                        params.output,
                        ", %i)",
                        operand.data.indexed.scale
                    );
                    break;
                case X86_64_OPERAND_SCALED:
                    render_symbol_operand(
                        operand.data.scaled.displacement,
                        params
                    );
                    fputs("(, ",  params.output);
                    render_register(operand.data.scaled.index, params);
                    fprintf(
                        params.output,
                        ", %i)",
                        operand.data.scaled.scale
                    );
                    break;
                case X86_64_OPERAND_DISPLACED:
                    render_symbol_operand(
                        operand.data.displaced.displacement,
                        params
                    );
                    fputc('(', params.output);
                    render_register(operand.data.displaced.base, params);
                    fputc(')', params.output);
                    break;
                case X86_64_OPERAND_DISPLACED_PLT:
                    render_symbol_operand(
                        operand.data.displaced.displacement,
                        params
                    );
                    fputs("@PLT(", params.output);
                    render_register(operand.data.displaced.base, params);
                    fputc(')', params.output);
                    break;
                case X86_64_OPERAND_IMMEDIATE:
                    fputc('$', params.output);
                    render_symbol_operand(operand.data.immediate, params);
                    break;
                case X86_64_OPERAND_ADDRESS:
                    fprintf(
                        params.output,
                        "\"%s\"",
                        operand.data.address->content
                    );
                    break;
                case X86_64_OPERAND_PLT:
                    fprintf(
                        params.output,
                        "\"%s\"@PLT",
                        operand.data.address->content
                    );
                    break;
                default:
                    panic(
                        "operand tag %i's render not implemented",
                        operand.tag
                    );
            }
            break;
        default:
            panic(
                "assembler syntax %i's render operand not implemented",
                x86_64_assembler_syntax(params.assembler)
            );
    }
}

char const *x86_64_register_name(enum x86_64_register reg)
{
    switch (reg) {
        case X86_64_AH: return "ah";
        case X86_64_BH: return "bh";
        case X86_64_CH: return "ch";
        case X86_64_DH: return "dh";
        case X86_64_AL: return "al";
        case X86_64_BL: return "bl";
        case X86_64_CL: return "cl";
        case X86_64_DL: return "dl";
        case X86_64_SIL: return "sil";
        case X86_64_DIL: return "dil";
        case X86_64_SPL: return "spl";
        case X86_64_BPL: return "bpl";
        case X86_64_R8B: return "r8b";
        case X86_64_R9B: return "r9b";
        case X86_64_R10B: return "r10b";
        case X86_64_R11B: return "r11b";
        case X86_64_R12B: return "r12b";
        case X86_64_R13B: return "r13b";
        case X86_64_R14B: return "r14b";
        case X86_64_R15B: return "r15b";
        case X86_64_AX: return "ax";
        case X86_64_BX: return "bx";
        case X86_64_CX: return "cx";
        case X86_64_DX: return "dx";
        case X86_64_SI: return "si";
        case X86_64_DI: return "di";
        case X86_64_SP: return "sp";
        case X86_64_BP: return "bp";
        case X86_64_R8W: return "r8w";
        case X86_64_R9W: return "r9w";
        case X86_64_R10W: return "r10w";
        case X86_64_R11W: return "r11w";
        case X86_64_R12W: return "r12w";
        case X86_64_R13W: return "r13w";
        case X86_64_R14W: return "r14w";
        case X86_64_R15W: return "r15w";
        case X86_64_EAX: return "eax";
        case X86_64_EBX: return "ebx";
        case X86_64_ECX: return "ecx";
        case X86_64_EDX: return "edx";
        case X86_64_ESI: return "esi";
        case X86_64_EDI: return "edi";
        case X86_64_ESP: return "esp";
        case X86_64_EBP: return "ebp";
        case X86_64_R8D: return "r8d";
        case X86_64_R9D: return "r9d";
        case X86_64_R10D: return "r10d";
        case X86_64_R11D: return "r11d";
        case X86_64_R12D: return "r12d";
        case X86_64_R13D: return "r13d";
        case X86_64_R14D: return "r14d";
        case X86_64_R15D: return "r15d";
        case X86_64_RAX: return "rax";
        case X86_64_RBX: return "rbx";
        case X86_64_RCX: return "rcx";
        case X86_64_RDX: return "rdx";
        case X86_64_RSI: return "rsi";
        case X86_64_RDI: return "rdi";
        case X86_64_RSP: return "rsp";
        case X86_64_RBP: return "rbp";
        case X86_64_R8: return "r8";
        case X86_64_R9: return "r9";
        case X86_64_R10: return "r10";
        case X86_64_R11: return "r11";
        case X86_64_R12: return "r12";
        case X86_64_R13: return "r13";
        case X86_64_R14: return "r14";
        case X86_64_R15: return "r15";
        case X86_64_RIP: return "rip";
        case X86_64_XMM0: return "xmm0";
        case X86_64_XMM1: return "xmm1";
        case X86_64_XMM2: return "xmm2";
        case X86_64_XMM3: return "xmm3";
        case X86_64_XMM4: return "xmm4";
        case X86_64_XMM5: return "xmm5";
        case X86_64_XMM6: return "xmm6";
        case X86_64_XMM7: return "xmm7";
        case X86_64_XMM8: return "xmm8";
        case X86_64_XMM9: return "xmm9";
        case X86_64_XMM10: return "xmm10";
        case X86_64_XMM11: return "xmm11";
        case X86_64_XMM12: return "xmm12";
        case X86_64_XMM13: return "xmm13";
        case X86_64_XMM14: return "xmm14";
        case X86_64_XMM15: return "xmm15";
    }
    panic("unhandled register %i in rendering", reg);
}


char const *x86_64_opcode_mnemonic(enum x86_64_opcode opcode)
{
    switch (opcode) {
        case X86_64_MOV: return "mov";
        case X86_64_MOVABS: return "movabs";
        case X86_64_CMOVNS: return "cmovns";
        case X86_64_LEA: return "lea";
        case X86_64_NOT: return "not";
        case X86_64_SHL: return "shl";
        case X86_64_SAR: return "sar";
        case X86_64_AND: return "and";
        case X86_64_OR: return "or";
        case X86_64_XOR: return "xor";
        case X86_64_ADD: return "add";
        case X86_64_INC: return "inc";
        case X86_64_NEG: return "neg";
        case X86_64_SUB: return "sub";
        case X86_64_DEC: return "dec";
        case X86_64_CQO: return "cqo";
        case X86_64_IMUL: return "imul";
        case X86_64_IDIV: return "idiv";
        case X86_64_TEST: return "test";
        case X86_64_CMP: return "cmp";
        case X86_64_JMP: return "jmp";
        case X86_64_JZ: return "jz";
        case X86_64_JNZ: return "jnz";
        case X86_64_JC: return "jc";
        case X86_64_JNC: return "jnc";
        case X86_64_JP: return "jp";
        case X86_64_JNP: return "jnp";
        case X86_64_JL: return "jl";
        case X86_64_JG: return "jg";
        case X86_64_JLE: return "jle";
        case X86_64_JGE: return "jge";
        case X86_64_PUSH: return "push";
        case X86_64_POP: return "pop";
        case X86_64_CALL: return "call";
        case X86_64_RET: return "ret";
        case X86_64_SETZ: return "setz";
        case X86_64_SETNZ: return "setnz";
        case X86_64_SETP: return "setp";
        case X86_64_SETNP: return "setnp";
        case X86_64_SETC: return "setc";
        case X86_64_SETNC: return "setnc";
        case X86_64_SETG: return "setg";
        case X86_64_SETGE: return "setge";
        case X86_64_SETL: return "setl";
        case X86_64_SETLE: return "setle";
        case X86_64_MOVQ: return "movq";
        case X86_64_MOVSD: return "movsd";
        case X86_64_ADDSD: return "addsd";
        case X86_64_SUBSD: return "subsd";
        case X86_64_MULSD: return "mulsd";
        case X86_64_DIVSD: return "divsd";
        case X86_64_UCOMISD: return "ucomisd";
    }
    panic("unhandled opcode %i in rendering", opcode);
}

static char const *directive_name(
    enum x86_64_directive_name directive_name,
    struct x86_64_render_params params
)
{
    switch (params.assembler) {
        case X86_64_GAS:
            switch (directive_name) {
                case X86_64_DATA: return ".section .data";
                case X86_64_RODATA: return ".section .rodata";
                case X86_64_TEXT: return ".section .text";
                case X86_64_GLOBL: return ".globl";
                case X86_64_TYPE: return ".type";
                case X86_64_EXTERN: return ".extern";
                case X86_64_ASCII: return ".ascii";
                case X86_64_DOUBLE: return ".double";
                case X86_64_QUAD: return ".quad";
                case X86_64_ZERO: return ".zero";
                case X86_64_ALIGN: return ".align";
                default:
                    panic(
                        "directive name %i's render not implemented",
                        directive_name
                    );
            }
            break;
        default:
            panic(
                "assembler %i's render directive name not implemented",
                params.assembler
            );
    }
}

static void write_indent(struct x86_64_render_params params)
{
    size_t i;

    if (params.space_count < 0) {
        fputc('\t', params.output);
    } else {
        for (i = 0; i < params.space_count; i++) {
            fputc(' ', params.output);
        }
    }
}

static void render_symbol_operand(
    struct symbol *symbol,
    struct x86_64_render_params params
)
{
    switch (x86_64_assembler_syntax(params.assembler)) {
        case X86_64_AT_T_SYNTAX:
            switch (symbol->type) {
                case SYM_LIT_CHAR:
                    fprintf(
                        params.output,
                        "%i",
                        (int) (unsigned char) symbol->data.parsed_char
                    );
                    break;
                case SYM_LIT_INT:
                case SYM_LIT_FLOAT:
                    fputs(symbol->content, params.output);
                    break;
                case SYM_TMP_VAR:
                case SYM_SCALAR_VAR:
                case SYM_VECTOR_VAR:
                case SYM_STR_ADDR:
                case SYM_FLOAT_ADDR:
                case SYM_LABEL:
                    fprintf(params.output, "\"%s\"", symbol->content);
                    break;
                default:
                    panic("invalid symbol type %i for x86_64 operandn");
            }
            break;
        default:
            panic(
                "assembler syntax %i's render symbol operand not implemented",
                x86_64_assembler_syntax(params.assembler)
            );
    }
}

enum x86_64_register x86_64_make_register_of_size(
    enum x86_64_register_size size,
    enum x86_64_register reg
)
{
    switch (size) {
        case X86_64_BYTE: return x86_64_make_register_b(reg);
        case X86_64_WORD: return x86_64_make_register_w(reg);
        case X86_64_DWORD: return x86_64_make_register_dw(reg);
        case X86_64_QWORD: return x86_64_make_register_qw(reg);
        case X86_64_SSE: return reg;
    }
    panic("size %i is not supported for conversion", size);
}

enum x86_64_register_size x86_64_register_size(enum x86_64_register reg)
{
    switch (reg) {
        case X86_64_AH:
        case X86_64_BH:
        case X86_64_CH:
        case X86_64_DH:
        case X86_64_AL:
        case X86_64_BL:
        case X86_64_CL:
        case X86_64_DL:
        case X86_64_SIL:
        case X86_64_DIL:
        case X86_64_SPL:
        case X86_64_BPL:
        case X86_64_R8B:
        case X86_64_R9B:
        case X86_64_R10B:
        case X86_64_R11B:
        case X86_64_R12B:
        case X86_64_R13B:
        case X86_64_R14B:
        case X86_64_R15B:
            return X86_64_BYTE;
        case X86_64_AX:
        case X86_64_BX:
        case X86_64_CX:
        case X86_64_DX:
        case X86_64_SI:
        case X86_64_DI:
        case X86_64_SP:
        case X86_64_BP:
        case X86_64_R8W:
        case X86_64_R9W:
        case X86_64_R10W:
        case X86_64_R11W:
        case X86_64_R12W:
        case X86_64_R13W:
        case X86_64_R14W:
        case X86_64_R15W:
            return X86_64_WORD;
        case X86_64_EAX:
        case X86_64_EBX:
        case X86_64_ECX:
        case X86_64_EDX:
        case X86_64_ESI:
        case X86_64_EDI:
        case X86_64_ESP:
        case X86_64_EBP:
        case X86_64_R8D:
        case X86_64_R9D:
        case X86_64_R10D:
        case X86_64_R11D:
        case X86_64_R12D:
        case X86_64_R13D:
        case X86_64_R14D:
        case X86_64_R15D:
            return X86_64_DWORD;
        case X86_64_RAX:
        case X86_64_RBX:
        case X86_64_RCX:
        case X86_64_RDX:
        case X86_64_RSI:
        case X86_64_RDI:
        case X86_64_RSP:
        case X86_64_RBP:
        case X86_64_R8:
        case X86_64_R9:
        case X86_64_R10:
        case X86_64_R11:
        case X86_64_R12:
        case X86_64_R13:
        case X86_64_R14:
        case X86_64_R15:
        case X86_64_RIP:
            return X86_64_QWORD;
        case X86_64_XMM0:
        case X86_64_XMM1:
        case X86_64_XMM2:
        case X86_64_XMM3:
        case X86_64_XMM4:
        case X86_64_XMM5:
        case X86_64_XMM6:
        case X86_64_XMM7:
        case X86_64_XMM8:
        case X86_64_XMM9:
        case X86_64_XMM10:
        case X86_64_XMM11:
        case X86_64_XMM12:
        case X86_64_XMM13:
        case X86_64_XMM14:
        case X86_64_XMM15:
            return X86_64_SSE;
    }
    panic(
        "unimplemented register size routine for register %i",
        reg
    );
}

static int opcode_needs_size_suffix(enum x86_64_opcode opcode)
{
    switch (opcode) {
        case X86_64_MOV:
        case X86_64_MOVABS:
        case X86_64_XOR:
        case X86_64_NOT:
        case X86_64_AND:
        case X86_64_OR:
        case X86_64_LEA:
        case X86_64_ADD:
        case X86_64_NEG:
        case X86_64_SUB:
        case X86_64_INC:
        case X86_64_DEC:
        case X86_64_IMUL:
        case X86_64_IDIV:
        case X86_64_TEST:
        case X86_64_CMP:
        case X86_64_PUSH:
        case X86_64_POP:
        case X86_64_CALL:
        case X86_64_RET:
        case X86_64_SHL:
        case X86_64_SAR:
            return 1;
        case X86_64_CMOVNS:
        case X86_64_CQO:
        case X86_64_JMP:
        case X86_64_JZ:
        case X86_64_JNZ:
        case X86_64_JC:
        case X86_64_JNC:
        case X86_64_JP:
        case X86_64_JNP:
        case X86_64_JL:
        case X86_64_JG:
        case X86_64_JLE:
        case X86_64_JGE:
        case X86_64_SETC:
        case X86_64_SETNC:
        case X86_64_SETP:
        case X86_64_SETNP:
        case X86_64_SETZ:
        case X86_64_SETNZ:
        case X86_64_SETG:
        case X86_64_SETGE:
        case X86_64_SETL:
        case X86_64_SETLE:
        case X86_64_MOVQ:
        case X86_64_MOVSD:
        case X86_64_ADDSD:
        case X86_64_SUBSD:
        case X86_64_MULSD:
        case X86_64_DIVSD:
        case X86_64_UCOMISD:
            return 0;
    }
    panic("implementation of opcode %li's need size suffix required");
}

static char const *size_suffix(enum x86_64_register_size size)
{
    switch (size) {
        case X86_64_BYTE: return "b";
        case X86_64_WORD: return "w";
        case X86_64_DWORD: return "l";
        case X86_64_QWORD: return "q";
        case X86_64_SSE: return "";
    }
    panic("size %i suffix is not supported", size);
}

enum x86_64_register_size x86_64_symbol_reg_size(struct symbol const *symbol)
{
    switch (symbol->type) {
        case SYM_LIT_CHAR:
            return X86_64_BYTE;
        case SYM_LIT_INT:
        case SYM_LIT_STR:
        case SYM_LABEL:
        case SYM_FUNCTION:
        case SYM_EXTERNAL:
        case SYM_STR_ADDR:
            return X86_64_QWORD;
        case SYM_FLOAT_ADDR:
        case SYM_LIT_FLOAT:
            return X86_64_SSE;
        case SYM_SCALAR_VAR:
        case SYM_VECTOR_VAR:
        case SYM_TMP_VAR:
            switch (symbol->data.variable.type) {
                case DATATYPE_CARA:
                    return X86_64_BYTE;
                case DATATYPE_INTE:
                    return X86_64_QWORD;
                case DATATYPE_REAL:
                    return X86_64_SSE;
                default:
                    panic(
                        "datatype %i's register size not supported",
                        symbol->data.variable.type
                    );
            }
        default: 
            panic("symbol type %i does not have register size", symbol->type);
    }
}

int x86_64_symbol_data_size(struct symbol const *symbol)
{
    switch (symbol->type) {
        case SYM_LIT_CHAR:
            return 1;
        case SYM_LIT_FLOAT:
        case SYM_LIT_INT:
        case SYM_LIT_STR:
        case SYM_LABEL:
        case SYM_FUNCTION:
        case SYM_EXTERNAL:
        case SYM_STR_ADDR:
        case SYM_FLOAT_ADDR:
            return 8;
        case SYM_SCALAR_VAR:
        case SYM_VECTOR_VAR:
        case SYM_TMP_VAR:
            switch (symbol->data.variable.type) {
                case DATATYPE_CARA:
                    return 1;
                case DATATYPE_INTE:
                case DATATYPE_REAL:
                    return 8;
                default:
                    panic(
                        "datatype %i's data size not supported",
                        symbol->data.variable.type
                    );
            }
        default: 
            panic("symbol type %i does not have data size", symbol->type);
    }
}

x86_64_operand_flags_type x86_64_operand_flags(enum x86_64_opcode opcode)
{
    switch (opcode) {
        case X86_64_MOV:
        case X86_64_MOVABS:
        case X86_64_LEA:
        case X86_64_MOVQ:
        case X86_64_MOVSD:
            return X86_64_OPERAND_0_DEST | X86_64_OPERAND_1_SRC;
        case X86_64_CMOVNS:
            return X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_1_SRC
                | X86_64_OPERAND_EFLAGS_SRC;
        case X86_64_AND:
        case X86_64_OR:
        case X86_64_XOR:
        case X86_64_ADD:
        case X86_64_SUB:
        case X86_64_SHL:
        case X86_64_SAR:
            return X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_1_SRC
                | X86_64_OPERAND_EFLAGS_DEST;
        case X86_64_INC:
        case X86_64_DEC:
            return X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_1_SRC;
        case X86_64_ADDSD:
        case X86_64_SUBSD:
        case X86_64_MULSD:
        case X86_64_DIVSD:
            return X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_1_SRC;
        case X86_64_NOT:
            return X86_64_OPERAND_0_SRC | X86_64_OPERAND_0_DEST;
        case X86_64_NEG:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_EFLAGS_DEST;
        case X86_64_IMUL:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_RAX_SRC
                | X86_64_OPERAND_RAX_DEST
                | X86_64_OPERAND_RDX_DEST;
        case X86_64_IDIV:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_RAX_SRC
                | X86_64_OPERAND_RDX_SRC
                | X86_64_OPERAND_RAX_DEST
                | X86_64_OPERAND_RDX_DEST;
        case X86_64_CQO:
            return X86_64_OPERAND_RDX_DEST | X86_64_OPERAND_RAX_SRC;
        case X86_64_TEST:
        case X86_64_CMP:
        case X86_64_UCOMISD:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_1_SRC
                | X86_64_OPERAND_EFLAGS_DEST;
        case X86_64_JMP:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_RIP_SRC
                | X86_64_OPERAND_RIP_DEST;
        case X86_64_JZ:
        case X86_64_JNZ:
        case X86_64_JC:
        case X86_64_JNC:
        case X86_64_JP:
        case X86_64_JNP:
        case X86_64_JL:
        case X86_64_JG:
        case X86_64_JLE:
        case X86_64_JGE:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_EFLAGS_SRC
                | X86_64_OPERAND_RIP_SRC
                | X86_64_OPERAND_RIP_DEST;
        case X86_64_SETZ:
        case X86_64_SETNZ:
        case X86_64_SETP:
        case X86_64_SETNP:
        case X86_64_SETC:
        case X86_64_SETNC:
        case X86_64_SETG:
        case X86_64_SETGE:
        case X86_64_SETL:
        case X86_64_SETLE:
            return X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_EFLAGS_SRC;
        case X86_64_PUSH:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_RSP_SRC
                | X86_64_OPERAND_RSP_DEST;
        case X86_64_POP:
            return X86_64_OPERAND_0_DEST
                | X86_64_OPERAND_RSP_SRC
                | X86_64_OPERAND_RSP_DEST;
        case X86_64_CALL:
            return X86_64_OPERAND_0_SRC
                | X86_64_OPERAND_RIP_SRC
                | X86_64_OPERAND_RIP_DEST
                | X86_64_OPERAND_RSP_SRC
                | X86_64_OPERAND_RSP_DEST;
        case X86_64_RET:
            return X86_64_OPERAND_RIP_SRC
                | X86_64_OPERAND_RIP_DEST
                | X86_64_OPERAND_RSP_SRC
                | X86_64_OPERAND_RSP_DEST;
    }
    panic("opcode %i's flags not implemented", opcode);
}

int x86_64_operand_cmp(struct x86_64_operand left, struct x86_64_operand right)
{
    int cmp = left.tag - right.tag;
    if (cmp != 0) {
        return cmp;
    }
    switch (left.tag) {
        case X86_64_OPERAND_DIRECT:
            return left.data.direct - right.data.direct;
        case X86_64_OPERAND_INDEXED:
            cmp = left.data.indexed.base - right.data.indexed.base;
            if (cmp != 0) {
                return cmp;
            }
            cmp = left.data.indexed.index - right.data.indexed.index;
            if (cmp != 0) {
                return cmp;
            }
            return symbol_cmp(
                left.data.indexed.displacement,
                right.data.indexed.displacement
            );
        case X86_64_OPERAND_SCALED:
            cmp = left.data.scaled.index - right.data.scaled.index;
            if (cmp != 0) {
                return cmp;
            }
            return symbol_cmp(
                left.data.scaled.displacement,
                right.data.scaled.displacement
            );
        case X86_64_OPERAND_DISPLACED:
        case X86_64_OPERAND_DISPLACED_PLT:
            cmp = left.data.displaced.base - right.data.displaced.base;
            if (cmp != 0) {
                return cmp;
            }
            return symbol_cmp(
                left.data.displaced.displacement,
                right.data.displaced.displacement
            );
        case X86_64_OPERAND_IMMEDIATE:
            return symbol_cmp(left.data.immediate, right.data.immediate);
        case X86_64_OPERAND_ADDRESS:
            return symbol_cmp(left.data.address, right.data.address);
        case X86_64_OPERAND_PLT:
            return symbol_cmp(left.data.address, right.data.address);
    }
    panic("operand tag %i's equality not implemented", left.tag);
}

int x86_64_is_operand_memory(enum x86_64_operand_tag tag)
{
    switch (tag) {
        case X86_64_OPERAND_DIRECT:
        case X86_64_OPERAND_IMMEDIATE:
            return 0;
        case X86_64_OPERAND_INDEXED:
        case X86_64_OPERAND_DISPLACED:
        case X86_64_OPERAND_DISPLACED_PLT:
        case X86_64_OPERAND_ADDRESS:
        case X86_64_OPERAND_PLT:
            return 1;
        default:
            panic("operand tag %i's is-operand-memory not implemented", tag);
    }
}

int x86_64_reg_unsized_eq(
    enum x86_64_register reg_left,
    enum x86_64_register reg_right
)
{
    enum x86_64_register_size size_left, size_right;

    size_left = x86_64_register_size(reg_left);
    size_right = x86_64_register_size(reg_right);

    if (size_left == X86_64_SSE) {
        if (size_right == X86_64_SSE) {
            return reg_left == reg_right;
        }
        return 0;
    }
    if (size_right == X86_64_SSE) {
        return 0;
    }
    return reg_left == x86_64_make_register_of_size(size_left, reg_right);
}

int x86_64_operand_data_size(struct x86_64_operand operand)
{
    switch (operand.tag) {
        case X86_64_OPERAND_DIRECT:
            return x86_64_register_size(operand.data.direct);
        case X86_64_OPERAND_DISPLACED:
        case X86_64_OPERAND_DISPLACED_PLT:
            return x86_64_symbol_data_size(operand.data.displaced.displacement);
        case X86_64_OPERAND_INDEXED:
            return x86_64_symbol_data_size(operand.data.indexed.displacement);
        case X86_64_OPERAND_SCALED:
            return x86_64_symbol_data_size(operand.data.scaled.displacement);
        case X86_64_OPERAND_IMMEDIATE:
            if (
                operand.data.immediate->data.parsed_int <= INT8_MAX
                || operand.data.immediate->data.parsed_int >= INT8_MIN
            ) {
                return 1;
            }
            if (
                operand.data.immediate->data.parsed_int <= INT16_MAX
                || operand.data.immediate->data.parsed_int >= INT16_MIN
            ) {
                return 2;
            }
            if (
                operand.data.immediate->data.parsed_int <= INT32_MAX
                || operand.data.immediate->data.parsed_int >= INT32_MIN
            ) {
                return 4;
            }
            return 8;
            break;
        case X86_64_OPERAND_ADDRESS:
        case X86_64_OPERAND_PLT:
            return x86_64_symbol_data_size(operand.data.address);
    }
    panic(
        "operand tag %i's not considered in opereand data size",
        operand.tag
    );
}

int x86_64_operand_uses_reg(
    struct x86_64_operand operand,
    enum x86_64_register reg
)
{
    switch (operand.tag) {
        case X86_64_OPERAND_DIRECT:
            return operand.data.direct == reg;
        case X86_64_OPERAND_INDEXED:
            return operand.data.indexed.base == reg
                || operand.data.indexed.index == reg;
        case X86_64_OPERAND_SCALED:
            return operand.data.scaled.index == reg;
        case X86_64_OPERAND_DISPLACED:
        case X86_64_OPERAND_DISPLACED_PLT:
            return operand.data.displaced.base == reg;
        case X86_64_OPERAND_IMMEDIATE:
            return 0;
        case X86_64_OPERAND_ADDRESS:
            return 0;
        case X86_64_OPERAND_PLT:
            return 0;
    }

    panic("operand tag %i's register usage not implemented", operand.tag);
}
