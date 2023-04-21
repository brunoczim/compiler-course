#include <stdlib.h>
#include "ast.h"
#include "lexer.h"
#include "alloc.h"

struct ast g_ast;

static int expression_needs_paren(struct ast_expression expression);

static void paren_expression_render(
    struct ast_expression expression,
    struct ast_fmt_params params
);

void ast_write_indent(struct ast_fmt_params params)
{
    size_t level;
    size_t spaces;

    for (level = 0; level < params.level; level++) {
        for (spaces = 0; spaces < params.spaces_per_level;  spaces++) {
            fputc(' ', params.output);
        }
    }
}

void ast_binary_operator_render(
    enum ast_binary_operator binary_operator,
    struct ast_fmt_params params
)
{
    switch (binary_operator) {
        case AST_ADD:
            fputs("+", params.output);
            break;
        case AST_SUB:
            fputs("-", params.output);
            break;
        case AST_MUL:
            fputs("*", params.output);
            break;
        case AST_DIV:
            fputs("/", params.output);
            break;
        case AST_LESS_THAN:
            fputs("<", params.output);
            break;
        case AST_GREATER_THAN:
            fputs(">", params.output);
            break;
        case AST_LESS_OR_EQUALS:
            fputs("<=", params.output);
            break;
        case AST_GREATER_OR_EQUALS:
            fputs(">=", params.output);
            break;
        case AST_EQUALS:
            fputs("==", params.output);
            break;
        case AST_NOT_EQUAL:
            fputs("!=", params.output);
            break;
        case AST_AND:
            fputs("&", params.output);
            break;
        case AST_OR:
            fputs("|", params.output);
            break;
    }
}

void ast_binary_operation_render(
    struct ast_binary_operation binary_operation,
    struct ast_fmt_params params
)
{
    paren_expression_render(*binary_operation.left_operand, params);
    fputs(" ", params.output);
    ast_binary_operator_render(binary_operation.operator, params);
    fputs(" ", params.output);
    paren_expression_render(*binary_operation.right_operand, params);
}

void ast_unary_operator_render(
    enum ast_unary_operator unary_operator,
    struct ast_fmt_params params
)
{
    switch (unary_operator) {
        case AST_NOT:
            fputs("~", params.output);
            break;
    }
}

void ast_unary_operation_render(
    struct ast_unary_operation unary_operation,
    struct ast_fmt_params params
)
{
    ast_unary_operator_render(unary_operation.operator, params);
    paren_expression_render(*unary_operation.operand, params);
}

void ast_function_call_render(
    struct ast_function_call function_call,
    struct ast_fmt_params params
)
{
    fputs(function_call.function->content, params.output);
    fputs("(", params.output);
    ast_expression_list_render(function_call.argument_list, params);
    fputs(")", params.output);
}

void ast_variable_render(
    struct ast_variable variable,
    struct ast_fmt_params params
)
{
    fputs(variable.name->content, params.output);
}

void ast_subscription_render(
    struct ast_subscription subscription,
    struct ast_fmt_params params
)
{
    fputs(subscription.variable->content, params.output);
    fputs("[", params.output);
    ast_expression_render(*subscription.index, params);
    fputs("]", params.output);
}

void ast_expression_list_render(
    struct ast_expression_list expression_list,
    struct ast_fmt_params params
)
{
    size_t i;

    char const *padding = "";

    for (i = 0; i < expression_list.length; i++) {
        fputs(padding, params.output);
        paren_expression_render(expression_list.expressions[i], params);
        padding = " ";
    }
}

static int expression_needs_paren(struct ast_expression expression)
{
    switch (expression.tag) {
        case AST_VARIABLE:
        case AST_SUBSCRIPTION:
        case AST_INPUT:
        case AST_INT_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_CHAR_LITERAL:
            return 0;
        default:
            return 1;
    }
}

static void paren_expression_render(
    struct ast_expression expression,
    struct ast_fmt_params params
)
{
    int needs_paren = expression_needs_paren(expression);
    if (needs_paren) {
        fputs("(", params.output);
    }
    ast_expression_render(expression, params);
    if (needs_paren) {
        fputs(")", params.output);
    }
}

void ast_expression_render(
    struct ast_expression expression,
    struct ast_fmt_params params
)
{
    switch (expression.tag) {
        case AST_INT_LITERAL:
            fputs(expression.data.literal->content, params.output);
            break;
        case AST_CHAR_LITERAL:
            fputs(expression.data.literal->content, params.output);
            break;
        case AST_FLOAT_LITERAL:
            fputs(expression.data.literal->content, params.output);
            break;
        case AST_SUBSCRIPTION:
            ast_subscription_render(expression.data.subscription, params);
            break;
        case AST_VARIABLE:
            fputs(expression.data.variable.name->content, params.output);
            break;
        case AST_BINARY_OPERATION:
            ast_binary_operation_render(
                expression.data.binary_operation,
                params
            );
            break;
        case AST_UNARY_OPERATION:
            ast_unary_operation_render(expression.data.unary_operation, params);
            break;
        case AST_FUNCTION_CALL:
            ast_function_call_render(expression.data.function_call, params);
            break;
        case AST_INPUT:
            fputs("entrada", params.output);
            break;
    }
}

void ast_body_render(
    struct ast_body body,
    struct ast_fmt_params params
)
{
    fputs("{\n", params.output);

    params.level += 1;
    ast_statement_list_render(body.statement_list, params);
    params.level -= 1;

    ast_write_indent(params);
    fputs("}", params.output);
}

void ast_if_render(
    struct ast_if if_,
    struct ast_fmt_params params
)
{
    fputs("entaum\n", params.output);
    if (if_.then != NULL) {
        ast_write_indent(params);
        params.level += if_.then->tag != AST_BODY;
        ast_statement_render(*if_.then, params);
        params.level -= if_.then->tag != AST_BODY;
    }

    if (if_.else_ != NULL) {
        fputs("\n", params.output);
        ast_write_indent(params);
        fputs("senaum\n", params.output);
        params.level += if_.else_->tag != AST_BODY;
        ast_write_indent(params);
        ast_statement_render(*if_.else_, params);
        params.level -= if_.else_->tag != AST_BODY;
    }

    fputs(" se (", params.output);
    ast_expression_render(if_.condition, params);
    fputs(")", params.output);
}

void ast_while_render(
    struct ast_while while_,
    struct ast_fmt_params params
)
{
    if (while_.do_ != NULL) {
        params.level += while_.do_->tag != AST_BODY;
        ast_statement_render(*while_.do_, params);
        params.level -= while_.do_->tag != AST_BODY;
    }

    fputs(" enquanto (", params.output);
    ast_expression_render(while_.condition, params);
    fputs(")", params.output);
}

void ast_return_render(
    struct ast_return return_,
    struct ast_fmt_params params
)
{
    fputs("retorne ", params.output);
    ast_expression_render(return_.returned_value, params);
}

void ast_scalar_var_assign_render(
    struct ast_scalar_var_assign scalar_var_assign,
    struct ast_fmt_params params
)
{
    fputs(scalar_var_assign.variable->content, params.output);
    fputs(" = ", params.output);
    ast_expression_render(scalar_var_assign.assigned_value, params);
}

void ast_subscripted_assign_render(
    struct ast_subscripted_assign subscripted_assign,
    struct ast_fmt_params params
)
{
    fputs(subscripted_assign.variable->content, params.output);
    fputs("[", params.output);
    ast_expression_render(subscripted_assign.index, params);
    fputs("] = ", params.output);
    ast_expression_render(subscripted_assign.assigned_value, params);
}

void ast_write_argument_render(
    struct ast_write_argument write_argument,
    struct ast_fmt_params params
)
{
    switch (write_argument.tag) {
        case AST_WRITE_EXPRESSION:
            paren_expression_render(write_argument.data.expression, params);
            break;
        case AST_WRITE_STRING_LIT:
            fputs(
                write_argument.data.string_literal->content,
                params.output
            );
            break;
    }
}

void ast_write_argument_list_render(
    struct ast_write_argument_list write_argument_list,
    struct ast_fmt_params params
)
{
    size_t i;

    char const *padding = "";

    for (i = 0; i < write_argument_list.length; i++) {
        fputs(padding, params.output);
        ast_write_argument_render(
            write_argument_list.write_arguments[i],
            params
        );
        padding = " ";
    }
}

void ast_write_render(
    struct ast_write write,
    struct ast_fmt_params params
)
{
    fputs("escreva ", params.output);
    ast_write_argument_list_render(write.argument_list, params);
}

void ast_statement_list_render(
    struct ast_statement_list statement_list,
    struct ast_fmt_params params
)
{
    size_t i;

    for (i = 0; i < statement_list.length; i++) {
        ast_write_indent(params);
        ast_statement_render(statement_list.statements[i], params);
        fputs(";\n", params.output);
    }
}

void ast_statement_render(
    struct ast_statement statement,
    struct ast_fmt_params params
)
{
    switch (statement.tag) {
        case AST_SCALAR_VAR_ASSIGN:
            ast_scalar_var_assign_render(
                statement.data.scalar_var_assign,
                params
            );
            break;
        case AST_SUBSCRIPTED_ASSIGN:
            ast_subscripted_assign_render(
                statement.data.subscripted_assign,
                params
            );
            break;
        case AST_IF:
            ast_if_render(statement.data.if_, params);
            break;
        case AST_WHILE:
            ast_while_render(statement.data.while_, params);
            break;
        case AST_WRITE:
            ast_write_render(statement.data.write, params);
            break;
        case AST_RETURN:
            ast_return_render(statement.data.return_, params);
            break;
        case AST_BODY:
            ast_body_render(statement.data.body, params);
            break;
        case AST_EXPRESSION_STATEMENT:
            ast_expression_render(statement.data.expression, params);
            break;
    }
}

void datatype_render(
    enum datatype datatype,
    struct ast_fmt_params params
)
{
    switch (datatype) {
        case DATATYPE_INTE:
            fputs("inte", params.output);
            break;
        case DATATYPE_REAL:
            fputs("real", params.output);
            break;
        case DATATYPE_CARA:
            fputs("cara", params.output);
            break;
    }
}

void ast_parameter_render(
    struct ast_parameter parameter,
    struct ast_fmt_params params
)
{
    datatype_render(parameter.datatype, params);
    fputs(" ", params.output);
    fputs(parameter.name->content, params.output);
}

void ast_parameter_list_render(
    struct ast_parameter_list parameter_list,
    struct ast_fmt_params params
)
{
    size_t i;

    char const *padding = "";

    for (i = 0; i < parameter_list.length; i++) {
        fputs(padding, params.output);
        ast_parameter_render(
            parameter_list.parameters[i],
            params
        );
        padding = " ";
    }
}

void ast_function_decl_render(
    struct ast_function_decl function_decl,
    struct ast_fmt_params params
)
{
    ast_write_indent(params);
    datatype_render(function_decl.return_datatype, params);
    fputs(" ", params.output);
    fputs(function_decl.name->content, params.output);
    fputs("(", params.output);
    ast_parameter_list_render(function_decl.parameter_list, params);
    fputs(")\n", params.output);
    ast_write_indent(params);
    ast_body_render(function_decl.body, params);
    fputs("\n", params.output);
}

void ast_scalar_var_decl_render(
    struct ast_scalar_var_decl scalar_var_decl,
    struct ast_fmt_params params
)
{
    ast_write_indent(params);
    datatype_render(scalar_var_decl.datatype, params);
    fputs(" ", params.output);
    fputs(scalar_var_decl.name->content, params.output);
    fputs(" = ", params.output);
    ast_expression_render(scalar_var_decl.init, params);
    fputs(";\n", params.output);
}

void ast_vector_var_decl_render(
    struct ast_vector_var_decl vector_var_decl,
    struct ast_fmt_params params
)
{
    ast_write_indent(params);
    datatype_render(vector_var_decl.datatype, params);
    fputs(" ", params.output);
    fputs(vector_var_decl.name->content, params.output);
    fputs("[", params.output);
    ast_expression_render(vector_var_decl.length, params);
    fputs("]", params.output);
    if (vector_var_decl.init.length > 0) {
        fputs(" ", params.output);
    }
    ast_expression_list_render(vector_var_decl.init, params);
    fputs(";\n", params.output);
}

void ast_declaration_render(
    struct ast_declaration declaration,
    struct ast_fmt_params params
)
{
    switch (declaration.tag) {
        case AST_SCALAR_VAR_DECL:
            ast_scalar_var_decl_render(declaration.data.scalar_var, params);
            break;
        case AST_VECTOR_VAR_DECL:
            ast_vector_var_decl_render(declaration.data.vector_var, params);
            break;
        case AST_FUNCTION_DECL:
            ast_function_decl_render(declaration.data.function, params);
            break;
    }
}

void ast_declaration_list_render(
    struct ast_declaration_list declaration_list,
    struct ast_fmt_params params
)
{
    size_t i;

    char const *padding = "";

    for (i = 0; i < declaration_list.length; i++) {
        fputs(padding, params.output);
        ast_declaration_render(
            declaration_list.declarations[i],
            params
        );
        padding = "\n";
    }
}

void ast_render(struct ast ast, struct ast_fmt_params params)
{
    ast_declaration_list_render(ast.declaration_list, params);
}

struct ast_expression ast_expression_base_init(void)
{
    struct ast_expression expression;
    expression.line_number = getLineNumber();
    expression.inference_status = INFERENCE_UNKNOWN;
    return expression;
}

struct ast_statement ast_statement_base_init(void)
{
    struct ast_statement statement;
    statement.line_number = getLineNumber();
    return statement;
}

struct ast_declaration ast_declaration_base_init(void)
{
    struct ast_declaration declaration;
    declaration.line_number = getLineNumber();
    return declaration;
}

void ast_free(struct ast ast)
{
    if (ast.is_valid) {
        ast_declaration_list_free(ast.declaration_list);
    }
}

void ast_declaration_list_free(struct ast_declaration_list declaration_list)
{
    size_t i;
    for (i = 0; i < declaration_list.length; i++) {
        ast_declaration_free(declaration_list.declarations[i]);
    }
    free(declaration_list.declarations);
}

void ast_declaration_free(struct ast_declaration declaration)
{
    switch (declaration.tag) {
        case AST_SCALAR_VAR_DECL:
            ast_scalar_var_decl_free(declaration.data.scalar_var);
            break;
        case AST_VECTOR_VAR_DECL:
            ast_vector_var_decl_free(declaration.data.vector_var);
            break;
        case AST_FUNCTION_DECL:
            ast_function_decl_free(declaration.data.function);
            break;
    }
}

void ast_scalar_var_decl_free(struct ast_scalar_var_decl scalar_var_decl)
{
    ast_expression_free(scalar_var_decl.init);
}

void ast_vector_var_decl_free(struct ast_vector_var_decl vector_var_decl)
{
    ast_expression_free(vector_var_decl.length);
    ast_expression_list_free(vector_var_decl.init);
}

void ast_function_decl_free(struct ast_function_decl function_decl)
{
    ast_parameter_list_free(function_decl.parameter_list);
    ast_body_free(function_decl.body);
}

void ast_parameter_list_free(struct ast_parameter_list parameter_list)
{
    free(parameter_list.parameters);
}

void ast_expression_list_free(struct ast_expression_list expression_list)
{
    size_t i;
    for (i = 0; i < expression_list.length; i++) {
        ast_expression_free(expression_list.expressions[i]);
    }
    free(expression_list.expressions);
}

void ast_expression_free(struct ast_expression expression)
{
    switch (expression.tag) {
        case AST_INT_LITERAL:
        case AST_CHAR_LITERAL:
        case AST_FLOAT_LITERAL:
        case AST_VARIABLE:
        case AST_INPUT:
            break;
        case AST_SUBSCRIPTION:
            ast_subscription_free(expression.data.subscription);
            break;
        case AST_BINARY_OPERATION:
            ast_binary_operation_free(expression.data.binary_operation);
            break;
        case AST_UNARY_OPERATION:
            ast_unary_operation_free(expression.data.unary_operation);
            break;
        case AST_FUNCTION_CALL:
            ast_function_call_free(expression.data.function_call);
            break;
    }
}


void ast_subscription_free(struct ast_subscription subscription)
{
    ast_expression_free(*subscription.index);
    free(subscription.index);
}

void ast_binary_operation_free(struct ast_binary_operation binary_operation)
{
    ast_expression_free(*binary_operation.left_operand);
    ast_expression_free(*binary_operation.right_operand);
    free(binary_operation.left_operand);
    free(binary_operation.right_operand);
}

void ast_unary_operation_free(struct ast_unary_operation unary_operation)
{
    ast_expression_free(*unary_operation.operand);
    free(unary_operation.operand);
}

void ast_function_call_free(struct ast_function_call function_call)
{
    ast_expression_list_free(function_call.argument_list);
}

void ast_body_free(struct ast_body body)
{
    ast_statement_list_free(body.statement_list);
}

void ast_statement_list_free(struct ast_statement_list statement_list)
{
    size_t i;
    for (i = 0; i < statement_list.length; i++) {
        ast_statement_free(statement_list.statements[i]);
    }
    free(statement_list.statements);
}

void ast_statement_free(struct ast_statement statement)
{
    switch (statement.tag) {
        case AST_SCALAR_VAR_ASSIGN:
            ast_scalar_var_assign_free(statement.data.scalar_var_assign);
            break;
        case AST_SUBSCRIPTED_ASSIGN:
            ast_subscripted_assign_free(statement.data.subscripted_assign);
            break;
        case AST_IF:
            ast_if_free(statement.data.if_);
            break;
        case AST_WHILE:
            ast_while_free(statement.data.while_);
            break;
        case AST_WRITE:
            ast_write_free(statement.data.write);
            break;
        case AST_RETURN:
            ast_return_free(statement.data.return_);
            break;
        case AST_BODY:
            ast_body_free(statement.data.body);
            break;
        case AST_EXPRESSION_STATEMENT:
            ast_expression_free(statement.data.expression);
            break;
    }
}

void ast_scalar_var_assign_free(struct ast_scalar_var_assign scalar_var_assign)
{
    ast_expression_free(scalar_var_assign.assigned_value);
}

void ast_subscripted_assign_free(
    struct ast_subscripted_assign subscripted_assign
)
{
    ast_expression_free(subscripted_assign.index);
    ast_expression_free(subscripted_assign.assigned_value);
}

void ast_if_free(struct ast_if if_)
{
    if (if_.then != NULL) {
        ast_statement_free(*if_.then);
    }
    if (if_.else_ != NULL) {
        ast_statement_free(*if_.else_);
    }
    ast_expression_free(if_.condition);
    free(if_.then);
    free(if_.else_);
}

void ast_while_free(struct ast_while while_)
{
    if (while_.do_ != NULL) {
        ast_statement_free(*while_.do_);
    }
    ast_expression_free(while_.condition);
    free(while_.do_);
}

void ast_return_free(struct ast_return return_)
{
    ast_expression_free(return_.returned_value);
}

void ast_write_free(struct ast_write write)
{
    ast_write_argument_list_free(write.argument_list);
}

void ast_write_argument_free(struct ast_write_argument write_argument)
{
    switch (write_argument.tag) {
        case AST_WRITE_STRING_LIT:
            break;
        case AST_WRITE_EXPRESSION:
            ast_expression_free(write_argument.data.expression);
            break;
    }
}

void ast_write_argument_list_free(
    struct ast_write_argument_list write_argument_list
)
{
    size_t i;
    for (i = 0; i < write_argument_list.length; i++) {
        ast_write_argument_free(write_argument_list.write_arguments[i]);
    }
    free(write_argument_list.write_arguments);
}

struct ast_expression ast_create_binary_operation(
    struct ast_expression left_operand,
    enum ast_binary_operator binary_operator,
    struct ast_expression right_operand
)
{
    struct ast_expression expression = ast_expression_base_init();
    expression.tag = AST_BINARY_OPERATION;
    expression.data.binary_operation.operator = binary_operator;
    expression.data.binary_operation.left_operand =
        aborting_malloc(sizeof(left_operand));
    *expression.data.binary_operation.left_operand = left_operand;
    expression.data.binary_operation.right_operand =
        aborting_malloc(sizeof(right_operand));
    *expression.data.binary_operation.right_operand = right_operand;
    return expression;
}

struct ast_expression ast_create_unary_operation(
    enum ast_unary_operator unary_operator,
    struct ast_expression operand
)
{
    struct ast_expression expression = ast_expression_base_init();
    expression.tag = AST_UNARY_OPERATION;
    expression.data.unary_operation.operator = unary_operator;
    expression.data.unary_operation.operand = aborting_malloc(sizeof(operand));
    *expression.data.unary_operation.operand = operand;
    return expression;
}

int ast_body_returns(struct ast_body body)
{
    size_t i;
    int returns; 

    returns = 0;
    i = body.statement_list.length;

    while (i > 0 && !returns) {
        i--;
        returns = ast_statement_returns(body.statement_list.statements[i]);
    }

    return returns;
}

int ast_statement_returns(struct ast_statement statement)
{
    switch (statement.tag) {
        case AST_RETURN:
            return 1;
        case AST_BODY:
            return ast_body_returns(statement.data.body);
        case AST_IF:
            return statement.data.if_.then != NULL
                && statement.data.if_.else_ != NULL
                && ast_statement_returns(*statement.data.if_.then)
                && ast_statement_returns(*statement.data.if_.else_);
        default:
            return 0;
    }
}
