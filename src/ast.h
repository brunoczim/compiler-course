#ifndef AST_H_
#define AST_H_ 1

#include "symboltable.h"
#include "token_data.h"
#include "types.h"
#include <stdio.h>

/**
 * *********************
 * *********************
 * ******* ENUMS *******
 * *********************
 * *********************
 */

enum ast_declaration_tag {
    AST_SCALAR_VAR_DECL,
    AST_VECTOR_VAR_DECL,
    AST_FUNCTION_DECL
};

enum ast_statement_tag {
    AST_SCALAR_VAR_ASSIGN,
    AST_SUBSCRIPTED_ASSIGN,
    AST_IF,
    AST_WHILE,
    AST_WRITE,
    AST_RETURN,
    AST_BODY,
    AST_EXPRESSION_STATEMENT
};

enum ast_expression_tag {
    AST_INT_LITERAL,
    AST_CHAR_LITERAL,
    AST_FLOAT_LITERAL,
    AST_SUBSCRIPTION,
    AST_VARIABLE,
    AST_BINARY_OPERATION,
    AST_UNARY_OPERATION,
    AST_FUNCTION_CALL,
    AST_INPUT
};

enum ast_binary_operator {
    AST_ADD,
    AST_SUB,
    AST_MUL,
    AST_DIV,
    AST_LESS_THAN,
    AST_GREATER_THAN,
    AST_LESS_OR_EQUALS,
    AST_GREATER_OR_EQUALS,
    AST_EQUALS,
    AST_NOT_EQUAL,
    AST_AND,
    AST_OR
};

enum ast_unary_operator {
    AST_NOT
};

enum ast_write_argument_tag {
    AST_WRITE_EXPRESSION,
    AST_WRITE_STRING_LIT
};

/**
 * *********************
 * *********************
 * ******* LISTS *******
 * *********************
 * *********************
 */

struct ast_write_argument_list {
    size_t length;
    struct ast_write_argument *write_arguments;
};

struct ast_expression_list {
    size_t length;
    struct ast_expression *expressions;
};

struct ast_declaration_list {
    size_t length;
    struct ast_declaration *declarations;
};

struct ast_parameter_list {
    size_t length;
    struct ast_parameter *parameters;
};

struct ast_statement_list {
    size_t length;
    struct ast_statement *statements;
};

/**
 * *********************
 * *********************
 * ******* EXPRS *******
 * *********************
 * *********************
 */

struct ast_subscription {
    struct symbol *variable;
    struct ast_expression *index;
};

struct ast_variable {
    struct symbol *name;
};

struct ast_binary_operation {
    enum ast_binary_operator operator;
    struct ast_expression *left_operand;
    struct ast_expression *right_operand;
};

struct ast_unary_operation {
    enum ast_unary_operator operator;
    struct ast_expression *operand;
};

struct ast_function_call {
    struct symbol *function;
    struct ast_expression_list argument_list;
};

struct ast_expression {
    enum ast_expression_tag tag;
    int line_number;
    enum inference_status inference_status;
    enum semantic_type semantic_type;
    union {
        struct symbol *literal;
        struct ast_subscription subscription;
        struct ast_variable variable;
        struct ast_binary_operation binary_operation;
        struct ast_unary_operation unary_operation;
        struct ast_function_call function_call;
    } data;
};

/**
 * *********************
 * *********************
 * ******* STMTS *******
 * *********************
 * *********************
 */

struct ast_body {
    struct ast_statement_list statement_list;
};

struct ast_scalar_var_assign {
    struct symbol *variable;
    struct ast_expression assigned_value;
};

struct ast_subscripted_assign {
    struct symbol *variable;
    struct ast_expression index;
    struct ast_expression assigned_value;
};

struct ast_if {
    struct ast_statement *then;
    struct ast_statement *else_;
    struct ast_expression condition;
};

struct ast_while {
    struct ast_statement *do_;
    struct ast_expression condition;
};

struct ast_write_argument {
    enum ast_write_argument_tag tag;
    union {
        struct ast_expression expression;
        struct symbol *string_literal;
    } data;
};

struct ast_write {
    struct ast_write_argument_list argument_list;
};

struct ast_return {
    struct ast_expression returned_value;
};

struct ast_statement {
    enum ast_statement_tag tag;
    int line_number;
    union {
        struct ast_scalar_var_assign scalar_var_assign;
        struct ast_subscripted_assign subscripted_assign;
        struct ast_if if_;
        struct ast_while while_;
        struct ast_write write;
        struct ast_return return_;
        struct ast_body body;
        struct ast_expression expression;
    } data;
};

/**
 * *********************
 * *********************
 * ******* DECLS *******
 * *********************
 * *********************
 */

struct ast_parameter {
    enum datatype datatype;
    int line_number;
    struct symbol *name;
};

struct ast_function_decl {
    enum datatype return_datatype;
    struct symbol *name;
    struct ast_parameter_list parameter_list;
    struct ast_body body;
};

struct ast_scalar_var_decl {
    enum datatype datatype;
    struct symbol *name;
    struct ast_expression init;
};

struct ast_vector_var_decl {
    enum datatype datatype;
    struct symbol *name;
    struct ast_expression length;
    struct ast_expression_list init;
};

struct ast_declaration {
    enum ast_declaration_tag tag;
    int line_number;
    union {
        struct ast_scalar_var_decl scalar_var;
        struct ast_vector_var_decl vector_var;
        struct ast_function_decl function;
    } data;
};

/**
 * *********************
 * *********************
 * ******** AST ********
 * *********************
 * *********************
 */

struct ast {
    int is_valid;
    struct ast_declaration_list declaration_list;
};

/**
 * *********************
 * *********************
 * ******* RENDER ******
 * *********************
 * *********************
 */

struct ast_fmt_params {
    FILE *output;
    unsigned spaces_per_level;
    unsigned level;
};

void ast_write_indent(struct ast_fmt_params params);

void ast_binary_operator_render(
    enum ast_binary_operator binary_operator,
    struct ast_fmt_params params
);

void ast_binary_operation_render(
    struct ast_binary_operation binary_operation,
    struct ast_fmt_params params
);

void ast_unary_operator_render(
    enum ast_unary_operator unary_operator,
    struct ast_fmt_params params
);

void ast_unary_operation_render(
    struct ast_unary_operation unary_operation,
    struct ast_fmt_params params
);

void ast_variable_render(
    struct ast_variable variable,
    struct ast_fmt_params params
);

void ast_subscription_render(
    struct ast_subscription subscription,
    struct ast_fmt_params params
);

void ast_function_call_render(
    struct ast_function_call function_call,
    struct ast_fmt_params params
);

void ast_expression_list_render(
    struct ast_expression_list expression_list,
    struct ast_fmt_params params
);

void ast_expression_render(
    struct ast_expression expression,
    struct ast_fmt_params params
);

void ast_body_render(
    struct ast_body body,
    struct ast_fmt_params params
);

void ast_if_render(
    struct ast_if if_,
    struct ast_fmt_params params
);

void ast_while_render(
    struct ast_while while_,
    struct ast_fmt_params params
);

void ast_return_render(
    struct ast_return return_,
    struct ast_fmt_params params
);

void ast_write_render(
    struct ast_write write,
    struct ast_fmt_params params
);

void ast_scalar_var_assign_render(
    struct ast_scalar_var_assign scalar_var_assign,
    struct ast_fmt_params params
);

void ast_subscripted_assign_render(
    struct ast_subscripted_assign subscripted_assign,
    struct ast_fmt_params params
);

void ast_write_argument_render(
    struct ast_write_argument write_argument,
    struct ast_fmt_params params
);

void ast_write_argument_list_render(
    struct ast_write_argument_list write_argument_list,
    struct ast_fmt_params params
);

void ast_statement_list_render(
    struct ast_statement_list statement_list,
    struct ast_fmt_params params
);

void ast_statement_render(
    struct ast_statement statement,
    struct ast_fmt_params params
);

void datatype_render(
    enum datatype datatype,
    struct ast_fmt_params params
);

void ast_parameter_render(
    struct ast_parameter parameter,
    struct ast_fmt_params params
);

void ast_parameter_list_render(
    struct ast_parameter_list parameter_list,
    struct ast_fmt_params params
);

void ast_function_decl_render(
    struct ast_function_decl function_decl,
    struct ast_fmt_params params
);

void ast_scalar_var_decl_render(
    struct ast_scalar_var_decl scalar_var_decl,
    struct ast_fmt_params params
);

void ast_vector_var_decl_render(
    struct ast_vector_var_decl vector_var_decl,
    struct ast_fmt_params params
);

void ast_declaration_render(
    struct ast_declaration declaration,
    struct ast_fmt_params params
);

void ast_declaration_list_render(
    struct ast_declaration_list declaration_list,
    struct ast_fmt_params params
);

void ast_render(struct ast ast, struct ast_fmt_params params);

/**
 * *********************
 * *********************
 * ******* INIT ********
 * *********************
 * *********************
 */

struct ast_expression ast_expression_base_init(void);

struct ast_statement ast_statement_base_init(void);

struct ast_declaration ast_declaration_base_init(void);

/**
 * *********************
 * *********************
 * ******* FREE *******
 * *********************
 * *********************
 */

void ast_free(struct ast ast);

void ast_declaration_list_free(struct ast_declaration_list declaration_list);

void ast_declaration_free(struct ast_declaration declaration);

void ast_scalar_var_decl_free(struct ast_scalar_var_decl scalar_var_decl);

void ast_vector_var_decl_free(struct ast_vector_var_decl vector_var_decl);

void ast_function_decl_free(struct ast_function_decl function_decl);

void ast_parameter_list_free(struct ast_parameter_list parameter_list);

void ast_expression_list_free(struct ast_expression_list expression_list);

void ast_expression_free(struct ast_expression expression);

void ast_subscription_free(struct ast_subscription subscription);

void ast_binary_operation_free(struct ast_binary_operation binary_operation);

void ast_unary_operation_free(struct ast_unary_operation unary_operation);

void ast_function_call_free(struct ast_function_call function_call);

void ast_body_free(struct ast_body body);

void ast_statement_list_free(struct ast_statement_list statement_list);

void ast_statement_free(struct ast_statement statement);

void ast_scalar_var_assign_free(struct ast_scalar_var_assign scalar_var_assign);

void ast_subscripted_assign_free(
    struct ast_subscripted_assign subscripted_assign
);

void ast_if_free(struct ast_if if_);

void ast_while_free(struct ast_while while_);

void ast_return_free(struct ast_return return_);

void ast_write_free(struct ast_write write);

void ast_write_argument_free(struct ast_write_argument write_argument);

void ast_write_argument_list_free(
    struct ast_write_argument_list write_argument_list
);

/**
 * *********************
 * *********************
 * ****** HELPERS ******
 * *********************
 * *********************
 */

struct ast_expression ast_create_binary_operation(
    struct ast_expression left_operand,
    enum ast_binary_operator operator,
    struct ast_expression right_operand
);

struct ast_expression ast_create_unary_operation(
    enum ast_unary_operator operator,
    struct ast_expression operand
);

int ast_body_returns(struct ast_body body);

int ast_statement_returns(struct ast_statement statement);

/**
 * *********************
 * *********************
 * ****** GLOBAL *******
 * *********************
 * *********************
 */

extern struct ast g_ast;

#endif
