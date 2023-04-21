#include "tacgen.h"
#include "const_eval.h"
#include "panic.h"
#include <stdlib.h>

struct tac gen_tac_for_ast(struct ast ast)
{
    size_t i;
    struct tac tac, decl_tac;
    tac = tac_empty();

    for (i = 0; i < ast.declaration_list.length; i++) {
        decl_tac = gen_tac_for_decl(ast.declaration_list.declarations[i]);
        tac = tac_join(2, tac, decl_tac);
    }

    return tac;
}

struct tac gen_tac_for_decl(struct ast_declaration declaration)
{
    switch (declaration.tag) {
        case AST_SCALAR_VAR_DECL:
            return gen_tac_for_scalar_var_decl(declaration.data.scalar_var);
        case AST_VECTOR_VAR_DECL:
            return gen_tac_for_vector_var_decl(declaration.data.vector_var);
        case AST_FUNCTION_DECL:
            return gen_tac_for_function_decl(declaration.data.function);
    }
    panic(
        "declaration tag %i's TAC generation not implemented",
       declaration.tag
    );
}

struct tac gen_tac_for_scalar_var_decl(struct ast_scalar_var_decl declaration)
{
    struct symbol *src_symbol;
    long const_int;
    char const_char;
    double const_float;
    struct tac_instruction instruction;

    switch (declaration.datatype) {
        case DATATYPE_INTE:
            if (!const_eval_int_expression(declaration.init, &const_int)) {
                const_int = 0;
            }
            src_symbol = symbol_table_create_int_lit(const_int);
            break;
        case DATATYPE_CARA:
            if (!const_eval_char_expression(declaration.init, &const_char)) {
                const_char = 0;
            }
            src_symbol = symbol_table_create_char_lit(const_char);
            break;
        case DATATYPE_REAL:
            if (!const_eval_float_expression(declaration.init, &const_float)) {
                const_float = 0.0;
            }
            src_symbol = symbol_table_create_float_lit(const_float);
            break;
        default:
            panic(
                "declaration datatype %i's TAC generation not implemented",
               declaration.datatype
            );
    }

    instruction.opcode = TAC_DEFS;
    instruction.dest = declaration.name;
    instruction.srcs[0] = src_symbol;
    instruction.srcs[1] = NULL;
    return tac_singleton(instruction);
}

struct tac gen_tac_for_vector_var_decl(struct ast_vector_var_decl declaration)
{
    size_t i;
    struct symbol *src_symbol;
    long const_length;
    long const_int;
    char const_char;
    double const_float;
    struct tac_instruction instruction;
    struct tac tac = tac_empty();

    if (!const_eval_int_expression(declaration.length, &const_length)) {
        const_length = 0;
    }

    src_symbol = symbol_table_create_int_lit(const_length);

    instruction.opcode = TAC_BEGINVEC;
    instruction.dest = declaration.name;
    instruction.srcs[0] = src_symbol;
    instruction.srcs[1] = NULL;
    tac_append(&tac, instruction);

    for (i = 0; i < declaration.init.length; i++) {
        switch (declaration.datatype) {
            case DATATYPE_INTE:
                if (!const_eval_int_expression(
                    declaration.init.expressions[i],
                    &const_int
                )) {
                    const_int = 0;
                }
                src_symbol = symbol_table_create_int_lit(const_int);
                break;
            case DATATYPE_CARA:
                if (!const_eval_char_expression(
                    declaration.init.expressions[i],
                    &const_char
                )) {
                    const_char = 0;
                }
                src_symbol = symbol_table_create_char_lit(const_char);
                break;
            case DATATYPE_REAL:
                if (!const_eval_float_expression(
                    declaration.init.expressions[i],
                    &const_float
                )) {
                    const_int = 0.0;
                }
                src_symbol = symbol_table_create_float_lit(const_float);
                break;
        }

        instruction.opcode = TAC_DEFV;
        instruction.dest = declaration.name;
        instruction.srcs[0] = src_symbol;
        instruction.srcs[1] = NULL;
        tac_append(&tac, instruction);
    }

    src_symbol = symbol_table_create_int_lit(
        const_length - declaration.init.length
    );
    instruction.opcode = TAC_ENDVEC;
    instruction.dest = declaration.name;
    instruction.srcs[0] = src_symbol;
    instruction.srcs[1] = NULL;
    tac_append(&tac, instruction);

    return tac;
}

struct tac gen_tac_for_function_decl(struct ast_function_decl declaration)
{
    size_t i;

    struct symbol *default_ret_value;
    struct tac_instruction instruction;
    struct tac tac = tac_empty();

    instruction.opcode = TAC_BEGINFUN;
    instruction.dest = declaration.name;
    instruction.srcs[0] = NULL;
    instruction.srcs[1] = NULL;
    tac_append(&tac, instruction);

    for (i = 0; i < declaration.parameter_list.length; i++) {
        instruction.opcode = TAC_DEFP;
        instruction.dest = declaration.parameter_list.parameters[i].name;
        instruction.srcs[0] = NULL;
        instruction.srcs[1] = NULL;
        tac_append(&tac, instruction);
    }

    tac = tac_join(2, tac, gen_tac_for_body(declaration.body));

    if (!ast_body_returns(declaration.body)) {
        switch (declaration.return_datatype) {
            case DATATYPE_CARA:
                default_ret_value = symbol_table_create_char_lit(0);
                break;
            case DATATYPE_INTE:
                default_ret_value = symbol_table_create_int_lit(0);
                break;
            case DATATYPE_REAL:
                default_ret_value = symbol_table_create_float_lit(0.0);
                break;
        }

        instruction.opcode = TAC_RET;
        instruction.dest = NULL;
        instruction.srcs[0] = default_ret_value;
        instruction.srcs[1] = NULL;
        tac_append(&tac, instruction);
    }

    instruction.opcode = TAC_ENDFUN;
    instruction.dest = NULL;
    instruction.srcs[0] = NULL;
    instruction.srcs[1] = NULL;
    tac_append(&tac, instruction);

    return tac;
}

struct tac gen_tac_for_body(struct ast_body body)
{
    size_t i;
    struct tac stmt_tac;
    struct tac tac = tac_empty();

    for (i = 0; i < body.statement_list.length; i++) {
        stmt_tac = gen_tac_for_stmt(body.statement_list.statements[i]);
        tac = tac_join(2, tac, stmt_tac);
    }

    return tac;
}

struct tac gen_tac_for_scalar_var_assign(
    struct ast_scalar_var_assign scalar_var_assign
)
{
    struct tac tac = gen_tac_for_expr(scalar_var_assign.assigned_value);
    if (tac.last != NULL) {
        tac.last->instruction.dest = scalar_var_assign.variable;
    }
    return tac;
}

struct tac gen_tac_for_subscripted_assign(
    struct ast_subscripted_assign subscripted_assign
)
{
    struct symbol *index_symbol;
    struct symbol *assigned_value_symbol;
    struct tac_instruction instruction;
    struct tac index_tac, assigned_value_tac, tac;

    index_tac = gen_tac_for_expr_with_dest(
        subscripted_assign.index,
        &index_symbol
    );

    assigned_value_tac = gen_tac_for_expr_with_dest(
        subscripted_assign.assigned_value,
        &assigned_value_symbol
    );

    tac = tac_join(2, index_tac, assigned_value_tac);

    if (index_symbol != NULL && assigned_value_symbol != NULL) {
        instruction.opcode = TAC_MOVV;
        instruction.dest = subscripted_assign.variable;
        instruction.srcs[0] = index_symbol;
        instruction.srcs[1] = assigned_value_symbol;
        tac_append(&tac, instruction);
    }

    return tac;
}

struct tac gen_tac_for_if(struct ast_if if_)
{
    struct symbol *condition_symbol;
    struct symbol *post_then_label;
    struct symbol *post_else_label;
    struct tac_instruction ifz_instruction, jump_instruction;
    struct tac_instruction post_then_instr, post_else_instr;
    struct tac condition_tac, then_tac, else_tac;

    post_then_label = symbol_table_create_tmp_label();
    post_else_label = symbol_table_create_tmp_label();
    then_tac = tac_empty();
    else_tac = tac_empty();

    condition_tac = gen_tac_for_expr_with_dest(
        if_.condition,
        &condition_symbol
    );

    ifz_instruction.opcode = TAC_IFZ;
    ifz_instruction.dest = post_then_label;
    ifz_instruction.srcs[0] = condition_symbol;
    ifz_instruction.srcs[1] = NULL;
    tac_append(&condition_tac, ifz_instruction);

    if (if_.then != NULL) {
        then_tac = gen_tac_for_stmt(*if_.then);
    }

    jump_instruction.opcode = TAC_JUMP;
    jump_instruction.dest = post_else_label;
    jump_instruction.srcs[0] = NULL;
    jump_instruction.srcs[1] = NULL;
    tac_append(&then_tac, jump_instruction);

    post_then_instr.opcode = TAC_LABEL; 
    post_then_instr.dest = NULL; 
    post_then_instr.srcs[0] = post_then_label; 
    post_then_instr.srcs[1] = NULL; 
    tac_append(&then_tac, post_then_instr);

    if (if_.else_ != NULL) {
        else_tac = gen_tac_for_stmt(*if_.else_);
    }

    post_else_instr.opcode = TAC_LABEL; 
    post_else_instr.dest = NULL; 
    post_else_instr.srcs[0] = post_else_label; 
    post_else_instr.srcs[1] = NULL; 
    tac_append(&else_tac, post_else_instr);

    return tac_join(3, condition_tac, then_tac, else_tac);
}

struct tac gen_tac_for_write(struct ast_write write)
{
    size_t i;
    struct tac_instruction instruction;
    struct symbol *argument_symbol;
    struct tac tac, argument_tac;

    tac = tac_empty();

    for (i = 0; i < write.argument_list.length; i++) {
        switch (write.argument_list.write_arguments[i].tag) {
            case AST_WRITE_EXPRESSION:
                argument_tac = gen_tac_for_expr_with_dest(
                    write.argument_list.write_arguments[i].data.expression,
                    &argument_symbol
                );
                break;
            case AST_WRITE_STRING_LIT:
                argument_tac = tac_empty();
                argument_symbol = 
                    write.argument_list.write_arguments[i].data.string_literal;
                break;
                
        }
        instruction.opcode = TAC_PRINT;
        instruction.dest = NULL;
        instruction.srcs[0] = argument_symbol;
        instruction.srcs[1] = NULL;
        tac_append(&argument_tac, instruction);
        tac = tac_join(2, tac, argument_tac);
    }

    return tac;
}

struct tac gen_tac_for_while(struct ast_while while_)
{
    struct symbol *condition_symbol;
    struct symbol *pre_cond_label;
    struct symbol *post_do_label;
    struct tac_instruction ifz_instruction, jump_instruction;
    struct tac_instruction pre_cond_instr, post_do_instr;
    struct tac condition_tac, do_tac;

    pre_cond_label = symbol_table_create_tmp_label();
    post_do_label = symbol_table_create_tmp_label();
    do_tac = tac_empty();

    condition_tac = gen_tac_for_expr_with_dest(
        while_.condition,
        &condition_symbol
    );

    pre_cond_instr.opcode = TAC_LABEL; 
    pre_cond_instr.dest = NULL; 
    pre_cond_instr.srcs[0] = pre_cond_label; 
    pre_cond_instr.srcs[1] = NULL; 
    tac_prepend(&condition_tac, pre_cond_instr);

    ifz_instruction.opcode = TAC_IFZ;
    ifz_instruction.dest = post_do_label;
    ifz_instruction.srcs[0] = condition_symbol;
    ifz_instruction.srcs[1] = NULL;
    tac_append(&condition_tac, ifz_instruction);

    if (while_.do_ != NULL) {
        do_tac = gen_tac_for_stmt(*while_.do_);
    }

    jump_instruction.opcode = TAC_JUMP;
    jump_instruction.dest = pre_cond_label;
    jump_instruction.srcs[0] = NULL;
    jump_instruction.srcs[1] = NULL;
    tac_append(&do_tac, jump_instruction);

    post_do_instr.opcode = TAC_LABEL; 
    post_do_instr.dest = NULL; 
    post_do_instr.srcs[0] = post_do_label; 
    post_do_instr.srcs[1] = NULL; 
    tac_append(&do_tac, post_do_instr);

    return tac_join(2, condition_tac, do_tac);
}

struct tac gen_tac_for_return(struct ast_return return_)
{
    struct tac_instruction return_instr;
    struct symbol *symbol;
    struct tac tac = gen_tac_for_expr_with_dest(
        return_.returned_value,
        &symbol
    );

    return_instr.opcode = TAC_RET;
    return_instr.dest = NULL;
    return_instr.srcs[0] = symbol;
    return_instr.srcs[1] = NULL;
    tac_append(&tac, return_instr);

    return tac;
}

struct tac gen_tac_for_expr_stmt(struct ast_expression expression)
{
    struct symbol *symbol;
    return gen_tac_for_expr_with_dest(expression, &symbol);
}

struct tac gen_tac_for_stmt(struct ast_statement statement)
{
    switch (statement.tag) {
        case AST_SCALAR_VAR_ASSIGN:
            return gen_tac_for_scalar_var_assign(
                statement.data.scalar_var_assign
            );
        case AST_SUBSCRIPTED_ASSIGN:
            return gen_tac_for_subscripted_assign(
                statement.data.subscripted_assign
            );
        case AST_IF:
            return gen_tac_for_if(statement.data.if_);
        case AST_WHILE:
            return gen_tac_for_while(statement.data.while_);
        case AST_WRITE:
            return gen_tac_for_write(statement.data.write);
        case AST_RETURN:
            return gen_tac_for_return(statement.data.return_);
        case AST_BODY:
            return gen_tac_for_body(statement.data.body);
        case AST_EXPRESSION_STATEMENT:
            return gen_tac_for_expr_stmt(statement.data.expression);
    }
    panic("statement tag %i's TAC generation not implemented", statement.tag);
}

struct tac gen_tac_for_subscription(struct ast_subscription subscription)
{
    struct symbol *index_symbol;
    struct tac_instruction instruction;
    struct tac tac;

    tac = gen_tac_for_expr_with_dest(*subscription.index, &index_symbol);

    instruction.opcode = TAC_MOVI;
    instruction.dest = NULL;
    instruction.srcs[0] = subscription.variable;
    instruction.srcs[1] = index_symbol;

    tac_append(&tac, instruction);

    return tac;
}

struct tac gen_tac_for_symbol_expr(struct symbol *symbol)
{
    struct tac_instruction instruction;

    instruction.opcode = TAC_MOVE;
    instruction.dest = NULL;
    instruction.srcs[0] = symbol;
    instruction.srcs[1] = NULL;

    return tac_singleton(instruction);
}

struct tac gen_tac_for_input(void)
{
    struct tac_instruction instruction;
    instruction.opcode = TAC_READ;
    instruction.dest = NULL;
    instruction.srcs[0] = NULL;
    instruction.srcs[1] = NULL;
    return tac_singleton(instruction);
}

enum tac_opcode bin_operator_to_tac_opcode(enum ast_binary_operator operator)
{
    switch (operator) {
        case AST_ADD: return TAC_ADD;
        case AST_SUB: return TAC_SUB;
        case AST_MUL: return TAC_MUL;
        case AST_DIV: return TAC_DIV;
        case AST_LESS_THAN: return TAC_LT;
        case AST_GREATER_THAN: return TAC_GT;
        case AST_LESS_OR_EQUALS: return TAC_LE;
        case AST_GREATER_OR_EQUALS: return TAC_GE;
        case AST_EQUALS: return TAC_EQ;
        case AST_NOT_EQUAL: return TAC_NE;
        case AST_AND: return TAC_AND;
        case AST_OR: return TAC_OR;
    }
    panic("binary operator %i's TAC generation not implemented", operator);
}

enum tac_opcode unary_operator_to_tac_opcode(enum ast_unary_operator operator)
{
    switch (operator) {
       case AST_NOT: return TAC_NOT;
    }
    panic("unary operator %i's TAC generation not implemented", operator);
}

struct tac gen_tac_for_bin_operation(struct ast_binary_operation operation)
{
    struct tac_instruction instruction;
    struct symbol *left_symbol;
    struct symbol *right_symbol;
    struct tac tac, left_tac, right_tac;

    left_tac = gen_tac_for_expr_with_dest(
        *operation.left_operand,
        &left_symbol
    );
    right_tac = gen_tac_for_expr_with_dest(
        *operation.right_operand,
        &right_symbol
    );

    tac = tac_join(2, left_tac, right_tac);
    instruction.opcode = bin_operator_to_tac_opcode(operation.operator);
    instruction.dest = NULL;
    instruction.srcs[0] = left_symbol;
    instruction.srcs[1] = right_symbol;
    tac_append(&tac, instruction);
    return tac;
}

struct tac gen_tac_for_unary_operation(struct ast_unary_operation operation)
{
    struct tac_instruction instruction;
    struct symbol *operand_symbol;
    struct tac tac;

    tac = gen_tac_for_expr_with_dest(
        *operation.operand,
        &operand_symbol
    );

    instruction.opcode = unary_operator_to_tac_opcode(operation.operator);
    instruction.dest = NULL;
    instruction.srcs[0] = operand_symbol;
    instruction.srcs[1] = NULL;
    tac_append(&tac, instruction);
    return tac;
}

struct tac gen_tac_for_function_call(struct ast_function_call function_call)
{
    size_t i;
    struct symbol *arg_symbol;
    struct tac_instruction instruction;
    struct tac tac, arg_tac;

    tac = tac_empty();

    for (i = 0; i < function_call.argument_list.length; i++) {
        arg_tac = gen_tac_for_expr_with_dest(
            function_call.argument_list.expressions[i],
            &arg_symbol
        );

        instruction.opcode = TAC_ARG;
        instruction.dest = NULL;
        instruction.srcs[0] = arg_symbol;
        instruction.srcs[1] = NULL;
        tac_append(&arg_tac, instruction);

        tac = tac_join(2, tac, arg_tac);
    }

    instruction.opcode = TAC_CALL;
    instruction.dest = NULL;
    instruction.srcs[0] = function_call.function;
    instruction.srcs[1] = NULL;
    tac_append(&tac, instruction);

    return tac;
}

struct tac gen_tac_for_expr_with_dest(
    struct ast_expression expression,
    struct symbol **dest
)
{
    struct tac tac = gen_tac_for_expr(expression);

    *dest = NULL;

    if (tac.last != NULL) {
        if (tac.last->instruction.opcode == TAC_MOVE) {
            *dest = tac.last->instruction.srcs[0];
            tac_pop(&tac, NULL);
        } else {
            *dest = symbol_table_create_tmp_scalar_var(
                semantic_type_to_datatype(expression.semantic_type)
            );
            tac.last->instruction.dest = *dest;
        }
    }

    return tac;
}

struct tac gen_tac_for_expr(struct ast_expression expression)
{
    switch (expression.tag) {
        case AST_INT_LITERAL:
        case AST_CHAR_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_VARIABLE:
            return gen_tac_for_symbol_expr(expression.data.literal);
        case AST_SUBSCRIPTION:
            return gen_tac_for_subscription(expression.data.subscription);
        case AST_BINARY_OPERATION:
            return gen_tac_for_bin_operation(expression.data.binary_operation);
        case AST_UNARY_OPERATION:
            return gen_tac_for_unary_operation(expression.data.unary_operation);
        case AST_FUNCTION_CALL:
            return gen_tac_for_function_call(expression.data.function_call);
        case AST_INPUT:
            return gen_tac_for_input();
    }

    panic("expression tag %i's TAC generation not implemented", expression.tag);
}

