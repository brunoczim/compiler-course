#include "tac.h"
#include "symboltable.h"
#include "alloc.h"
#include "panic.h"
#include "vector.h"
#include <stdlib.h>

struct id_computer {
    struct tac_node *current;
    int inside_function;
    int inside_block;
    tac_function_id_type function_id;
    tac_block_id_type block_id;
    tac_local_id_type local_id;
};

static void free_node(struct tac_node *node);

static struct id_computer id_computer_init(struct tac *tac);

static struct tac_node *compute_ids_next(struct id_computer *computer);

static void compute_ids(struct tac *tac);

static void compute_local_values(struct tac *tac);

static int opcode_needs_indent(enum tac_opcode opcode);

static int opcode_needs_colon(enum tac_opcode opcode);

static void write_indent(struct tac_render_params params);

char const *tac_opcode_mnemonic(enum tac_opcode opcode)
{
    switch (opcode) {
        case TAC_MOVE: return "move";
        case TAC_MOVI: return "movi";
        case TAC_MOVV: return "movv";
        case TAC_ADD: return "add";
        case TAC_SUB: return "sub";
        case TAC_MUL: return "mul";
        case TAC_DIV: return "div";
        case TAC_LT: return "lt";
        case TAC_GT: return "gt";
        case TAC_LE: return "le";
        case TAC_GE: return "ge";
        case TAC_EQ: return "eq";
        case TAC_NE: return "ne";
        case TAC_SHMUL: return "shmul";
        case TAC_SHDIV: return "shdiv";
        case TAC_AND: return "and";
        case TAC_OR: return "or";
        case TAC_NOT: return "not";
        case TAC_LABEL: return "";
        case TAC_DEFS: return ".defs";
        case TAC_DEFV: return ".defv";
        case TAC_DEFP: return ".defp";
        case TAC_BEGINVEC: return ".beginvec";
        case TAC_ENDVEC: return ".endvec";
        case TAC_BEGINFUN: return ".beginfun";
        case TAC_ENDFUN: return ".endfun";
        case TAC_IFZ: return "ifz";
        case TAC_JUMP: return "jump";
        case TAC_CALL: return "call";
        case TAC_ARG: return "arg";
        case TAC_RET: return "ret";
        case TAC_PRINT: return "print";
        case TAC_READ: return "read";
    }
    panic("TAC opcode %i's mnemonic not implemented", opcode);
}

char const *tac_opcode_raw_mnemonic(enum tac_opcode opcode)
{
    switch (opcode) {
        case TAC_MOVE: return "TAC_MOVE";
        case TAC_MOVI: return "TAC_MOVI";
        case TAC_MOVV: return "TAC_MOVV";
        case TAC_ADD: return "TAC_ADD";
        case TAC_SUB: return "TAC_SUB";
        case TAC_MUL: return "TAC_MUL";
        case TAC_DIV: return "TAC_DIV";
        case TAC_LT: return "TAC_LT";
        case TAC_GT: return "TAC_GT";
        case TAC_LE: return "TAC_LE";
        case TAC_GE: return "TAC_GE";
        case TAC_EQ: return "TAC_EQ";
        case TAC_NE: return "TAC_NE";
        case TAC_SHMUL: return "TAC_SHMUL";
        case TAC_SHDIV: return "TAC_SHDIV";
        case TAC_AND: return "TAC_AND";
        case TAC_OR: return "TAC_OR";
        case TAC_NOT: return "TAC_NOT";
        case TAC_LABEL: return "TAC_LABEL";
        case TAC_DEFS: return "TAC_DEFS";
        case TAC_DEFV: return "TAC_DEFV";
        case TAC_DEFP: return "TAC_DEFP";
        case TAC_BEGINVEC: return "TAC_BEGINVEC";
        case TAC_ENDVEC: return "TAC_ENDVEC";
        case TAC_BEGINFUN: return "TAC_BEGINFUN";
        case TAC_ENDFUN: return "TAC_ENDFUN";
        case TAC_IFZ: return "TAC_IFZ";
        case TAC_JUMP: return "TAC_JUMP";
        case TAC_CALL: return "TAC_CALL";
        case TAC_ARG: return "TAC_ARG";
        case TAC_RET: return "TAC_RET";
        case TAC_PRINT: return "TAC_PRINT";
        case TAC_READ: return "TAC_READ";
    }
    panic("TAC opcode %i's raw debug mnemonic not implemented", opcode);
}

void tac_instruction_print(
    struct tac_instruction instruction,
    struct tac_render_params params
)
{
    if (opcode_needs_indent(instruction.opcode)) {
        write_indent(params);
    }
    fputs(tac_opcode_mnemonic(instruction.opcode), params.output);

    if (instruction.dest != NULL) {
        fprintf(params.output, " %s", instruction.dest->content);
    }

    if (instruction.srcs[0] != NULL) {
        if (instruction.dest != NULL) {
            fputc(',', params.output);
        }
        fprintf(params.output, " %s", instruction.srcs[0]->content);
    }

    if (instruction.srcs[1] != NULL) {
        if (instruction.dest != NULL || instruction.srcs[0] != NULL) {
            fputc(',', params.output);
        }
        fprintf(params.output, " %s", instruction.srcs[1]->content);
    }

    if (opcode_needs_colon(instruction.opcode)) {
        fputc(':', params.output);
    }
    fputc('\n', params.output);
}

void tac_instruction_raw_print(struct tac_instruction instruction)
{
    printf(
        "TAC(%s, %s, %s, %s)\n",
        tac_opcode_raw_mnemonic(instruction.opcode),
        instruction.dest != NULL ? instruction.dest->content : "@0",
        instruction.srcs[0] != NULL ? instruction.srcs[0]->content : "@0",
        instruction.srcs[1] != NULL ? instruction.srcs[1]->content : "@0"
    );
}

void tac_print(struct tac tac, struct tac_render_params params)
{
    struct tac_node *node;

    for (node = tac.first; node != NULL; node = node->next) {
        tac_instruction_print(
            node->instruction,
            params
        );
    }
}

void tac_raw_print(struct tac tac)
{
    struct tac_node *node;

    for (node = tac.first; node != NULL; node = node->next) {
        tac_instruction_raw_print(node->instruction);
    }
}

struct tac tac_empty(void)
{
    struct tac tac;
    tac.first = NULL;
    tac.last = NULL;
    tac.locality_computed = 0;
    return tac;
}

struct tac tac_singleton(struct tac_instruction instruction)
{
    struct tac tac = tac_empty();
    tac_append(&tac, instruction);
    return tac;
}

void tac_prepend(struct tac *tac, struct tac_instruction instruction)
{
    struct tac_node *node = tac_create_node(instruction);
    node->next = tac->first;
    if (tac->first == NULL) {
        tac->last = node;
    } else {
        tac->first->prev = node;
    }
    tac->first = node;
}

void tac_append(struct tac *tac, struct tac_instruction instruction)
{
    struct tac_node *node = tac_create_node(instruction);
    node->prev = tac->last;
    if (tac->last == NULL) {
        tac->first = node;
    } else {
        tac->last->next = node;
    }
    tac->last = node;
}

int tac_pop(struct tac *tac, struct tac_instruction *output)
{
    struct tac_node *prev;

    if (tac->last != NULL) {
        if (output != NULL) {
            *output = tac->last->instruction;
        }
        prev = tac->last->prev;
        free_node(tac->last);
        tac->last = prev;
        if (prev == NULL) {
            tac->first = NULL;
        }
       
        return 1;
    }

    return 0;
}

struct tac tac_vjoin(size_t count, va_list vargs)
{
    size_t i;
    struct tac argument;
    struct tac result = tac_empty();

    for (i = 0; i < count; i++) {
        argument = va_arg(vargs, struct tac);
        if (result.last == NULL) {
            result = argument;
        } else if (argument.first != NULL) {
            result.last->next = argument.first;
            argument.first->prev = result.last;
            result.last = argument.last;
        }
    }

    return result;
}

struct tac tac_join(size_t count, ...)
{
    struct tac tac;
    va_list vargs;

    va_start(vargs, count);
    tac = tac_vjoin(count, vargs);
    va_end(vargs);

    return tac;
}

void tac_free(struct tac tac)
{
    struct tac_node *node;
    struct tac_node *node_next;

    for (node = tac.first; node != NULL; node = node_next) {
        node_next = node->next;
        free_node(node);
        node = node_next;
    }
}

static int opcode_needs_indent(enum tac_opcode opcode)
{
    switch (opcode) {
        case TAC_LABEL:
        case TAC_BEGINFUN:
        case TAC_ENDFUN:
        case TAC_BEGINVEC:
        case TAC_ENDVEC:
            return 0;
        default:
            return 1;
    }
}

static int opcode_needs_colon(enum tac_opcode opcode)
{
    switch (opcode) {
        case TAC_LABEL:
        case TAC_BEGINFUN:
        case TAC_BEGINVEC:
            return 1;
        default:
            return 0;
    }
}

static void write_indent(struct tac_render_params params)
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

int tac_is_block_boundary(enum tac_opcode opcode)
{
    switch (opcode) {
        case TAC_MOVE:
        case TAC_MOVI:
        case TAC_MOVV:
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_DIV:
        case TAC_LT:
        case TAC_GT:
        case TAC_LE:
        case TAC_GE:
        case TAC_EQ:
        case TAC_NE:
        case TAC_SHMUL:
        case TAC_SHDIV:
        case TAC_AND:
        case TAC_OR:
        case TAC_NOT:
        case TAC_ARG:
        case TAC_PRINT:
        case TAC_READ:
            return 0;
        case TAC_LABEL:
        case TAC_DEFS:
        case TAC_DEFV:
        case TAC_DEFP:
        case TAC_BEGINVEC:
        case TAC_ENDVEC:
        case TAC_BEGINFUN:
        case TAC_ENDFUN:
        case TAC_IFZ:
        case TAC_JUMP:
        case TAC_CALL:
        case TAC_RET:
            return 1;
    }
    panic("TAC opcode %i's boundary test not implemented", opcode);
}

struct tac_node *tac_create_node(struct tac_instruction instruction)
{
    struct tac_node *node;
    node = aborting_malloc(sizeof(*node));
    node->instruction = instruction;
    node->prev = NULL;
    node->next = NULL;
    node->function_id = TAC_ID_UNKNOWN;
    node->block_id = TAC_ID_UNKNOWN;
    node->local_id = TAC_ID_UNKNOWN;
    node->starting_local_value = NULL;
    node->owns_starting_local_value = 1;
    node->ending_local_values.length = 0;
    node->ending_local_values.ordered_values = NULL;
    return node;
}

struct tac_local_value *tac_create_local_value(void)
{
    struct tac_local_value *local_value;

    local_value = aborting_malloc(sizeof(*local_value));

    local_value->block_id = TAC_ID_UNKNOWN;
    local_value->block_id = TAC_ID_UNKNOWN;
    local_value->start_id = TAC_ID_UNKNOWN;
    local_value->end_id = TAC_ID_UNKNOWN;
    local_value->start_node = NULL;
    local_value->end_node = NULL;
    local_value->old_symbol = NULL;
    local_value->symbol_in_use = NULL;
    local_value->symbol_offered = NULL;

    return local_value;
}

void tac_compute_locality(struct tac *tac)
{
    if (!tac->locality_computed) {
        compute_ids(tac);
        compute_local_values(tac);
        tac->locality_computed = 1;
    }
}

static struct tac_node *compute_ids_next(struct id_computer *computer)
{
    struct tac_node *node = NULL;
    if (computer->current != NULL) {
        node = computer->current;

        if (tac_is_block_boundary(node->instruction.opcode)) {
            if (computer->inside_function) {
                node->function_id = computer->function_id;
            } else {
                node->function_id = TAC_ID_BOUNDARY;
            }
            node->block_id = TAC_ID_BOUNDARY;
            node->local_id = TAC_ID_BOUNDARY;
            if (node->instruction.opcode == TAC_BEGINFUN) {
                computer->inside_function = 1;
                computer->function_id++;
                node->block_id = 0;
            } else if (node->instruction.opcode == TAC_ENDFUN) {
                computer->inside_function = 0;
            }
            computer->inside_block = 0;
        } else {
            if (!computer->inside_block) {
                computer->inside_block = 1;
                node->block_id = computer->block_id;
                computer->block_id++;
            }
            node->function_id = computer->function_id;
            node->block_id = computer->block_id;
            node->local_id = computer->local_id;
            computer->local_id++;
        }
        computer->current = computer->current->next;
    }
    return node;
}

static struct id_computer id_computer_init(struct tac *tac)
{
    struct id_computer id_computer;
    id_computer.current = tac->first;
    id_computer.function_id = TAC_ID_BOUNDARY;
    id_computer.block_id = TAC_ID_BOUNDARY;
    return id_computer;
}

static void compute_ids(struct tac *tac)
{
    struct id_computer id_computer = id_computer_init(tac);
    while (compute_ids_next(&id_computer) != NULL) {}
}

static void compute_local_values(struct tac *tac)
{
    struct tac_local_value *local_value;
    struct tac_node *function_start;
    struct tac_node *target;
    struct tac_node *current;

    function_start = tac_find_function_start(tac->first);

    while (function_start != NULL) {
        target = function_start;
        while (target->instruction.opcode != TAC_ENDFUN) {
            if (
                target->instruction.dest != NULL
                && target->instruction.dest->type == SYM_TMP_VAR
            ) {
                local_value = tac_create_local_value();
                tac_draft_local_value_start(target, local_value);
                tac_draft_local_value_end(target, local_value);
                current = target->next;

                while (
                    current->instruction.opcode != TAC_ENDFUN
                    && local_value->start_id != TAC_ID_NON_LOCAL
                ) {
                    if (
                        local_value->symbol_in_use == current->instruction.dest
                        || local_value->symbol_in_use
                            == current->instruction.srcs[0]
                        || local_value->symbol_in_use
                            == current->instruction.srcs[1]
                    ) {
                        if (local_value->block_id == current->block_id) {
                            tac_draft_local_value_end(current, local_value);
                        } else if (current->block_id <= TAC_ID_MAX_OK) {
                            tac_mark_non_local_value(local_value);
                        }
                    }
                    current = current->next;
                }

                current = function_start;
                while (
                    current != target
                    && local_value->start_id != TAC_ID_NON_LOCAL
                ) {
                    if (
                        local_value->symbol_in_use == current->instruction.dest
                        || local_value->symbol_in_use
                            == current->instruction.srcs[0]
                        || local_value->symbol_in_use
                            == current->instruction.srcs[1]
                    ) {
                        if (local_value->block_id != current->block_id) {
                            tac_mark_non_local_value(local_value);
                        }
                    }
                    current = current->next;
                }

                if (local_value->start_id == TAC_ID_NON_LOCAL) {
                    free(local_value);
                } else {
                    tac_confirm_local_value_start(local_value);
                    tac_confirm_local_value_end(local_value);
                }
            }

            target = target->next;
        }
        function_start = tac_find_function_start(target);
    }
}

struct tac_node *tac_find_next_block_start(struct tac_node *from)
{
    struct tac_node *block_start = from;

    while (
        block_start != NULL
        && block_start->block_id != TAC_ID_UNKNOWN
        && block_start->instruction.opcode == TAC_ENDFUN
    ) {
        block_start = block_start->next;
    }

    return block_start;
}

struct tac_node *tac_find_function_start(struct tac_node *from)
{
    struct tac_node *function_start = from;

    while (
        function_start != NULL
        && function_start->instruction.opcode != TAC_BEGINFUN
    ) {
        function_start = function_start->next;
    }

    return function_start;
}

void tac_draft_local_value_start(
    struct tac_node *target,
    struct tac_local_value *local_value
)
{
    local_value->function_id = target->function_id;
    local_value->block_id = target->block_id;
    local_value->start_node = target;
    local_value->start_id = target->local_id;
    local_value->old_symbol = target->instruction.dest;
    local_value->symbol_in_use = target->instruction.dest;
    local_value->symbol_offered = target->instruction.dest;
}

void tac_draft_local_value_end(
    struct tac_node *target,

    struct tac_local_value *local_value
)
{
    local_value->end_node = target;
    local_value->end_id = target->local_id;
}

void tac_confirm_local_value_start(struct tac_local_value *local_value)
{
    if (local_value->start_node->starting_local_value != NULL) {
        local_value->start_node->starting_local_value = 0;
    }
    local_value->start_node->starting_local_value = local_value;
}

void tac_confirm_local_value_end(struct tac_local_value *local_value)
{
    tac_insert_local_value(
        &local_value->end_node->ending_local_values,
        local_value,
        NULL
    );
}

void tac_mark_non_local_value(struct tac_local_value *local_value)
{
    local_value->start_id = TAC_ID_NON_LOCAL;
    local_value->end_id = TAC_ID_NON_LOCAL;
    local_value->start_node = NULL;
    local_value->end_node = NULL;
}

int tac_binsearch_local_value(
    struct tac_local_value_set set,
    tac_block_id_type block_id,
    tac_local_id_type start_id,
    size_t *index_out
)
{
    int cmp;
    size_t low, high;
    low = 0;
    high = set.length;

    while (low < high) {
        *index_out = low + (high - low) / 2;
        cmp = tac_local_value_cmp_block_and_start(
            set.ordered_values[*index_out],
            block_id,
            start_id
        );

        if (cmp == 0) {
            return 1;
        }

        if (cmp < 0) {
            high = *index_out;
        } else {
            low = *index_out + 1;
        }
    }

    *index_out = low;
    return 0;
}

int tac_insert_local_value(
    struct tac_local_value_set *set,
    struct tac_local_value *local_value,
    size_t *index_out
)
{
    size_t index;
    if (index_out == NULL) {
        index_out = &index;
    }

    if (tac_binsearch_local_value(
       *set,
       local_value->block_id,
       local_value->start_id,
       index_out
    )) {
        return 0;
    }

    set->ordered_values = vector_splice(
        set->ordered_values,
        sizeof(*set->ordered_values),
        &set->length,
        *index_out,
        *index_out,
        &local_value,
        1,
        NULL
    );
    return 1;
}

int tac_remove_local_value(
    struct tac_local_value_set *set,
    struct tac_local_value *local_value,
    size_t *index_out
)
{
    size_t index;
    if (index_out == NULL) {
        index_out = &index;
    }

    if (tac_binsearch_local_value(
        *set,
        local_value->block_id,
        local_value->start_id,
        index_out
    )) {
        set->ordered_values = vector_splice(
            set->ordered_values,
            sizeof(*set->ordered_values),
            &set->length,
            *index_out,
            *index_out + 1,
            NULL,
            0,
            NULL
        );
        return 1;
    }

    return 0;
}

static void free_node(struct tac_node *node)
{
    if (node->owns_starting_local_value) {
        free(node->starting_local_value);
    }
    free(node->ending_local_values.ordered_values);
    free(node);
}

void tac_debug_locality(struct tac tac)
{
    size_t i;
    struct tac_node *node;
    struct tac_render_params params;

    params.space_count = 4;
    params.output = stdout;

    for (node = tac.first; node != NULL; node = node->next) {
        tac_instruction_print(
            node->instruction,
            params
        );
        for (i = 0; i < node->ending_local_values.length; i++) {
            printf(
                "end %s\n",
                node->ending_local_values.ordered_values[i]
                    ->symbol_in_use->content
            );
        }
        if (node->starting_local_value != NULL) {
            printf(
                "start %s\n",
                node->starting_local_value->symbol_in_use->content
            );
        }
    }
}

int tac_is_directive(enum tac_opcode opcode)
{
    switch (opcode) {
        case TAC_MOVE:
        case TAC_MOVI:
        case TAC_MOVV:
        case TAC_ADD:
        case TAC_SUB:
        case TAC_MUL:
        case TAC_DIV:
        case TAC_LT:
        case TAC_GT:
        case TAC_LE:
        case TAC_GE:
        case TAC_EQ:
        case TAC_NE:
        case TAC_SHMUL:
        case TAC_SHDIV:
        case TAC_AND:
        case TAC_OR:
        case TAC_NOT:
        case TAC_ARG:
        case TAC_PRINT:
        case TAC_READ:
        case TAC_IFZ:
        case TAC_JUMP:
        case TAC_CALL:
        case TAC_RET:
        case TAC_LABEL:
            return 0;
        case TAC_DEFS:
        case TAC_DEFV:
        case TAC_DEFP:
        case TAC_BEGINVEC:
        case TAC_ENDVEC:
        case TAC_BEGINFUN:
        case TAC_ENDFUN:
            return 1;
    }
    panic("TAC opcode %i's directive test not implemented", opcode);
}

int tac_local_value_cmp_block_and_start(
    struct tac_local_value *left,
    tac_block_id_type right_block_id,
    tac_local_id_type right_start_id
)
{
    int cmp = left->start_id - right_start_id;
    if (cmp == 0) {
        cmp = left->block_id - right_block_id;
    }
    return cmp;
}

int tac_local_value_cmp(
    struct tac_local_value *left,
    struct tac_local_value *right
)
{
    return tac_local_value_cmp_block_and_start(
        left,
        right->block_id,
        right->start_id
    );
}
