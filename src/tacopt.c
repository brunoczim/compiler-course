#include <limits.h>
#include <stdlib.h>
#include "tacopt.h"
#include "symboltable.h"
#include "vector.h"

static int exact_log2(long integer, int *log);

static void power_of_two(struct tac *tac);

static void reuse_tmps(struct tac *tac);

void optimize_tac(struct tac *tac, tac_opt_flags_type flags)
{
    if (flags & TAC_OPT_POWER_OF_TWO) {
        power_of_two(tac);
    }

    if (flags & TAC_OPT_REUSE_TMPS) {
        reuse_tmps(tac);
    }
}

static void power_of_two(struct tac *tac)
{
    struct tac_node *node;
    int is_power_of_two;
    int log2;
    size_t i;
    
    for (node = tac->first; node != NULL; node = node->next) {
        is_power_of_two = 0;
        switch (node->instruction.opcode) {
            case TAC_MUL:
                i = 0;
                while (!is_power_of_two && i < 2) {
                    if (node->instruction.srcs[i]->type == SYM_LIT_INT) {
                        is_power_of_two = exact_log2(
                            node->instruction.srcs[i]->data.parsed_int,
                            &log2
                        );
                        if (is_power_of_two) {
                            node->instruction.opcode = TAC_SHMUL;
                            node->instruction.srcs[i] =
                                symbol_table_create_int_lit(log2);
                        }
                    }
                    i++;
                }
                break;
            case TAC_DIV:
                if (node->instruction.srcs[1]->type == SYM_LIT_INT) {
                    is_power_of_two = exact_log2(
                        node->instruction.srcs[1]->data.parsed_int,
                        &log2
                    );
                    if (is_power_of_two) {
                        node->instruction.opcode = TAC_SHDIV;
                        node->instruction.srcs[1] =
                            symbol_table_create_int_lit(log2);
                    }
                }
                break;
            default:
                break;
        }
    }
}

static int exact_log2(long integer, int *log)
{
    int is_neg = integer < 0;
    if (is_neg) {
        integer = -integer;
    }
    if (integer == 0) {
        return 0;
    }
    *log = 0;
    while ((integer & 1) == 0) {
        integer >>= 1;
        *log += 1;
    }
    if (integer > 1) {
        return 0;
    }
    if (is_neg) {
        *log = -*log;
    }
    return 1;
}

static void reuse_tmps(struct tac *tac)
{
    size_t i;
    struct tac_node *node;
    struct tac_local_value *local_value;
    struct tac_local_value_set free_symbol_owners;

    tac_compute_locality(tac);

    node = tac->first;

    while (node != NULL) {
        if (!tac_is_directive(node->instruction.opcode)) {
            free_symbol_owners.length = 0;
            free_symbol_owners.ordered_values = NULL;
            while (
                node != NULL
                && !tac_is_directive(node->instruction.opcode)
            ) {
                if (node->instruction.opcode != TAC_LABEL) {
                    if (
                        node->instruction.srcs[0] != NULL
                        && node->instruction.srcs[0]->data.variable.replacement
                            != NULL
                    ) {
                        node->instruction.srcs[0] = node->instruction.srcs[0]
                            ->data.variable.replacement;
                    }
                    if (
                        node->instruction.srcs[1] != NULL
                        && node->instruction.srcs[1]->data.variable.replacement
                            != NULL
                    ) {
                        node->instruction.srcs[1] = node->instruction.srcs[1]
                            ->data.variable.replacement;
                    }
                    for (i = 0; i < node->ending_local_values.length; i++) {
                        tac_insert_local_value(
                            &free_symbol_owners,
                            node->ending_local_values.ordered_values[i],
                            NULL
                        );
                        node->ending_local_values.ordered_values[i]
                            ->old_symbol->data.variable.replacement = NULL;
                        node->ending_local_values
                            .ordered_values[i]->old_symbol =
                                node->ending_local_values
                                    .ordered_values[i]->symbol_in_use;
                        node->ending_local_values
                            .ordered_values[i]->symbol_offered =
                                node->ending_local_values
                                    .ordered_values[i]->symbol_in_use;
                    }
                    if (
                        node->starting_local_value != NULL
                        && free_symbol_owners.length > 0
                    ) {
                        free_symbol_owners.ordered_values = vector_pop(
                            free_symbol_owners.ordered_values,
                            sizeof(*free_symbol_owners.ordered_values),
                            &free_symbol_owners.length,
                            &local_value
                        );
                        node->starting_local_value->symbol_in_use =
                            local_value->symbol_offered;
                        node->instruction.dest =
                            node->starting_local_value->symbol_in_use;
                        node->starting_local_value
                            ->old_symbol->data.variable.replacement =
                                node->starting_local_value->symbol_in_use;
                        node->starting_local_value->old_symbol =
                            local_value->symbol_in_use;
                        local_value->symbol_offered = NULL;
                    }
                }
                node = node->next;
            }
            free(free_symbol_owners.ordered_values);
        } else {
            node = node->next;
        }
    }
}
