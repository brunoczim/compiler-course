#ifndef TACGEN_H_
#define TACGEN_H_ 1

#include "ast.h"
#include "tac.h"

struct tac gen_tac_for_ast(struct ast ast);

struct tac gen_tac_for_decl(struct ast_declaration declaration);

struct tac gen_tac_for_scalar_var_decl(struct ast_scalar_var_decl declaration);

struct tac gen_tac_for_vector_var_decl(struct ast_vector_var_decl declaration);

struct tac gen_tac_for_function_decl(struct ast_function_decl declaration);

struct tac gen_tac_for_body(struct ast_body body);

struct tac gen_tac_for_scalar_var_assign(
    struct ast_scalar_var_assign scalar_var_assign
);

struct tac gen_tac_for_subscripted_assign(
    struct ast_subscripted_assign subscripted_assign
);

struct tac gen_tac_for_if(struct ast_if if_);

struct tac gen_tac_for_write(struct ast_write write);

struct tac gen_tac_for_while(struct ast_while while_);

struct tac gen_tac_for_return(struct ast_return return_);

struct tac gen_tac_for_expr_stmt(struct ast_expression expression);

struct tac gen_tac_for_stmt(struct ast_statement statement);

struct tac gen_tac_for_expr_with_dest(
    struct ast_expression expression,
    struct symbol **dest
);

struct tac gen_tac_for_input(void);

struct tac gen_tac_for_subscription(struct ast_subscription subscription);

enum tac_opcode unary_operator_to_tac_opcode(enum ast_unary_operator operator);

enum tac_opcode bin_operator_to_tac_opcode(enum ast_binary_operator operator);

struct tac gen_tac_for_bin_operation(struct ast_binary_operation operation);

struct tac gen_tac_for_unary_operation(struct ast_unary_operation operation);

struct tac gen_tac_for_function_call(struct ast_function_call function_call);

struct tac gen_tac_for_symbol_expr(struct symbol *symbol);

struct tac gen_tac_for_expr(struct ast_expression expression);

#endif
