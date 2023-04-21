#include "semantics.h"
#include "const_eval.h"
#include "alloc.h"
#include "panic.h"

static void infer_expression_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

static void infer_binop_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

static void infer_unop_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

static void infer_variable_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

static void infer_subscription_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

static void infer_func_call_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
);

static void semantic_check_var_assign(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_subscripted_assign(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_if(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_while(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_return(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_body(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_expr_stmt(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_write(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
);

static void semantic_check_scalar_var_decl(
    struct ast_declaration *declaration,
    struct semantic_error_params params
);

static void semantic_check_vector_var_decl(
    struct ast_declaration *declaration,
    struct semantic_error_params params
);

static void semantic_check_function(
    struct ast_function_decl *function,
    struct semantic_error_params params
);

static void fill_symbol_table_for_decl(
    struct ast_declaration declaration,
    struct semantic_error_params params
);

void semantic_check_program(
    struct ast *ast,
    struct semantic_error_params params
)
{
    size_t i;

    for (i = 0; i < ast->declaration_list.length; i++) {
        fill_symbol_table_for_decl(
            ast->declaration_list.declarations[i],
            params
        );
    }

    for (i = 0; i < ast->declaration_list.length; i++) {
        semantic_check_declaration_init(
            &ast->declaration_list.declarations[i],
            params
        );
    }
    
    for (i = 0; i < ast->declaration_list.length; i++) {
        if (ast->declaration_list.declarations[i].tag == AST_FUNCTION_DECL) {
            semantic_check_function(
                &ast->declaration_list.declarations[i].data.function,
                params
            );
        }
    }
}

static void fill_symbol_table_for_decl(
    struct ast_declaration declaration,
    struct semantic_error_params params
)
{
    size_t i;

    switch (declaration.tag) {
        case AST_FUNCTION_DECL:
            if (declaration.data.function.name->type != SYM_UNKNOWN_IDENT) {
                print_redeclared_symbol(
                    *declaration.data.function.name,
                    declaration.line_number,
                    params
                );
            } else {
                declaration.data.function.name->type = SYM_FUNCTION;
                declaration.data.function.name->data.function.return_type
                    = declaration.data.function.return_datatype;
                declaration
                    .data
                    .function.name->data.function.parameter_types.length
                    =
                    declaration.data.function.parameter_list.length;
                declaration
                    .data
                    .function.name->data.function.parameter_types.types
                    =
                    aborting_malloc(
                        sizeof(enum datatype)
                        * declaration.data.function.parameter_list.length
                    );
                for (
                    i = 0;
                    i < declaration
                        .data
                        .function.name->data.function.parameter_types.length;
                    i++
                ) {
                    if (
                        declaration
                            .data
                            .function
                            .parameter_list
                            .parameters[i].name->type != SYM_UNKNOWN_IDENT
                    ) {
                        print_redeclared_symbol(
                            *declaration
                                .data
                                .function.parameter_list.parameters[i].name,
                            declaration
                                .data
                                .function
                                .parameter_list.parameters[i].line_number,
                            params
                        );
                    } else {
                        declaration
                            .data
                            .function
                            .parameter_list
                            .parameters[i].name->type = SYM_SCALAR_VAR;
                        declaration
                            .data
                            .function
                            .parameter_list
                            .parameters[i].name->data.variable.type
                            = 
                            declaration
                            .data
                            .function
                            .parameter_list.parameters[i].datatype;
                        declaration
                            .data
                            .function
                            .parameter_list
                            .parameters[i].name->data.variable.in_scope = 0;
                    }
                    declaration
                        .data
                        .function.name->data.function.parameter_types.types[i]
                        = declaration
                            .data
                            .function.parameter_list.parameters[i].datatype;
                }
            }
            break;
        case AST_SCALAR_VAR_DECL:

            if (declaration.data.scalar_var.name->type != SYM_UNKNOWN_IDENT) {
                print_redeclared_symbol(
                    *declaration.data.function.name,
                    declaration.line_number,
                    params
                );
            } else {
                declaration.data.scalar_var.name->type = SYM_SCALAR_VAR;
                declaration.data.scalar_var.name->data.variable.in_scope = 1;
                declaration.data.scalar_var.name->data.variable.type
                    = declaration.data.scalar_var.datatype;
            }
            break;
        case AST_VECTOR_VAR_DECL:
            if (declaration.data.vector_var.name->type != SYM_UNKNOWN_IDENT) {
                print_redeclared_symbol(
                    *declaration.data.function.name,
                    declaration.line_number,
                    params
                );
            } else {
                declaration.data.vector_var.name->type = SYM_VECTOR_VAR;
                declaration.data.vector_var.name->data.variable.in_scope = 1;
                declaration.data.vector_var.name->data.variable.type
                    = declaration.data.scalar_var.datatype;
            }
            break;
    }
}

static void semantic_check_function(
    struct ast_function_decl *function,
    struct semantic_error_params params
)
{
    size_t i;

    for (i = 0; i < function->parameter_list.length; i++) {
        function->parameter_list.parameters[i].name->data.variable.in_scope = 1;
    }

    for (i = 0; i < function->body.statement_list.length; i++) {
        semantic_check_statement(
            &function->body.statement_list.statements[i],
            datatype_to_semantic_type(function->return_datatype),
            params
        );
    }

    for (i = 0; i < function->parameter_list.length; i++) {
        function->parameter_list.parameters[i].name->data.variable.in_scope = 0;
    }
}

void semantic_check_declaration_init(
    struct ast_declaration *declaration,
    struct semantic_error_params params
)
{
    switch (declaration->tag) {
        case AST_SCALAR_VAR_DECL:
            semantic_check_scalar_var_decl(declaration, params);
            break;
        case AST_VECTOR_VAR_DECL:
            semantic_check_vector_var_decl(declaration, params);
            break;
        case AST_FUNCTION_DECL:
            break;
    }
}

void semantic_check_statement(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    switch (statement->tag) {
        case AST_SCALAR_VAR_ASSIGN:
            semantic_check_var_assign(statement, expected_return, params);
            break;
        case AST_SUBSCRIPTED_ASSIGN:
            semantic_check_subscripted_assign(
                statement,
                expected_return,
                params
            );
            break;
        case AST_IF:
            semantic_check_if(statement, expected_return, params);
            break;
        case AST_WHILE:
            semantic_check_while(statement, expected_return, params);
            break;
        case AST_WRITE:
            semantic_check_write(statement, expected_return, params);
            break;
        case AST_RETURN:
            semantic_check_return(statement, expected_return, params);
            break;
        case AST_BODY:
            semantic_check_body(statement, expected_return, params);
            break;
        case AST_EXPRESSION_STATEMENT:
            semantic_check_expr_stmt(statement, expected_return, params);
            break;
    }
}

static void semantic_check_var_assign(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    enum semantic_type variable_type;
    enum semantic_type const *assigned_type;

    if (statement->data.scalar_var_assign.variable->type != SYM_SCALAR_VAR) {
        print_symbol_type_mismatch(
            SYM_SCALAR_VAR,
            *statement->data.scalar_var_assign.variable,
            statement->line_number,
            params
        );
    } else {
        if (
            !statement->data.scalar_var_assign.variable->data.variable.in_scope
        ) {
            print_symbol_not_in_scope(
                *statement->data.scalar_var_assign.variable,
                statement->line_number,
                params
            );
        }

        variable_type = datatype_to_semantic_type(
            statement->data.scalar_var_assign.variable->data.variable.type
        );
        assigned_type = expression_semantic_type(
            &statement->data.scalar_var_assign.assigned_value,
            params
        );
        if (
            assigned_type != NULL
            && !semantic_type_equiv(*assigned_type, variable_type)
        ) {
            print_semantic_type_mismatch(
                variable_type,
                *assigned_type,
                statement->data.scalar_var_assign.assigned_value.line_number,
                params
            );
        }
    }
}

static void semantic_check_subscripted_assign(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    enum semantic_type variable_type;
    enum semantic_type const *assigned_type;
    enum semantic_type const *index_type;
    
    if (statement->data.subscripted_assign.variable->type != SYM_VECTOR_VAR) {
        print_symbol_type_mismatch(
            SYM_VECTOR_VAR,
            *statement->data.subscripted_assign.variable,
            statement->line_number,
            params
        );
    } else {
        if (
            !statement->data.subscripted_assign.variable->data.variable.in_scope
        ) {
            print_symbol_not_in_scope(
                *statement->data.subscripted_assign.variable,
                statement->line_number,
                params
            );
        }

        variable_type = datatype_to_semantic_type(
            statement->data.subscripted_assign.variable->data.variable.type
        );
        assigned_type = expression_semantic_type(
            &statement->data.subscripted_assign.assigned_value,
            params
        );
        index_type = expression_semantic_type(
            &statement->data.subscripted_assign.index,
            params
        );

        if (
            index_type != NULL
            && !semantic_type_equiv(*index_type, SEMANTIC_INT)
        ) {
            print_index_must_be_int_char(
                *index_type,
                statement->data.subscripted_assign.index.line_number,
                params
            );
        }

        if (
            assigned_type != NULL
            && !semantic_type_equiv(*assigned_type, variable_type)
        ) {
            print_semantic_type_mismatch(
                variable_type,
                *assigned_type,
                statement->data.subscripted_assign.assigned_value.line_number,
                params
            );
        }
    }
}

static void semantic_check_if(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    enum semantic_type const *condition_type;

    if (statement->data.if_.then != NULL) {
        semantic_check_statement(
            statement->data.if_.then,
            expected_return,
            params
       );
    }
    if (statement->data.if_.else_ !=  NULL) {
        semantic_check_statement(
            statement->data.if_.else_,
            expected_return,
            params
       );
    }

    condition_type = expression_semantic_type(
        &statement->data.if_.condition,
        params
    );

    if (
        condition_type != NULL
        && !semantic_type_equiv(*condition_type, SEMANTIC_BOOL)
    ) {
        print_semantic_type_mismatch(
            SEMANTIC_BOOL,
            *condition_type,
            statement->data.if_.condition.line_number,
            params
        );
    }
}

static void semantic_check_while(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    enum semantic_type const *condition_type;

    if (statement->data.while_.do_ !=  NULL) {
        semantic_check_statement(
            statement->data.while_.do_,
            expected_return,
            params
        );
    }

    condition_type = expression_semantic_type(
        &statement->data.while_.condition,
        params
    );

    if (
        condition_type != NULL
        && !semantic_type_equiv(*condition_type, SEMANTIC_BOOL)
    ) {
        print_semantic_type_mismatch(
            SEMANTIC_BOOL,
            *condition_type,
            statement->data.if_.condition.line_number,
            params
        );
    }
}

static void semantic_check_return(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    enum semantic_type const *actual_return = expression_semantic_type(
        &statement->data.return_.returned_value,
        params
    );


    if (actual_return != NULL && *actual_return != expected_return) {
        print_semantic_type_mismatch(
            expected_return,
            *actual_return,
            statement->data.return_.returned_value.line_number,
            params
        );
    }
}

static void semantic_check_write(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    size_t i;
    enum semantic_type const *argument_type;

    for (i = 0; i < statement->data.write.argument_list.length; i++) {
        switch (statement->data.write.argument_list.write_arguments[i].tag) {
            case AST_WRITE_EXPRESSION:
                argument_type = expression_semantic_type(
                    &statement
                        ->data
                        .write.argument_list.write_arguments[i].data.expression,
                    params
                );
                if (
                    argument_type != NULL
                    && semantic_type_equiv(*argument_type, SEMANTIC_BOOL)
                ) {
                    print_inv_semantic_type_mismatch(
                        *argument_type,
                        statement
                            ->data
                            .write
                            .argument_list
                            .write_arguments[i].data.expression.line_number,
                        params
                    );
                }
                break;
            case AST_WRITE_STRING_LIT:
                break;
        }
    }
}

static void semantic_check_body(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    size_t i;

    for (i = 0; i < statement->data.body.statement_list.length; i++) {
        semantic_check_statement(
            &statement->data.body.statement_list.statements[i],
            expected_return,
            params
        );
    }
}

static void semantic_check_expr_stmt(
    struct ast_statement *statement,
    enum semantic_type expected_return,
    struct semantic_error_params params
)
{
    expression_semantic_type(&statement->data.expression, params);
}

enum semantic_type const *expression_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
)
{
    switch (expression->inference_status) {
        case INFERENCE_ERROR:
            return NULL;
        case INFERENCE_UNKNOWN:
            infer_expression_semantic_type(expression, params);
            if (expression->inference_status == INFERENCE_ERROR) {
                return NULL;
            }
        case INFERENCE_OK:
            return &expression->semantic_type;
        default:
            panic(
                "unimplemented inference status %i",
                expression->inference_status
            );
    }
}

static void infer_expression_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
)
{
    switch (expression->tag) {
        case AST_INT_LITERAL:
            expression->inference_status = INFERENCE_OK;
            expression->semantic_type = SEMANTIC_INT;
            break;
        case AST_CHAR_LITERAL:
            expression->inference_status = INFERENCE_OK;
            expression->semantic_type = SEMANTIC_CHAR;
            break;
        case AST_FLOAT_LITERAL:
            expression->inference_status = INFERENCE_OK;
            expression->semantic_type = SEMANTIC_FLOAT;
            break;
        case AST_SUBSCRIPTION:
            infer_subscription_semantic_type(expression, params);
            break;
        case AST_VARIABLE:
            infer_variable_semantic_type(expression, params);
            break;
        case AST_BINARY_OPERATION:
            infer_binop_semantic_type(expression, params);
            break;
        case AST_UNARY_OPERATION:
            infer_unop_semantic_type(expression, params);
            break;
        case AST_FUNCTION_CALL:
            infer_func_call_semantic_type(expression, params);
            break;
        case AST_INPUT:
            expression->inference_status = INFERENCE_OK;
            expression->semantic_type = SEMANTIC_INT;
            break;
    }
}

static void infer_binop_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
)
{
    enum semantic_type const *left_operand;
    enum semantic_type const *right_operand;
    enum operation_type operation_type;

    operation_type = binary_operation_type(
        expression->data.binary_operation.operator
    );
    left_operand = expression_semantic_type(
        expression->data.binary_operation.left_operand,
        params
    );
    right_operand = expression_semantic_type(
        expression->data.binary_operation.right_operand,
        params
    );

    if (left_operand == NULL && right_operand == NULL) {
        expression->inference_status = INFERENCE_ERROR;
    }

    switch (operation_type) {
        case OPERATION_ARITHMETIC:
            if (
                left_operand != NULL
                && semantic_type_equiv(*left_operand, SEMANTIC_BOOL)
            ) {
                expression->inference_status = INFERENCE_ERROR;
                print_inv_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    expression->data.binary_operation.left_operand->line_number,
                    params
                );
            }

            if (
                right_operand != NULL
                && semantic_type_equiv(*right_operand, SEMANTIC_BOOL)
            ) {
                expression->inference_status = INFERENCE_ERROR;
                print_inv_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    expression
                        ->data.binary_operation.right_operand->line_number,
                    params
                );
            }

            if (expression->inference_status == INFERENCE_UNKNOWN) {
                if (left_operand == NULL) {
                    expression->semantic_type = *right_operand;
                    expression->inference_status = INFERENCE_OK;
                } else if (right_operand == NULL) {
                    expression->semantic_type = *left_operand;
                    expression->inference_status = INFERENCE_OK;
                } else if (
                    !semantic_type_equiv(*left_operand, *right_operand)
                ) {
                    expression->inference_status = INFERENCE_ERROR;
                    print_semantic_type_mismatch(
                        *left_operand,
                        *right_operand,
                        expression
                            ->data.binary_operation.right_operand->line_number,
                        params
                    );
                } else {
                    expression->semantic_type = *left_operand;
                    expression->inference_status = INFERENCE_OK;
                }
            }
            break;
        case OPERATION_LOGICAL:
            if (
                left_operand != NULL
                && !semantic_type_equiv(*left_operand, SEMANTIC_BOOL)
            ) {
                print_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    *left_operand,
                    expression
                        ->data.binary_operation.left_operand->line_number,
                    params
                );
            }
            if (
                right_operand != NULL
                && !semantic_type_equiv(*right_operand, SEMANTIC_BOOL)
            ) {
                print_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    *right_operand,
                    expression
                        ->data.binary_operation.right_operand->line_number,
                    params
                );
            }

            expression->semantic_type = SEMANTIC_BOOL;
            expression->inference_status = INFERENCE_OK;
            break;

        case OPERATION_COMPARISON:
            if (
                left_operand != NULL
                && semantic_type_equiv(*left_operand, SEMANTIC_BOOL)
             ) {
                expression->inference_status = INFERENCE_ERROR;
                print_inv_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    expression->data.binary_operation.left_operand->line_number,
                    params
                );
            }

            if (
                right_operand != NULL
                && semantic_type_equiv(*right_operand, SEMANTIC_BOOL)
            ) {
                expression->inference_status = INFERENCE_ERROR;
                print_inv_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    expression
                        ->data.binary_operation.right_operand->line_number,
                    params
                );
            }

            if (
                expression->inference_status == INFERENCE_UNKNOWN
                && left_operand != NULL
                && right_operand != NULL
                && !semantic_type_equiv(*left_operand, *right_operand)
            ) {
                print_semantic_type_mismatch(
                    *left_operand,
                    *right_operand,
                    expression
                        ->data.binary_operation.right_operand->line_number,
                    params
                );
            }

            expression->semantic_type = SEMANTIC_BOOL;
            expression->inference_status = INFERENCE_OK;
            break;
    }
}

static void infer_unop_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
)
{

    enum semantic_type const *operand;
    enum operation_type operation_type;

    operation_type = unary_operation_type(
        expression->data.unary_operation.operator
    );
    operand = expression_semantic_type(
        expression->data.binary_operation.left_operand,
        params
    );

    switch (operation_type) {
        case OPERATION_ARITHMETIC:
            if (operand == NULL) {
                expression->inference_status = INFERENCE_ERROR;
            } else if (semantic_type_equiv(*operand, SEMANTIC_BOOL)) {
                print_inv_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    expression->data.unary_operation.operand->line_number,
                    params
                );
                expression->inference_status = INFERENCE_ERROR;
            } else {
                expression->semantic_type = *operand;
                expression->inference_status = INFERENCE_OK;
            }
            break;
        case OPERATION_LOGICAL:
            expression->semantic_type = SEMANTIC_BOOL;
            expression->inference_status = INFERENCE_OK;
            if (
                operand != NULL
                && !semantic_type_equiv(*operand, SEMANTIC_BOOL)
            ) {
                print_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    *operand,
                    expression->data.unary_operation.operand->line_number,
                    params
                );
            }
            break;
        case OPERATION_COMPARISON:
            expression->semantic_type = SEMANTIC_BOOL;
            expression->inference_status = INFERENCE_OK;
            if (
                operand != NULL
                && semantic_type_equiv(*operand, SEMANTIC_BOOL)
            ) {
                print_inv_semantic_type_mismatch(
                    SEMANTIC_BOOL,
                    expression->data.unary_operation.operand->line_number,
                    params
                );
            }
            break;
    }
}

static void infer_variable_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
)
{
    if (expression->data.variable.name->type != SYM_SCALAR_VAR) {
        expression->inference_status = INFERENCE_ERROR;
        print_symbol_type_mismatch(
            SYM_SCALAR_VAR,
            *expression->data.subscription.variable,
            expression->line_number,
            params
        );
    }
    if (!expression->data.variable.name->data.variable.in_scope) {
        expression->inference_status = INFERENCE_ERROR;
        print_symbol_not_in_scope(
            *expression->data.subscription.variable,
            expression->line_number,
            params
        );
    }
    if (expression->inference_status == INFERENCE_UNKNOWN) {
        expression->inference_status = INFERENCE_OK;
        expression->semantic_type = datatype_to_semantic_type(
            expression->data.subscription.variable->data.variable.type
        );
    }
}

static void infer_subscription_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
)
{
    enum semantic_type const *index_type;

    if (expression->data.subscription.variable->type != SYM_VECTOR_VAR) {
        expression->inference_status = INFERENCE_ERROR;
        print_symbol_type_mismatch(
            SYM_VECTOR_VAR,
            *expression->data.subscription.variable,
            expression->line_number,
            params
        );
    }
    if (!expression->data.subscription.variable->data.variable.in_scope) {
        expression->inference_status = INFERENCE_ERROR;
        print_symbol_not_in_scope(
            *expression->data.subscription.variable,
            expression->line_number,
            params
        );
    }
    index_type = expression_semantic_type(
        expression->data.subscription.index,
        params
    );
    if (index_type == NULL) {
        expression->inference_status = INFERENCE_ERROR;
    } else if (!semantic_type_equiv(*index_type, SEMANTIC_INT)) {
        print_index_must_be_int_char(
            *index_type,
            expression->data.subscription.index->line_number,
            params
        );
    }
    if (expression->inference_status == INFERENCE_UNKNOWN) {
        expression->inference_status = INFERENCE_OK;
        expression->semantic_type = datatype_to_semantic_type(
            expression->data.subscription.variable->data.variable.type
        );
    }
}

static void infer_func_call_semantic_type(
    struct ast_expression *expression,
    struct semantic_error_params params
){
    size_t i, min_arg_param_count;
    enum semantic_type parameter_type;
    enum semantic_type const *argument_type;
    struct function_datatype function_datatype;

    if (expression->data.function_call.function->type != SYM_FUNCTION) {
        expression->inference_status = INFERENCE_ERROR;
        print_symbol_type_mismatch(
            SYM_FUNCTION,
            *expression->data.function_call.function,
            expression->line_number,
            params
        );
    } else {
        function_datatype =
            expression->data.function_call.function->data.function;
        
        if (
            function_datatype.parameter_types.length
            != expression->data.function_call.argument_list.length
        ) {
            print_argument_number_mismatch(
                function_datatype.parameter_types.length,
                expression->data.function_call.argument_list.length,
                expression->line_number,
                params
            );
        }

        min_arg_param_count = function_datatype.parameter_types.length;
        if (
            min_arg_param_count
            > expression->data.function_call.argument_list.length
        ) {
            min_arg_param_count =
                expression->data.function_call.argument_list.length;
        }

        for (i = 0; i < min_arg_param_count; i++) {
            parameter_type = datatype_to_semantic_type(
                function_datatype.parameter_types.types[i]
            );
            argument_type = expression_semantic_type(
                &expression->data.function_call.argument_list.expressions[i],
                params
            );
            if (
                argument_type != NULL
                && !semantic_type_equiv(*argument_type, parameter_type)
            ) {
                print_semantic_type_mismatch(
                    parameter_type,
                    *argument_type,
                    expression
                        ->data
                        .function_call.argument_list.expressions[i].line_number,
                    params
                );
            }
        }

        expression->semantic_type = datatype_to_semantic_type(
            function_datatype.return_type
        );
        expression->inference_status = INFERENCE_OK;
    }
}

static void semantic_check_scalar_var_decl(
    struct ast_declaration *declaration,
    struct semantic_error_params params
)
{
    enum semantic_type decl_type = datatype_to_semantic_type(
        declaration->data.scalar_var.datatype
    );
    enum semantic_type const *init_type = expression_semantic_type(
        &declaration->data.scalar_var.init,
        params
    );

    if (init_type != NULL && !semantic_type_equiv(*init_type, decl_type)) {
        print_semantic_type_mismatch(
            decl_type,
            *init_type,
            declaration->data.scalar_var.init.line_number,
            params
        );
    }
}

static void semantic_check_vector_var_decl(
    struct ast_declaration *declaration,
    struct semantic_error_params params
)
{
    size_t i, min_arg_param_count;

    long length_value;

    enum semantic_type decl_type = datatype_to_semantic_type(
        declaration->data.vector_var.datatype
    );
    
    enum semantic_type const *element_type;

    enum semantic_type const *length_type = expression_semantic_type(
        &declaration->data.vector_var.length,
        params
    );

    if (length_type != NULL) {
        if (!semantic_type_equiv(*length_type, SEMANTIC_INT)) {
            print_semantic_type_mismatch(
                SEMANTIC_INT,
                *length_type,
                declaration->data.vector_var.length.line_number,
                params
            );
        } else if (
            !const_eval_int_expression(
                declaration->data.vector_var.length,
                &length_value
            )
        ) {
            print_index_must_be_constant(
                declaration->data.vector_var.length.line_number,
                params
            );
        } else {
            if (length_value < declaration->data.vector_var.init.length) {
                print_vector_element_number_mismatch(
                    length_value,
                    declaration->data.vector_var.init.length,
                    declaration
                        ->data.vector_var.init.expressions[0].line_number,
                    params
                );
            }

            min_arg_param_count = declaration->data.vector_var.init.length;
            if (min_arg_param_count > length_value) {
                min_arg_param_count = length_value;
            }

            for (i = 0; i < min_arg_param_count; i++) {
                element_type = expression_semantic_type(
                    &declaration->data.vector_var.init.expressions[i],
                    params
                );
                if (
                    element_type != NULL
                    && !semantic_type_equiv(*element_type, decl_type)
                ) {
                    print_semantic_type_mismatch(
                        decl_type,
                        *element_type,
                        declaration
                            ->data.vector_var.init.expressions[i].line_number,
                        params
                    );
                }
            }
        }
    }
}

void print_semantic_type_mismatch(
    enum semantic_type expected_type,
    enum semantic_type found_type,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "expected type %s but found type %s at line %i\n",
        semantic_type_to_str(expected_type),
        semantic_type_to_str(found_type),
        line_number
    );
}

void print_inv_semantic_type_mismatch(
    enum semantic_type unexpected_type,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "unexpected type %s at line %i\n",
        semantic_type_to_str(unexpected_type),
        line_number
    );
}

void print_symbol_type_mismatch(
    enum symbol_type expected_type,
    struct symbol found_symbol,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "expected %s but found %s (`%s`) at line %i\n",
        symbol_type_to_str(expected_type),
        symbol_type_to_str(found_symbol.type),
        found_symbol.content,
        line_number
    );
}

void print_symbol_not_in_scope(
    struct symbol symbol,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "symbol `%s` is not in scope at line %i\n",
        symbol.content,
        line_number
    );
}

void print_index_must_be_int_char(
    enum semantic_type found,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "index must be an %s or %s, found %s at line %i\n",
        semantic_type_to_str(SEMANTIC_INT),
        semantic_type_to_str(SEMANTIC_CHAR),
        semantic_type_to_str(found),
        line_number
    );
}

void print_index_must_be_constant(
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "found vector length that is not a constant integer expression at line %i\n",
        line_number
    );
}

void print_argument_number_mismatch(
    size_t expected,
    size_t given,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "function call expects %zu parameters, given %zu at line %i\n",
        expected,
        given,
        line_number
    );
}

void print_vector_element_number_mismatch(
    size_t expected,
    size_t given,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "vector initialization expects at most %zu elements, given %zu at line %i\n",
        expected,
        given,
        line_number
    );
}

void print_redeclared_symbol(
    struct symbol symbol,
    int line_number,
    struct semantic_error_params params
)
{
    *params.error_count += 1;
    fprintf(
        params.output,
        "symbol `%s` (originally declared at line %i) redeclared at line %i\n",
        symbol.content,
        symbol.line_number,
        line_number
    );
}

enum operation_type binary_operation_type( enum ast_binary_operator operator)
{
    switch (operator) {
        case AST_ADD:
        case AST_SUB:
        case AST_MUL:
        case AST_DIV:
            return OPERATION_ARITHMETIC;
        case AST_LESS_THAN:
        case AST_GREATER_THAN:
        case AST_LESS_OR_EQUALS:
        case AST_GREATER_OR_EQUALS:
        case AST_EQUALS:
        case AST_NOT_EQUAL:
            return OPERATION_COMPARISON;
        case AST_AND:
        case AST_OR:
            return OPERATION_LOGICAL;
    }
    panic("unimplemented binary operation %i's semantic check", operator);
}


enum operation_type unary_operation_type( enum ast_unary_operator operator)
{
    switch (operator) {
        case AST_NOT:
            return OPERATION_LOGICAL;
    }
    panic("unimplemented unary operation %i's semantic check", operator);
}

