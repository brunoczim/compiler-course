#ifndef SEMANTICS_H_
#define SEMANTICS_H_ 1

#include "ast.h"

enum operation_type {
    OPERATION_ARITHMETIC,
    OPERATION_COMPARISON,
    OPERATION_LOGICAL
};

struct semantic_error_params {
    FILE *output;
    unsigned *error_count;
};

void semantic_check_program(
    struct ast *ast,
    struct semantic_error_params params
);

void semantic_check_declaration_init(
    struct ast_declaration *declaration,
    struct semantic_error_params params
);

void semantic_check_statement(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

enum semantic_type const *expression_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

void print_semantic_type_mismatch(
    enum semantic_type expected_type,
    enum semantic_type found_type,
    int line_number,
    struct semantic_error_params params
);

void print_inv_semantic_type_mismatch(
    enum semantic_type unexpected_type,
    int line_number,
    struct semantic_error_params params
);

void print_symbol_type_mismatch(
    enum symbol_type expected_type,
    struct symbol found_symbol,
    int line_number,
    struct semantic_error_params params
);

void print_symbol_not_in_scope(
    struct symbol symbol,
    int line_number,
    struct semantic_error_params params
);

void print_index_must_be_int_char(
    enum semantic_type found,
    int line_number,
    struct semantic_error_params params
);

void print_index_must_be_constant(
    int line_number,
    struct semantic_error_params params
);

void print_argument_number_mismatch(
    size_t expected,
    size_t given,
    int line_number,
    struct semantic_error_params params
);

void print_vector_element_number_mismatch(
    size_t expected,
    size_t given,
    int line_number,
    struct semantic_error_params params
);

void print_redeclared_symbol(
    struct symbol symbol,
    int line_number,
    struct semantic_error_params params
);

enum operation_type binary_operation_type(enum ast_binary_operator operator);

enum operation_type unary_operation_type(enum ast_unary_operator operator);

#endif
