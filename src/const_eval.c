#include "const_eval.h"
#include "panic.h"

int const_eval_int_expression(struct ast_expression expression, long *output)
{
    long left_operand;
    long right_operand;
    long operand;

    switch (expression.tag) {
        case AST_INT_LITERAL:
            *output = expression.data.literal->data.parsed_int;
            return 1;
        case AST_CHAR_LITERAL:
            *output = expression.data.literal->data.parsed_char;
            return 1;
        case AST_BINARY_OPERATION:
            if (
                const_eval_int_expression(
                    *expression.data.binary_operation.left_operand,
                    &left_operand
                )
                &&
                const_eval_int_expression(
                    *expression.data.binary_operation.right_operand,
                    &right_operand
                )
            ) {
                switch (expression.data.binary_operation.operator) {
                    case AST_ADD:
                        *output = left_operand + right_operand;
                        return 1;
                    case AST_SUB:
                        *output = left_operand - right_operand;
                        return 1;
                    case AST_MUL:
                        *output = left_operand * right_operand;
                        return 1;
                    case AST_DIV:
                        *output = left_operand / right_operand;
                        return 1;
                    default:
                        return 0;
                }
            }
            break;
        case AST_UNARY_OPERATION:
            if (
                const_eval_int_expression(
                    *expression.data.unary_operation.operand,
                    &operand
                )
            ) {
                switch (expression.data.unary_operation.operator) {
                    default: return 0;
                }
            }
            break;
        case AST_INPUT:
        case AST_FLOAT_LITERAL:
        case AST_FUNCTION_CALL:
        case AST_SUBSCRIPTION:
        case AST_VARIABLE:
            return 0;
    }

    panic(
        "expression tag %i's const evaluation to integer is not implemented",
        expression.tag
    );
}

int const_eval_char_expression(struct ast_expression expression, char *output)
{
    long long_value;
    if (const_eval_int_expression(expression, &long_value)) {
        *output = long_value;
        return 1;
    }
    return 0;
}

int const_eval_float_expression(
    struct ast_expression expression,
    double *output
)
{
    double left_operand;
    double right_operand;
    double operand;

    switch (expression.tag) {
        case AST_FLOAT_LITERAL:
            *output = expression.data.literal->data.float_.parsed;
            return 1;
        case AST_BINARY_OPERATION:
            if (
                const_eval_float_expression(
                    *expression.data.binary_operation.left_operand,
                    &left_operand
                )
                &&
                const_eval_float_expression(
                    *expression.data.binary_operation.right_operand,
                    &right_operand
                )
            ) {
                switch (expression.data.binary_operation.operator) {
                    case AST_ADD:
                        *output = left_operand + right_operand;
                        return 1;
                    case AST_SUB:
                        *output = left_operand - right_operand;
                        return 1;
                    case AST_MUL:
                        *output = left_operand * right_operand;
                        return 1;
                    case AST_DIV:
                        *output = left_operand / right_operand;
                        return 1;
                    default:
                        return 0;
                }
            }
            break;
        case AST_UNARY_OPERATION:
            if (
                const_eval_float_expression(
                    *expression.data.unary_operation.operand,
                    &operand
                )
            ) {
                switch (expression.data.unary_operation.operator) {
                    default: return 0;
                }
            }
            break;
        case AST_INT_LITERAL:
        case AST_CHAR_LITERAL:
        case AST_FUNCTION_CALL:
        case AST_SUBSCRIPTION:
        case AST_VARIABLE:
        case AST_INPUT:
            return 0;
    }
    panic(
        "expression tag %i's const evaluation to float is not implemented",
        expression.tag
    );
}
