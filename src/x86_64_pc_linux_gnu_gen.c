#include <stdint.h>
#include <stdlib.h>
#include "x86_64_pc_linux_gnu_gen.h"
#include "symboltable.h"
#include "panic.h"

#define MAX_REGISTER_PARAMS 6
#define MAX_SSE_REGISTER_PARAMS 8

struct sections {
    struct x86_64_asm_unit data;
    struct x86_64_asm_unit rodata;
    struct x86_64_asm_unit text;
};

struct stack_frame {
    size_t size;
};

struct call_state {
    size_t arg_count;
    size_t sse_arg_count;
    size_t arg_index;
    size_t sse_arg_index;
    size_t stack_arg_count;
    size_t call_stack_offset;
    struct symbol *call_stack_size;
};

int datatype_align(enum datatype datatype);

static struct sections sections_empty(void);

static struct call_state call_state_new(void);

static struct x86_64_asm_unit sections_finish(struct sections sections);

static void gen_enter_boilerplate(struct sections *sections);

static void gen_read_function(struct sections *sections);

static void check_prepare_reg_slice(
    struct sections *sections,
    enum x86_64_register reg
);

static void gen_char_def(struct x86_64_asm_unit *unit, struct symbol *ch_sym);

static void gen_int_def(struct x86_64_asm_unit *unit, struct symbol *int_sym);

static void gen_float_def(struct x86_64_asm_unit *unit, struct symbol *f_sym);

static void gen_string_def(struct x86_64_asm_unit *unit, struct symbol *s_sym);

static void gen_sym_def(struct x86_64_asm_unit *unit, struct symbol *symbol);

static void gen_zeroes_def(
    struct x86_64_asm_unit *unit,
    struct symbol *vector,
    long length
);

static struct x86_64_operand value_operand_from_sym(
    struct sections *sections,
    struct symbol *symbol
);

static struct x86_64_operand address_operand_from_sym(
    struct sections *sections,
    struct symbol *symbol
);

static int parameter_register(size_t param_index, enum x86_64_register *reg);

static int sse_parameter_register(
    size_t param_index,
    enum x86_64_register *reg
);

static void gen_data(struct sections *sections, struct tac tac);

static void gen_code(struct sections *sections, struct tac tac);

static void gen_code_headers(struct sections *sections, struct tac tac);

static void gen_code_bodies(struct sections *sections, struct tac tac);

static void gen_string_sym(struct sections *sections, struct symbol *str_sym);

static void gen_float_sym(struct sections *sections, struct symbol *float_sym);

static struct stack_frame gen_beginfun_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_leave_boilerplate(struct sections *sections);

static void gen_simple_int_bin_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_float_bin_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_shmul_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_shdiv_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_int_mul_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_int_div_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_not_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_int_comparison_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_float_comparison_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_return_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_arg_code(
    struct sections *sections,
    struct call_state *call_state,
    struct tac_node *tac_node
);

static void gen_call_code(
    struct sections *sections,
    struct call_state *call_state,
    struct tac_node *tac_node
);

static void gen_label_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_print_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_read_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_jump_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_ifz_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_move_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_movi_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_movv_code(
    struct sections *sections,
    struct tac_node *tac_node
);

static void gen_read_instructions(
    struct sections *sections,
    struct symbol *symbol,
    enum x86_64_opcode bin_opcode,
    enum x86_64_register reg 
);

static void gen_write_instructions(
    struct sections *sections,
    struct symbol *symbol,
    enum x86_64_opcode bin_opcode,
    enum x86_64_register reg 
);

struct x86_64_asm_unit x86_64_pc_linux_gnu_gen(struct tac tac)
{
    struct sections sections = sections_empty();
    gen_data(&sections, tac);
    gen_code(&sections, tac);
    return sections_finish(sections);
}

static void gen_data(struct sections *sections, struct tac tac)
{
    struct tac_node *tac_node;
    struct x86_64_asm_stmt statement;

    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_DATA;
    statement.data.directive.operand_count = 0;
    x86_64_asm_unit_push(&sections->data, statement);

    for (tac_node = tac.first; tac_node != NULL; tac_node = tac_node->next) {
        switch (tac_node->instruction.opcode) {
            case TAC_DEFS:
                statement.tag = X86_64_DIRECTIVE;
                statement.data.directive.name = X86_64_ALIGN;
                statement.data.directive.operands[0] =
                    symbol_table_create_int_lit(datatype_align(
                        tac_node->instruction.dest->data.variable.type
                    ));
                statement.data.directive.operand_count = 1;
                x86_64_asm_unit_push(&sections->data, statement);
                statement.tag = X86_64_LABEL;
                statement.data.label = tac_node->instruction.dest;
                x86_64_asm_unit_push(&sections->data, statement);
                gen_sym_def(&sections->data, tac_node->instruction.srcs[0]);
                break;
            case TAC_DEFV:
                gen_sym_def(&sections->data, tac_node->instruction.srcs[0]);
                break;
            case TAC_BEGINVEC:
                statement.tag = X86_64_DIRECTIVE;
                statement.data.directive.name = X86_64_ALIGN;
                statement.data.directive.operands[0] =
                    symbol_table_create_int_lit(datatype_align(
                        tac_node->instruction.dest->data.variable.type
                    ));
                statement.data.directive.operand_count = 1;
                x86_64_asm_unit_push(&sections->data, statement);
                statement.tag = X86_64_LABEL;
                statement.data.label = tac_node->instruction.dest;
                x86_64_asm_unit_push(&sections->data, statement);
                break;
            case TAC_ENDVEC:
                gen_zeroes_def(
                    &sections->data,
                    tac_node->instruction.dest,
                    tac_node->instruction.srcs[0]->data.parsed_int
                );
                break;
            default:
                break;
        }
    }
}

static void gen_code(struct sections *sections, struct tac tac)
{
    gen_code_headers(sections, tac);
    gen_code_bodies(sections, tac);
}

static void gen_code_headers(struct sections *sections, struct tac tac)
{
    struct tac_node *tac_node;
    struct x86_64_asm_stmt statement;

    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_TEXT;
    statement.data.directive.operand_count = 0;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_RODATA;
    statement.data.directive.operand_count = 0;
    x86_64_asm_unit_push(&sections->rodata, statement);

    for (tac_node = tac.first; tac_node != NULL; tac_node = tac_node->next) {
        switch (tac_node->instruction.opcode) {
            case TAC_BEGINFUN:
                statement.tag = X86_64_DIRECTIVE;
                statement.data.directive.name = X86_64_GLOBL;
                statement.data.directive.operands[0] =
                    tac_node->instruction.dest;
                statement.data.directive.operand_count = 1;
                x86_64_asm_unit_push(&sections->text, statement);
                statement.tag = X86_64_DIRECTIVE;
                statement.data.directive.name = X86_64_TYPE;
                statement.data.directive.operands[0] =
                    tac_node->instruction.dest;
                statement.data.directive.operands[1] =
                    symbol_table_insert("@function");
                statement.data.directive.operands[1]->type = SYM_ANNOTATION;
                statement.data.directive.operand_count = 2;
                x86_64_asm_unit_push(&sections->text, statement);
                break;
            default:
                break;
        }
    }
}

static void gen_code_bodies(struct sections *sections, struct tac tac)
{
    struct tac_node *tac_node;
    struct call_state call_state;
    enum x86_64_register_size reg_size;

    call_state = call_state_new();

    for (tac_node = tac.first; tac_node != NULL; tac_node = tac_node->next) {
        switch (tac_node->instruction.opcode) {
            case TAC_BEGINFUN:
                gen_beginfun_code(sections, tac_node);
                break;
            case TAC_MOVE:
                gen_move_code(sections, tac_node);
                break;
            case TAC_MOVI:
                gen_movi_code(sections, tac_node);
                break;
            case TAC_MOVV:
                gen_movv_code(sections, tac_node);
                break;
            case TAC_MUL:
                if (reg_size == X86_64_SSE) {
                    gen_float_bin_code(sections, tac_node);
                } else {
                    gen_int_mul_code(sections, tac_node);
                }
                break;
            case TAC_DIV:
                if (reg_size == X86_64_SSE) {
                    gen_float_bin_code(sections, tac_node);
                } else {
                    gen_int_div_code(sections, tac_node);
                }
                break;
            case TAC_ADD:
            case TAC_SUB:
                reg_size = x86_64_symbol_reg_size(
                    tac_node->instruction.srcs[0]
                );
                if (reg_size == X86_64_SSE) {
                    gen_float_bin_code(sections, tac_node);
                    break;
                }
            case TAC_AND:
            case TAC_OR:
                gen_simple_int_bin_code(sections, tac_node);
                break;
            case TAC_NOT:
                gen_not_code(sections, tac_node);
                break;
            case TAC_EQ:
            case TAC_NE:
            case TAC_LT:
            case TAC_LE:
            case TAC_GT:
            case TAC_GE:
                reg_size = x86_64_symbol_reg_size(
                    tac_node->instruction.srcs[0]
                );
                if (reg_size == X86_64_SSE) {
                    gen_float_comparison_code(sections, tac_node);
                } else {
                    gen_int_comparison_code(sections, tac_node);
                }
                break;
            case TAC_SHMUL:
                gen_shmul_code(sections, tac_node);
                break;
            case TAC_SHDIV:
                gen_shdiv_code(sections, tac_node);
                break;
            case TAC_RET:
                gen_return_code(sections, tac_node);
                break;
            case TAC_ARG:
                gen_arg_code(sections, &call_state, tac_node);
                break;
            case TAC_CALL:
                gen_call_code(sections, &call_state, tac_node);
                break;
            case TAC_LABEL:
                gen_label_code(sections, tac_node);
                break;
            case TAC_JUMP:
                gen_jump_code(sections, tac_node);
                break;
            case TAC_IFZ:
                gen_ifz_code(sections, tac_node);
                break;
            case TAC_PRINT:
                gen_print_code(sections, tac_node);
                break;
            case TAC_READ:
                gen_read_code(sections, tac_node);
                break;
            default:
                break;
        }
    }

    gen_read_function(sections);
}

static struct stack_frame gen_beginfun_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    int is_parameter_register;
    size_t i, arg_i, sse_arg_i, stack_arg_i;
    size_t stack_frame_byte_size;
    enum x86_64_register reg;
    enum x86_64_register_size reg_size;
    enum x86_64_opcode opcode;
    struct x86_64_asm_stmt statement;
    struct stack_frame stack_frame;
    struct tac_node *lookahead_node;
    struct tac_node *first_sse_stack_node;
    struct tac_node *first_stack_node;
    struct symbol *tac_operands[TAC_MAX_OPERANDS] = { NULL };

    stack_frame.size = 0;

    statement.tag = X86_64_LABEL;
    statement.data.label = tac_node->instruction.dest;
    x86_64_asm_unit_push(&sections->text, statement);

    gen_enter_boilerplate(sections);

    lookahead_node = tac_node->next;
    first_sse_stack_node = NULL;
    first_stack_node = NULL;
    arg_i = 0;
    sse_arg_i = 0;
    stack_arg_i = 0;
    while (
        lookahead_node != NULL && lookahead_node->instruction.opcode == TAC_DEFP
    ) {
        statement.tag = X86_64_LABEL;
        statement.data.label = lookahead_node->instruction.dest;
        x86_64_asm_unit_push(&sections->data, statement);
        gen_sym_def(&sections->data, symbol_table_create_int_lit(0));

        reg_size = x86_64_symbol_reg_size(lookahead_node->instruction.dest);

        if (reg_size == X86_64_SSE) {
            is_parameter_register = sse_parameter_register(sse_arg_i, &reg);
            if (!is_parameter_register) {
                stack_arg_i++;
                if (first_sse_stack_node == NULL) {
                    first_sse_stack_node = lookahead_node;
                }
            }
            sse_arg_i++;
            opcode = X86_64_MOVQ;
        } else {
            is_parameter_register = parameter_register(arg_i, &reg);
            if (!is_parameter_register) {
                stack_arg_i++;
                if (first_stack_node == NULL) {
                    first_stack_node = lookahead_node;
                }
            }
            arg_i++;
            opcode = X86_64_MOV;
        }

        if (is_parameter_register) {
            gen_write_instructions(
                sections,
                lookahead_node->instruction.dest,
                opcode,
                reg
            );
        }

        lookahead_node = lookahead_node->next;
    }

    lookahead_node = lookahead_node->prev;

    while (
        lookahead_node != NULL && (
            first_sse_stack_node != NULL
            || first_stack_node != NULL
        )
    ) {
        reg_size = x86_64_symbol_reg_size(lookahead_node->instruction.dest);

        if (reg_size == X86_64_SSE) {
            sse_arg_i--;
            is_parameter_register = sse_arg_i < MAX_SSE_REGISTER_PARAMS;
        } else {
            arg_i--;
            is_parameter_register = arg_i < MAX_REGISTER_PARAMS;
        }

        if (!is_parameter_register) {
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_MOV;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_RAX;
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_DISPLACED;
            statement.data.instruction.operands[1].data.displaced.base =
                X86_64_RBP;
            statement.data.instruction.operands[1].data.displaced.displacement =
                symbol_table_create_int_lit((stack_arg_i + 1) * 8);
            x86_64_asm_unit_push(&sections->text, statement);

            gen_write_instructions(
                sections,
                lookahead_node->instruction.dest,
                X86_64_MOV,
                X86_64_RAX
            );

            stack_arg_i--;
        }

        if (lookahead_node == first_sse_stack_node) {
            first_sse_stack_node = NULL;
        }
        if (lookahead_node == first_stack_node) {
            first_stack_node = NULL;
        }

        lookahead_node = lookahead_node->prev;
    }

    lookahead_node = tac_node;
    while (
        lookahead_node != NULL
        && lookahead_node->instruction.opcode != TAC_ENDFUN
    ) {
        tac_operands[0] = lookahead_node->instruction.dest;
        tac_operands[1] = lookahead_node->instruction.srcs[0];
        tac_operands[2] = lookahead_node->instruction.srcs[1];
        for (i = 0; i < TAC_MAX_OPERANDS; i++) {
            if (
                tac_operands[i] != NULL
                && tac_operands[i]->type == SYM_TMP_VAR
                && tac_operands[i]->data.variable
                    .stack_frame_index == SIZE_MAX
            ) {
                tac_operands[i]->data.variable.stack_frame_index =
                    stack_frame.size * 8;
                stack_frame.size++;
            }
        }
        lookahead_node = lookahead_node->next;
    }
    
    stack_frame_byte_size = stack_frame.size * 8;
    if (stack_frame_byte_size % 16 != 0) {
        stack_frame_byte_size += 8;
    }

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_SUB;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RSP;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit(stack_frame_byte_size);
    statement.data.instruction.operand_count = 2;
    x86_64_asm_unit_push(&sections->text, statement);

    return stack_frame;
}

static void gen_char_def(struct x86_64_asm_unit *unit, struct symbol *ch_sym)
{
    struct x86_64_asm_stmt statement;
    struct symbol *str_sym;

    str_sym = symbol_table_char_to_str_lit(ch_sym->data.parsed_char);

    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_ASCII;
    statement.data.directive.operands[0] = str_sym;
    statement.data.directive.operand_count = 1;
    x86_64_asm_unit_push(unit, statement);
}

static void gen_int_def(struct x86_64_asm_unit *unit, struct symbol *int_sym)
{
    struct x86_64_asm_stmt statement;
    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_QUAD;
    statement.data.directive.operands[0] = int_sym;
    statement.data.directive.operand_count = 1;
    x86_64_asm_unit_push(unit, statement);
}

static void gen_float_def(struct x86_64_asm_unit *unit, struct symbol *f_sym)
{
    struct x86_64_asm_stmt statement;
    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_DOUBLE;
    statement.data.directive.operands[0] = f_sym;
    statement.data.directive.operand_count = 1;
    x86_64_asm_unit_push(unit, statement);
}

static void gen_string_def(struct x86_64_asm_unit *unit, struct symbol *s_sym)
{
    struct x86_64_asm_stmt statement;
    statement.tag = X86_64_DIRECTIVE;
    statement.data.directive.name = X86_64_ASCII;
    statement.data.directive.operand_count = 1;
    statement.data.directive.operands[0] = s_sym;
    x86_64_asm_unit_push(unit, statement);
}

static void gen_sym_def(struct x86_64_asm_unit *unit, struct symbol *symbol)
{
    switch (symbol->type) {
        case SYM_LIT_CHAR:
            gen_char_def(unit, symbol);
            break;
        case SYM_LIT_INT:
            gen_int_def(unit, symbol);
            break;
        case SYM_LIT_FLOAT:
            gen_float_def(unit, symbol);
            break;
        case SYM_LIT_STR:
            gen_string_def(unit, symbol);
            break;
        default:
            panic(
                "symbol type %i not supported for symbol definition",
                symbol->type
            );
    }
}

static void gen_zeroes_def(
    struct x86_64_asm_unit *unit,
    struct symbol *vector,
    long length
)
{
    struct x86_64_asm_stmt statement;
    long size = length;

    switch (vector->data.variable.type) {
        case DATATYPE_INTE:
        case DATATYPE_REAL:
            size *= 8;
            break;
        case DATATYPE_CARA:
            break;
        default:
            panic(
                "datatype %i's zero definition is not implemented",
                vector->data.variable.type
            );
    }

    if (size > 0) {
        statement.tag = X86_64_DIRECTIVE;
        statement.data.directive.name = X86_64_ZERO;
        statement.data.directive.operands[0] =
            symbol_table_create_int_lit(size);
        statement.data.directive.operand_count = 1;
        x86_64_asm_unit_push(unit, statement);
    }
}

static struct x86_64_operand value_operand_from_sym(
    struct sections *sections,
    struct symbol *symbol
)
{
    struct x86_64_asm_stmt statement;
    struct x86_64_operand operand;

    switch (symbol->type) {
        case SYM_TMP_VAR:
            operand.tag = X86_64_OPERAND_DISPLACED;
            operand.data.displaced.base = X86_64_RBP;
            operand.data.displaced.displacement =
                symbol_table_create_int_lit(
                    - (long)  symbol->data.variable.stack_frame_index - 8
                );
            break;

        case SYM_LIT_STR:
            gen_string_sym(sections, symbol);
            symbol = symbol->data.string.identifier;
        case SYM_STR_ADDR:
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_LEA;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_DISPLACED;
            statement.data.instruction.operands[1].data.displaced.base =
                X86_64_RIP;
            statement.data.instruction.operands[1].data.displaced.displacement =
                symbol;
            x86_64_asm_unit_push(&sections->text, statement);
            operand.tag = X86_64_OPERAND_DIRECT;
            operand.data.direct = X86_64_R11;
            break;

        case SYM_LIT_CHAR:
            operand.tag = X86_64_OPERAND_IMMEDIATE;
            operand.data.immediate = symbol;
            break;

        case SYM_LIT_INT:
            statement.tag = X86_64_INSTRUCTION;
            if (
                symbol->data.parsed_int >= INT32_MIN
                && symbol->data.parsed_int <= INT32_MAX
            ) {
                statement.data.instruction.opcode = X86_64_MOV;
            } else {
                statement.data.instruction.opcode = X86_64_MOVABS;
            }
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_IMMEDIATE;
            statement.data.instruction.operands[1].data.immediate = symbol;
            x86_64_asm_unit_push(&sections->text, statement);
            operand.tag = X86_64_OPERAND_DIRECT;
            operand.data.direct = X86_64_R11;
            break;

        case SYM_VECTOR_VAR:
        case SYM_SCALAR_VAR:
            operand.tag = X86_64_OPERAND_DISPLACED;
            operand.data.displaced.base = X86_64_RIP;
            operand.data.displaced.displacement = symbol;
            break;

        case SYM_LIT_FLOAT:
            gen_float_sym(sections, symbol);
            symbol = symbol->data.float_.identifier;
        case SYM_FLOAT_ADDR:
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_MOVSD;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_XMM12;
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_DISPLACED;
            statement.data.instruction.operands[1].data.displaced.base =
                X86_64_RIP;
            statement.data.instruction.operands[1].data.displaced.displacement =
                symbol;
            x86_64_asm_unit_push(&sections->text, statement);

            operand.tag = X86_64_OPERAND_DIRECT;
            operand.data.direct = X86_64_XMM12;
            break;

        default:
            panic(
                "symbol type %i's value operand generation not implemented or supported",
                symbol->type
            );

    }

    return operand;
}

static struct x86_64_operand address_operand_from_sym(
    struct sections *sections,
    struct symbol *symbol
)
{
    struct x86_64_asm_stmt statement;
    struct x86_64_operand operand;

    switch (symbol->type) {
        case SYM_TMP_VAR:
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_LEA;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_DISPLACED;
            statement.data.instruction.operands[1].data.displaced.base =
                X86_64_RBP;
            statement.data.instruction.operands[1].data.displaced.displacement =
                symbol_table_create_int_lit(
                    - (long) symbol->data.variable.stack_frame_index - 8
                );
            x86_64_asm_unit_push(&sections->text, statement);
            operand.tag = X86_64_OPERAND_DIRECT;
            operand.data.direct = X86_64_R11;
            break;

        case SYM_SCALAR_VAR:
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_LEA;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_DISPLACED;
            statement.data.instruction.operands[1].data.displaced.base =
                X86_64_RIP;
            statement.data.instruction.operands[1].data.displaced.displacement =
                symbol;
            x86_64_asm_unit_push(&sections->text, statement);
            operand.tag = X86_64_OPERAND_DIRECT;
            operand.data.direct = X86_64_R11;
            break;
           
        default:
            panic(
                "symbol type %i's address operand generation not implemented or supported",
                symbol->type
            );
    }

    return operand;
}

static struct sections sections_empty(void)
{
    struct sections sections;
    sections.data = x86_64_asm_unit_empty();
    sections.rodata = x86_64_asm_unit_empty();
    sections.text = x86_64_asm_unit_empty();
    return sections;
}

static struct x86_64_asm_unit sections_finish(struct sections sections)
{
    return x86_64_asm_unit_join(
        3,
        sections.data,
        sections.rodata,
        sections.text
    );
}

static void gen_simple_int_bin_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    enum x86_64_opcode opcode;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    switch (tac_node->instruction.opcode) {
        case TAC_ADD:
            opcode = X86_64_ADD;
            break;
        case TAC_SUB:
            opcode = X86_64_SUB;
            break;
        case TAC_AND:
            opcode = X86_64_AND;
            break;
        case TAC_OR:
            opcode = X86_64_OR;
            break;
        default:
            panic("gen_simple_int_bin_code should only be called with ADD, SUB, AND and OR");
    }

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[1],
        opcode,
        X86_64_RAX
    );

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_RAX
    );
}

static void gen_return_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    enum x86_64_register_size reg_size;
    enum x86_64_opcode opcode;
    enum x86_64_register reg;

    reg_size = x86_64_symbol_reg_size(tac_node->instruction.srcs[0]);

    if (reg_size == X86_64_SSE) {
        opcode = X86_64_MOVQ;
        reg = X86_64_XMM0;
    } else {
        opcode = X86_64_MOV;
        reg = X86_64_RAX;
    }

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        opcode,
        reg
    );

    gen_leave_boilerplate(sections);
}

static void gen_arg_code(
    struct sections *sections,
    struct call_state *call_state,
    struct tac_node *tac_node
)
{
    size_t call_stack_size, index;
    enum x86_64_register reg;
    enum x86_64_opcode opcode;
    enum x86_64_register_size reg_size;
    struct x86_64_asm_stmt statement;
    struct tac_node *lookahead_node;

    if (call_state->arg_index == 0 && call_state->sse_arg_index == 0) {
        lookahead_node = tac_node;
        while (
            lookahead_node != NULL
            && lookahead_node->instruction.opcode != TAC_CALL
        ) {
            if (lookahead_node->instruction.opcode == TAC_ARG) {
                reg_size = x86_64_symbol_reg_size(
                    lookahead_node->instruction.srcs[0]
                );
                if (reg_size == X86_64_SSE) {
                    call_state->sse_arg_count++;
                } else {
                    call_state->arg_count++;
                }
            }

            lookahead_node = lookahead_node->next;
        }

        call_stack_size = 0;
        if (call_state->arg_count > MAX_REGISTER_PARAMS) {
            call_stack_size += call_state->arg_count - MAX_REGISTER_PARAMS;
        }
        if (call_state->sse_arg_count > MAX_SSE_REGISTER_PARAMS) {
            call_stack_size +=
                call_state->sse_arg_count - MAX_SSE_REGISTER_PARAMS;
        }

        call_stack_size *= 8;
        call_state->call_stack_offset = call_stack_size;
        if (call_stack_size % 16 == 8) {
            call_stack_size += 8;
        }
        call_state->call_stack_size = symbol_table_create_int_lit(
            call_stack_size
        );

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_SUB;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RSP;
        statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
        statement.data.instruction.operands[1].data.immediate = 
            call_state->call_stack_size;
        x86_64_asm_unit_push(&sections->text, statement);
    }

    reg_size = x86_64_symbol_reg_size(tac_node->instruction.srcs[0]);

    reg = X86_64_RAX;
    if (reg_size == X86_64_SSE) {
        opcode = X86_64_MOVQ;
        if (!sse_parameter_register(call_state->sse_arg_index, &reg)) {
            call_state->stack_arg_count += 1;
            opcode = X86_64_MOV;
        }
        call_state->sse_arg_index += 1;
    } else {
        if (!parameter_register(call_state->arg_index, &reg)) {
            call_state->stack_arg_count += 1;
        }
        opcode = X86_64_MOV;
        call_state->arg_index += 1;
    }

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        opcode,
        reg
    );

    if (reg == X86_64_RAX) {
        index = (call_state->stack_arg_count - 1) * 8;
        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_MOV;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DISPLACED;
        statement.data.instruction.operands[0].data.displaced.base = X86_64_RSP;
        statement.data.instruction.operands[0].data.displaced.displacement =
            symbol_table_create_int_lit(index);
        statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[1].data.direct = reg;
        x86_64_asm_unit_push(&sections->text, statement);
    }
}

static void gen_call_code(
    struct sections *sections,
    struct call_state *call_state,
    struct tac_node *tac_node
)
{
    enum x86_64_register_size reg_size;
    enum x86_64_register reg;
    enum x86_64_opcode opcode;
    struct x86_64_asm_stmt statement;

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CALL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address =
        tac_node->instruction.srcs[0];
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_ADD;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RSP;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        call_state->call_stack_size;
    x86_64_asm_unit_push(&sections->text, statement);

    reg_size = x86_64_symbol_reg_size(tac_node->instruction.dest);

    if (reg_size == X86_64_SSE) {
        opcode = X86_64_MOVQ;
        reg = X86_64_XMM0;
    } else {
        opcode = X86_64_MOV;
        reg = X86_64_RAX;
    }

    gen_write_instructions(
        sections,
        tac_node->instruction.dest, 
        opcode,
        reg
    );

    *call_state = call_state_new();
}

static int parameter_register(size_t param_index, enum x86_64_register *reg)
{
    switch (param_index) {
        case 0:
            *reg = X86_64_RDI;
            return 1;
        case 1:
            *reg = X86_64_RSI;
            return 1;
        case 2:
            *reg = X86_64_RDX;
            return 1;
        case 3:
            *reg = X86_64_RCX;
            return 1;
        case 4:
            *reg = X86_64_R8;
            return 1;
        case 5:
            *reg = X86_64_R9;
            return 1;
        default:
            return 0;
    }
}

static int sse_parameter_register(
    size_t param_index,
    enum x86_64_register *reg
)
{
    switch (param_index) {
        case 0:
            *reg = X86_64_XMM0;
            return 1;
        case 1:
            *reg = X86_64_XMM1;
            return 1;
        case 2:
            *reg = X86_64_XMM2;
            return 1;
        case 3:
            *reg = X86_64_XMM3;
            return 1;
        case 4:
            *reg = X86_64_XMM4;
            return 1;
        case 5:
            *reg = X86_64_XMM5;
            return 1;
        case 6:
            *reg = X86_64_XMM6;
            return 1;
        case 7:
            *reg = X86_64_XMM7;
            return 1;
        default:
            return 0;
    }
}

static void gen_label_code(struct sections *sections, struct tac_node *tac_node)
{
    struct x86_64_asm_stmt statement;
    statement.tag = X86_64_LABEL;
    statement.data.label = tac_node->instruction.srcs[0];
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_print_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    int sse_count = 0;
    enum x86_64_register param_reg;
    struct x86_64_operand operand;
    enum x86_64_opcode opcode;
    struct x86_64_asm_stmt statement;
    enum datatype datatype;
    struct symbol *format_spec;
    int is_string = 0;

    switch (tac_node->instruction.srcs[0]->type) {
        case SYM_LIT_CHAR:
            datatype = DATATYPE_CARA;
            break;
        case SYM_LIT_INT:
            datatype = DATATYPE_INTE;
            break;
        case SYM_LIT_FLOAT:
            datatype = DATATYPE_REAL;
            break;
        case SYM_LIT_STR:
            is_string = 1;
            break;
        case SYM_SCALAR_VAR:
        case SYM_TMP_VAR:
            datatype = tac_node->instruction.srcs[0]->data.variable.type;
            break;
        default:
            panic(
                "symbol type %i not supported for symbol definition",
                tac_node->instruction.srcs[0]->type
            );
    }
    
    if (is_string) {
        operand = value_operand_from_sym(
            sections,
            tac_node->instruction.srcs[0]
        );

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_MOV;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RDI;
        statement.data.instruction.operands[1] = operand;
        x86_64_asm_unit_push(&sections->text, statement);

        operand = value_operand_from_sym(
            sections,
            symbol_table_create_int_lit(1)
        );

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_MOV;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RSI;
        statement.data.instruction.operands[1] = operand;
        x86_64_asm_unit_push(&sections->text, statement);

        operand = value_operand_from_sym(
            sections,
            symbol_table_create_int_lit(
                tac_node->instruction.srcs[0]->data.string.literal.length
            )
        );

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_MOV;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RDX;
        statement.data.instruction.operands[1] = operand;
        x86_64_asm_unit_push(&sections->text, statement);

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_MOV;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RCX;
        statement.data.instruction.operands[1].tag =
            X86_64_OPERAND_DISPLACED;
        statement.data.instruction.operands[1].data.displaced.base =
            X86_64_RIP;
        statement.data.instruction.operands[1].data.displaced.displacement =
            symbol_table_insert("stdout");
        statement.data.instruction.operands[1].data.displaced.displacement
            ->type = SYM_SCALAR_VAR;
        statement.data.instruction.operands[1].data.displaced.displacement
            ->data.variable.in_scope = 1;
        statement.data.instruction.operands[1].data.displaced.displacement
            ->data.variable.type = DATATYPE_INTE;
        x86_64_asm_unit_push(&sections->text, statement);

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_CALL;
        statement.data.instruction.operand_count = 1;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_PLT;
        statement.data.instruction.operands[0].data.address =
            symbol_table_insert("fwrite");
        statement.data.instruction.operands[0].data.address->type =
            SYM_EXTERNAL;
        x86_64_asm_unit_push(&sections->text, statement);
    } else {
        switch (datatype) {
            case DATATYPE_CARA:
                format_spec = symbol_table_create_str_lit("%c\\0");
                opcode = X86_64_MOV;
                param_reg = X86_64_RSI;
                break;
            case DATATYPE_INTE:
                format_spec = symbol_table_create_str_lit("%li\\0");
                opcode = X86_64_MOV;
                param_reg = X86_64_RSI;
                break;
            case DATATYPE_REAL:
                format_spec = symbol_table_create_str_lit("%lf\\0");
                opcode = X86_64_MOVQ;
                param_reg = X86_64_XMM0;
                sse_count += 1;
                break;
        }

        gen_read_instructions(
            sections,
            format_spec,
            X86_64_MOV,
            X86_64_RDI
        );

        gen_read_instructions(
            sections,
            tac_node->instruction.srcs[0],
            opcode,
            param_reg
        );

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_MOV;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RAX;
        statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
        statement.data.instruction.operands[1].data.immediate =
            symbol_table_create_int_lit(sse_count);
        x86_64_asm_unit_push(&sections->text, statement);

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_CALL;
        statement.data.instruction.operand_count = 1;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_PLT;
        statement.data.instruction.operands[0].data.address =
            symbol_table_insert("printf");
        statement.data.instruction.operands[0].data.address->type =
            SYM_EXTERNAL;
        x86_64_asm_unit_push(&sections->text, statement);
    }
}

static void gen_read_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CALL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address =
        symbol_table_insert("@entrada");
    statement.data.instruction.operands[0].data.address->type =
        SYM_LABEL;
    x86_64_asm_unit_push(&sections->text, statement);

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_RAX
    );
}

static void gen_string_sym(struct sections *sections, struct symbol *str_sym)
{
    struct x86_64_asm_stmt statement;
    if (str_sym->data.string.identifier == NULL) {
        str_sym->data.string.identifier = symbol_table_create_str_addr();
        
        statement.tag = X86_64_LABEL;
        statement.data.label = str_sym->data.string.identifier;
        x86_64_asm_unit_push(&sections->rodata, statement);
        gen_sym_def(&sections->rodata, str_sym);
    }
}

static void gen_float_sym(struct sections *sections, struct symbol *float_sym)
{
    struct x86_64_asm_stmt statement;
    if (float_sym->data.float_.identifier == NULL) {
        float_sym->data.float_.identifier = symbol_table_create_float_addr();

        statement.tag = X86_64_DIRECTIVE;
        statement.data.directive.name = X86_64_ALIGN;
        statement.data.directive.operands[0] = symbol_table_create_int_lit(8);
        statement.data.directive.operand_count = 1;
        x86_64_asm_unit_push(&sections->rodata, statement);
        
        statement.tag = X86_64_LABEL;
        statement.data.label = float_sym->data.float_.identifier;
        x86_64_asm_unit_push(&sections->rodata, statement);
        gen_float_def(&sections->rodata, float_sym);
    }
}

static void gen_jump_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;
    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JMP;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address =
        tac_node->instruction.dest;
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_ifz_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_TEST;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1] =
        statement.data.instruction.operands[0];
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JZ;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address =
        tac_node->instruction.dest;
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_float_comparison_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOVSD,
        X86_64_XMM13
    );

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[1],
        X86_64_MOVSD,
        X86_64_XMM14
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_UCOMISD;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_XMM13;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = X86_64_XMM14;
    x86_64_asm_unit_push(&sections->text, statement);

    switch (tac_node->instruction.opcode) {
        case TAC_EQ:
            check_prepare_reg_slice(sections, X86_64_R11B);
            check_prepare_reg_slice(sections, X86_64_R10B);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETZ;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNP;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R10B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_AND;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[1].data.direct = X86_64_R10;
            x86_64_asm_unit_push(&sections->text, statement);
            break;

        case TAC_NE:
            check_prepare_reg_slice(sections, X86_64_R11B);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNZ;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11B;
            x86_64_asm_unit_push(&sections->text, statement);
            break;

        case TAC_LT:
            check_prepare_reg_slice(sections, X86_64_R11B);
            check_prepare_reg_slice(sections, X86_64_R10B);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETC;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNP;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R10B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_AND;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[1].data.direct = X86_64_R10;
            x86_64_asm_unit_push(&sections->text, statement);
            break;

        case TAC_LE:
            check_prepare_reg_slice(sections, X86_64_R11B);
            check_prepare_reg_slice(sections, X86_64_R10B);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETC;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETZ;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R10B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_OR;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[1].data.direct = X86_64_R10;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNP;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R10B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_AND;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[1].data.direct = X86_64_R10;
            x86_64_asm_unit_push(&sections->text, statement);
            break;

        case TAC_GT:
            check_prepare_reg_slice(sections, X86_64_R11B);
            check_prepare_reg_slice(sections, X86_64_R10B);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNC;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNZ;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R10B;
            x86_64_asm_unit_push(&sections->text, statement);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_AND;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11;
            statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[1].data.direct = X86_64_R10;
            x86_64_asm_unit_push(&sections->text, statement);
            break;

        case TAC_GE:
            check_prepare_reg_slice(sections, X86_64_R11B);
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_SETNC;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_R11B;
            x86_64_asm_unit_push(&sections->text, statement);
            break;

        default:
            panic("gen_float_comparison_code should only be called with comparison operators");
    }

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_R11
    );
}

static void gen_int_comparison_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[1],
        X86_64_MOV,
        X86_64_R10
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMP;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = X86_64_R10;
    x86_64_asm_unit_push(&sections->text, statement);

    check_prepare_reg_slice(sections, X86_64_R10B);
    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_R10B;

    switch (tac_node->instruction.opcode) {
        case TAC_EQ:
            statement.data.instruction.opcode = X86_64_SETZ;
            break;

        case TAC_NE:
            statement.data.instruction.opcode = X86_64_SETNZ;
            break;

        case TAC_LT:
            statement.data.instruction.opcode = X86_64_SETL;
            break;

        case TAC_LE:
            statement.data.instruction.opcode = X86_64_SETLE;
            break;

        case TAC_GT:
            statement.data.instruction.opcode = X86_64_SETG;
            break;

        case TAC_GE:
            statement.data.instruction.opcode = X86_64_SETGE;
            break;

        default:
            panic("gen_int_comparison_code should only be called with comparison operators");
    }

    x86_64_asm_unit_push(&sections->text, statement);
    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_R10
    );
}

static void gen_move_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_RAX
    );
}

static void gen_movi_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;
    int data_size;

    data_size = x86_64_symbol_data_size(tac_node->instruction.srcs[0]);

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_LEA,
        X86_64_RAX
    );

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[1],
        X86_64_MOV,
        X86_64_R9
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct =
        x86_64_make_register_of_size(data_size, X86_64_R8);
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_INDEXED;
    statement.data.instruction.operands[1].data.indexed.base = X86_64_RAX;
    statement.data.instruction.operands[1].data.indexed.index = X86_64_R9;
    statement.data.instruction.operands[1].data.indexed.scale = data_size;
    statement.data.instruction.operands[1].data.indexed.displacement =
       symbol_table_create_int_lit(0);
    x86_64_asm_unit_push(&sections->text, statement);

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_R8
    );
}

static void gen_movv_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    int data_size;
    struct x86_64_asm_stmt statement;

    data_size = x86_64_symbol_data_size(tac_node->instruction.dest);

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[1],
        X86_64_MOV,
        X86_64_R9
    );

    gen_read_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_LEA,
        X86_64_R8
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_INDEXED;
    statement.data.instruction.operands[0].data.indexed.base = X86_64_R8;
    statement.data.instruction.operands[0].data.indexed.index = X86_64_RAX;
    statement.data.instruction.operands[0].data.indexed.scale = data_size;
    statement.data.instruction.operands[0].data.indexed.displacement =
       symbol_table_create_int_lit(0);
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct =
        x86_64_make_register_of_size(data_size, X86_64_R9);
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_read_instructions(
    struct sections *sections,
    struct symbol *symbol,
    enum x86_64_opcode bin_opcode,
    enum x86_64_register reg 
)
{
    struct x86_64_operand operand;
    struct x86_64_asm_stmt statement;
    enum x86_64_register_size reg_size;

    operand = value_operand_from_sym(sections, symbol);
    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = bin_opcode;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    if  (bin_opcode == X86_64_LEA) {
        statement.data.instruction.operands[0].data.direct = reg;
    } else {
        reg_size = x86_64_symbol_reg_size(symbol);
        statement.data.instruction.operands[0].data.direct =
            x86_64_make_register_of_size(reg_size, reg);
    }
    check_prepare_reg_slice(
        sections,
        statement.data.instruction.operands[0].data.direct
    );
    statement.data.instruction.operands[1] = operand;
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_write_instructions(
    struct sections *sections,
    struct symbol *symbol,
    enum x86_64_opcode bin_opcode,
    enum x86_64_register reg 
)
{
    struct x86_64_operand operand;
    struct x86_64_asm_stmt statement;
    enum x86_64_register_size reg_size = x86_64_symbol_reg_size(symbol);

    operand = value_operand_from_sym(sections, symbol);
    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = bin_opcode;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0] = operand;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = 
        x86_64_make_register_of_size(reg_size, reg);
    check_prepare_reg_slice(
        sections,
        statement.data.instruction.operands[0].data.direct
    );
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_not_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_NOT;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_AND;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate = 
        symbol_table_create_int_lit(1);
    x86_64_asm_unit_push(&sections->text, statement);

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_RAX
    );
}


static void gen_shmul_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    if (labs(tac_node->instruction.srcs[1]->data.parsed_int) <= 3) {
        gen_read_instructions(
            sections,
            tac_node->instruction.srcs[0],
            X86_64_MOV,
            X86_64_RAX
        );

        if (tac_node->instruction.srcs[1]->data.parsed_int < 0) {
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_NEG;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_RAX;
            x86_64_asm_unit_push(&sections->text, statement);
        }

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_LEA;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RAX;
        statement.data.instruction.operands[1].tag = X86_64_OPERAND_SCALED;
        statement.data.instruction.operands[1].data.scaled.index = X86_64_RAX;
        statement.data.instruction.operands[1].data.scaled.scale = 
            1 << labs(tac_node->instruction.srcs[1]->data.parsed_int);
        statement.data.instruction.operands[1].data.scaled.displacement = 
            symbol_table_create_int_lit(0);
        x86_64_asm_unit_push(&sections->text, statement);

        gen_write_instructions(
            sections,
            tac_node->instruction.dest,
            X86_64_MOV,
            X86_64_RAX
        );
    } else {
        gen_read_instructions(
            sections,
            tac_node->instruction.srcs[0],
            X86_64_MOV,
            X86_64_RAX
        );

        if (tac_node->instruction.srcs[1]->data.parsed_int < 0) {
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_NEG;
            statement.data.instruction.operand_count = 1;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct = X86_64_RAX;
            x86_64_asm_unit_push(&sections->text, statement);
        }

        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_SHL;
        statement.data.instruction.operand_count = 2;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_RAX;
        statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
        statement.data.instruction.operands[1].data.immediate =
            symbol_table_create_int_lit(
                labs(tac_node->instruction.srcs[1]->data.parsed_int)
            );
        x86_64_asm_unit_push(&sections->text, statement);

        gen_write_instructions(
            sections,
            tac_node->instruction.dest,
            X86_64_MOV,
            X86_64_RAX
        );
    }
}

static void gen_shdiv_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_LEA;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_R9;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DISPLACED;
    statement.data.instruction.operands[1].data.displaced.base = X86_64_RAX;
    statement.data.instruction.operands[1].data.displaced.displacement =
        symbol_table_create_int_lit(
            (1 << tac_node->instruction.srcs[1]->data.parsed_int) - 1
        );
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_TEST;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1] =
        statement.data.instruction.operands[0];
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMOVNS;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_R9;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = X86_64_RAX;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_SAR;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_R9;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit(
            labs(tac_node->instruction.srcs[1]->data.parsed_int)
         );
    x86_64_asm_unit_push(&sections->text, statement);

    if (tac_node->instruction.srcs[1]->data.parsed_int < 0) {
        statement.tag = X86_64_INSTRUCTION;
        statement.data.instruction.opcode = X86_64_NEG;
        statement.data.instruction.operand_count = 1;
        statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
        statement.data.instruction.operands[0].data.direct = X86_64_R9;
        x86_64_asm_unit_push(&sections->text, statement);
    }

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_R9
    );
}

static void gen_int_mul_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;
    struct x86_64_operand operand;
    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    operand = value_operand_from_sym(
        sections,
        tac_node->instruction.srcs[1]
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_IMUL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0] = operand;
    x86_64_asm_unit_push(&sections->text, statement);

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_RAX
    );
}

static void gen_int_div_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    struct x86_64_asm_stmt statement;
    struct x86_64_operand operand;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOV,
        X86_64_RAX
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CQO;
    statement.data.instruction.operand_count = 0;
    x86_64_asm_unit_push(&sections->text, statement);

    operand = value_operand_from_sym(sections, tac_node->instruction.srcs[1]);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_IDIV;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0] = operand;
    x86_64_asm_unit_push(&sections->text, statement);

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOV,
        X86_64_RAX
    );
}

static void gen_float_bin_code(
    struct sections *sections,
    struct tac_node *tac_node
)
{
    enum x86_64_opcode opcode;

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[0],
        X86_64_MOVQ,
        X86_64_XMM15
    );

    switch (tac_node->instruction.opcode) {
        case TAC_ADD:
            opcode = X86_64_ADDSD;
            break;
        case TAC_SUB:
            opcode = X86_64_SUBSD;
            break;
        case TAC_MUL:
            opcode = X86_64_MULSD;
            break;
        case TAC_DIV:
            opcode = X86_64_DIVSD;
            break;
        default:
            panic("gen_float_bin_code should only be called with float arithmetic operators");
    }

    gen_read_instructions(
        sections,
        tac_node->instruction.srcs[1],
        opcode,
        X86_64_XMM15
    );

    gen_write_instructions(
        sections,
        tac_node->instruction.dest,
        X86_64_MOVQ,
        X86_64_XMM15
    );
}

static struct call_state call_state_new(void)
{
    struct call_state call_state;
    call_state.arg_count = 0;
    call_state.sse_arg_count = 0;
    call_state.arg_index = 0;
    call_state.sse_arg_index = 0;
    call_state.stack_arg_count = 0;
    call_state.call_stack_offset = 0;
    call_state.call_stack_size = symbol_table_create_int_lit(0);
    return call_state;
}

static void gen_leave_boilerplate(struct sections *sections)
{
    struct x86_64_asm_stmt statement;

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operands[0].tag =
        X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RSP;
    statement.data.instruction.operands[1].tag =
        X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = X86_64_RBP;
    statement.data.instruction.operand_count = 2;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_POP;
    statement.data.instruction.operands[0].tag =
        X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RBP;
    statement.data.instruction.operand_count = 1;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_RET;
    statement.data.instruction.operand_count = 0;
    x86_64_asm_unit_push(&sections->text, statement);
}

int datatype_align(enum datatype datatype)
{
    switch (datatype) {
        case DATATYPE_INTE:
        case DATATYPE_REAL:
            return 8;
        case DATATYPE_CARA:
            return 1;
        default:
            panic("datatype %i has no implemented align");
    }
}

static void check_prepare_reg_slice(
    struct sections *sections,
    enum x86_64_register reg
)
{
    struct x86_64_asm_stmt statement;

    switch (x86_64_register_size(reg)) {
        case X86_64_BYTE:
        case X86_64_WORD:
            statement.tag = X86_64_INSTRUCTION;
            statement.data.instruction.opcode = X86_64_MOV;
            statement.data.instruction.operand_count = 2;
            statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
            statement.data.instruction.operands[0].data.direct =
                x86_64_make_register_of_size(X86_64_QWORD, reg);
            statement.data.instruction.operands[1].tag =
                X86_64_OPERAND_IMMEDIATE;
            statement.data.instruction.operands[1].data.immediate =
                symbol_table_create_int_lit(0);
            x86_64_asm_unit_push(&sections->text, statement);
            break;
        default:
            break;
    }
}

static void gen_enter_boilerplate(struct sections *sections)
{
    struct x86_64_asm_stmt statement;

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_PUSH;
    statement.data.instruction.operands[0].tag =
        X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RBP;
    statement.data.instruction.operand_count = 1;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operands[0].tag =
        X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RBP;
    statement.data.instruction.operands[1].tag =
        X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = X86_64_RSP;
    statement.data.instruction.operand_count = 2;
    x86_64_asm_unit_push(&sections->text, statement);
}

static void gen_read_function(struct sections *sections)
{
    struct x86_64_asm_stmt statement;
    struct symbol *name;
    struct symbol *format_spec;
    struct symbol *retry;
    struct symbol *retry_done;

    name = symbol_table_insert("@entrada");
    name->type = SYM_LABEL;

    statement.tag = X86_64_LABEL;
    statement.data.label = name;
    x86_64_asm_unit_push(&sections->text, statement);

    gen_enter_boilerplate(sections);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_SUB;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RSP;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit(16);
    statement.data.instruction.operand_count = 2;
    x86_64_asm_unit_push(&sections->text, statement);

    format_spec = symbol_table_create_str_lit("%li\\0");

    retry = symbol_table_create_tmp_label();
    retry_done = symbol_table_create_tmp_label();

    statement.tag = X86_64_LABEL;
    statement.data.label = retry;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CALL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_PLT;
    statement.data.instruction.operands[0].data.address =
        symbol_table_insert("getchar");
    statement.data.instruction.operands[0].data.address->type =
        SYM_EXTERNAL;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMP;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit('+');
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JZ;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address = retry_done;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMP;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit('-');
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JZ;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address = retry_done;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMP;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit('0');
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address = retry;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMP;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit('9');
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JG;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address = retry;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_LABEL;
    statement.data.label = retry_done;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RDI;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[1].data.direct = X86_64_RAX;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RSI;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DISPLACED;
    statement.data.instruction.operands[1].data.displaced.base = X86_64_RIP;
    statement.data.instruction.operands[1].data.displaced.displacement =
        symbol_table_insert("stdin");
    statement.data.instruction.operands[1].data.displaced.displacement
        ->type = SYM_SCALAR_VAR;
    statement.data.instruction.operands[1].data.displaced.displacement
        ->data.variable.in_scope = 1;
    statement.data.instruction.operands[1].data.displaced.displacement
        ->data.variable.type = DATATYPE_INTE;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CALL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_PLT;
    statement.data.instruction.operands[0].data.address =
        symbol_table_insert("ungetc");
    statement.data.instruction.operands[0].data.address->type =
        SYM_EXTERNAL;
    x86_64_asm_unit_push(&sections->text, statement);

    gen_read_instructions(
        sections,
        format_spec,
        X86_64_MOV,
        X86_64_RDI
    );

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_LEA;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RSI;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DISPLACED;
    statement.data.instruction.operands[1].data.displaced.base = X86_64_RBP;
    statement.data.instruction.operands[1].data.displaced.displacement =
        symbol_table_create_int_lit(-8);
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_XOR;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1] =
        statement.data.instruction.operands[0];
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CALL;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_PLT;
    statement.data.instruction.operands[0].data.address =
        symbol_table_insert("scanf");
    statement.data.instruction.operands[0].data.address->type =
        SYM_EXTERNAL;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_CMP;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_IMMEDIATE;
    statement.data.instruction.operands[1].data.immediate =
        symbol_table_create_int_lit(1);
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_JNZ;
    statement.data.instruction.operand_count = 1;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_ADDRESS;
    statement.data.instruction.operands[0].data.address = retry;
    x86_64_asm_unit_push(&sections->text, statement);

    statement.tag = X86_64_INSTRUCTION;
    statement.data.instruction.opcode = X86_64_MOV;
    statement.data.instruction.operand_count = 2;
    statement.data.instruction.operands[0].tag = X86_64_OPERAND_DIRECT;
    statement.data.instruction.operands[0].data.direct = X86_64_RAX;
    statement.data.instruction.operands[1].tag = X86_64_OPERAND_DISPLACED;
    statement.data.instruction.operands[1].data.displaced.base = X86_64_RBP;
    statement.data.instruction.operands[1].data.displaced.displacement =
        symbol_table_create_int_lit(-8);
    x86_64_asm_unit_push(&sections->text, statement);

    gen_leave_boilerplate(sections);
}
