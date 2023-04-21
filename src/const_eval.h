#ifndef EVAL_H_
#define EVAL_H_ 1

#include "ast.h"

int const_eval_int_expression(struct ast_expression expression, long *output);

int const_eval_char_expression(struct ast_expression expression, char *output);

int const_eval_float_expression(
    struct ast_expression expression,
    double *output
);

#endif
