%{
#include "ast.h"
#include "types.h"
#include "symboltable.h"
#include "alloc.h"
#include "lexer.h"
#include "yacc_api.h"
#include "vector.h"
#include <stdio.h>
#include <stdarg.h>

#ifdef DEBUG
#   define TRACE printf("yacc hit line %i\n", __LINE__)
#else
#   define TRACE
#endif

static int g_parse_succeeded;

int parse(void)
{
    g_parse_succeeded = 1;
    return yyparse() == 0 && g_parse_succeeded;
}

void yyerror(char const *message)
{
    g_parse_succeeded = 0;
    fprintf(stderr, "%s at line %i\n", message, getLineNumber());
}

static void err_recovery(char const *message)
{
    g_parse_succeeded = 0;
    fprintf(stderr, "    %s\n", message);
}

%}

%union{
    struct symbol *symbol;

    struct ast ast;

    struct ast_declaration_list declaration_list;
    struct ast_declaration declaration;
    struct ast_vector_var_decl vector_var_decl;
    struct ast_scalar_var_decl scalar_var_decl;
    struct ast_function_decl function_decl;
    struct ast_parameter_list parameter_list;
    struct ast_parameter parameter;

    struct ast_statement_list statement_list;
    struct ast_statement statement;
    struct ast_statement *optional_statement;
    struct ast_body body;
    struct ast_scalar_var_assign scalar_var_assign;
    struct ast_subscripted_assign subscripted_assign;
    struct ast_if if_;
    struct ast_while while_;

    struct ast_write_argument write_argument;
    struct ast_write_argument_list write_argument_list;

    struct ast_expression_list expression_list;
    struct ast_expression expression;

    enum datatype datatype;
}


%token KW_CARA
%token KW_INTE
%token KW_REAL

%token KW_SE
%token KW_ENTAUM
%token KW_SENAUM
%token KW_ENQUANTO
%token KW_ENTRADA
%token KW_ESCREVA
%token KW_RETORNE

%token OPERATOR_LE
%token OPERATOR_GE
%token OPERATOR_EQ
%token OPERATOR_DIF

%token<symbol> TK_IDENTIFIER

%token<symbol> LIT_INTEIRO
%token<symbol> LIT_FLOAT
%token<symbol> LIT_CHAR
%token<symbol> LIT_STRING

%token TOKEN_ERROR

%left '{' '}'
%left ';'
%right '='
%left '<' '>' OPERATOR_LE OPERATOR_GE OPERATOR_EQ OPERATOR_DIF
%left '|'
%left '&'
%left '~' '+' '-'
%left '*' '/'
%left '[' ']'
%left '(' ')'

%type<ast> program

%type<datatype> datatype

%type<declaration_list> toplevel_declaration_list
%type<declaration> toplevel_declaration
%type<vector_var_decl> vector_var_declaration
%type<scalar_var_decl> scalar_var_declaration
%type<function_decl> function_declaration
%type<parameter_list> parameter_list
%type<parameter> parameter

%type<statement_list> statement_list
%type<statement> statement
%type<optional_statement> optional_statement
%type<body> body
%type<scalar_var_assign> scalar_var_assign
%type<subscripted_assign> subscripted_assign
%type<if_> if_
%type<while_> while_

%type<write_argument> write_argument
%type<write_argument_list> write_argument_list

%type<expression_list> expression_list
%type<expression> expression

%%

program: toplevel_declaration_list
            {
                $$.declaration_list = $1;
                $$.is_valid = 1;
                g_ast = $$;
                TRACE;
            }
       ;

toplevel_declaration_list: toplevel_declaration_list toplevel_declaration
                            {
                                $1.declarations = vector_push(
                                    $1.declarations,
                                    sizeof($2),
                                    &$1.length,
                                    &$2
                                );
                                $$ = $1;
                                TRACE;
                            }
                        |
                            {
                                $$.declarations = vector_empty(&$$.length);
                                TRACE;
                            }
                        ;

toplevel_declaration: scalar_var_declaration ';'
                        {
                            $$.tag = AST_SCALAR_VAR_DECL;
                            $$.data.scalar_var = $1;
                            TRACE;
                        }
                      | vector_var_declaration ';'
                        {
                            $$.tag = AST_VECTOR_VAR_DECL;
                            $$.data.vector_var = $1;
                            TRACE;
                        }
                      | function_declaration
                        {
                            $$.tag = AST_FUNCTION_DECL;
                            $$.data.function = $1;
                            TRACE;
                        }
                      | scalar_var_declaration error 
                        {
                            $$.tag = AST_SCALAR_VAR_DECL;
                            $$.data.scalar_var = $1;
                            err_recovery("missing semicolon after scalar variable declaration");
                            TRACE;
                        }
                      | vector_var_declaration error 
                        {
                            $$.tag = AST_VECTOR_VAR_DECL;
                            $$.data.vector_var = $1;
                            err_recovery("missing semicolon after vector variable declaration");
                            TRACE;
                        }
                      ;

datatype: KW_INTE
            {
                $$ = DATATYPE_INTE;
                TRACE;
            }
        | KW_CARA
            {
                $$ = DATATYPE_CARA;
                TRACE;
            }
        | KW_REAL
            {
                $$ = DATATYPE_REAL;
                TRACE;
            }
        ;

scalar_var_declaration: datatype TK_IDENTIFIER '=' expression
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.init = $4;
                                TRACE;
                            }
                      | datatype TK_IDENTIFIER error expression
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.init = $4;
                                err_recovery("missing assignment operator in scalar variable declaration");
                                TRACE;
                            }
                      | datatype TK_IDENTIFIER '=' error
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.init.tag = AST_INPUT;
                                err_recovery("missing initialization expression in scalar variable declaration");
                                TRACE;
                            }
                      | datatype TK_IDENTIFIER error
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.init.tag = AST_INPUT;
                                err_recovery("missing initialization in scalar variable declaration");
                                TRACE;
                            }
                      | datatype error '=' expression
                            {
                                $$.datatype = $1;
                                $$.name = symbol_table_insert("@null");
                                $$.init.tag = AST_INPUT;
                                err_recovery("missing variable name in scalar variable declaration");
                                TRACE;
                            }
                      | datatype error '=' error
                            {
                                $$.datatype = $1;
                                $$.name = symbol_table_insert("@null");
                                $$.init.tag = AST_INPUT;
                                err_recovery("missing variable name and initialization in scalar variable declaration");
                                TRACE;
                            }
                      ;

vector_var_declaration: datatype TK_IDENTIFIER '[' expression ']' expression_list
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.length = $4;
                                $$.init = $6;
                                TRACE;
                            }
                      | datatype TK_IDENTIFIER '[' expression error expression_list
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.length = $4;
                                $$.init = $6;
                                err_recovery("missing closing square bracket in vector declaration");
                                TRACE;
                            }
                      | datatype TK_IDENTIFIER error expression ']' expression_list
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.length = $4;
                                $$.init = $6;
                                err_recovery("missing opening square bracket in vector declaration");
                                TRACE;
                            }
                      | datatype TK_IDENTIFIER error expression error expression_list
                            {
                                $$.datatype = $1;
                                $$.name = $2;
                                $$.length = $4;
                                $$.init = $6;
                                err_recovery("missing square brackets in vector declaration");
                                TRACE;
                            }
                      ;

expression_list: expression_list expression 
                    {
                        $1.expressions = vector_push(
                            $1.expressions,
                            sizeof($2),
                            &$1.length,
                            &$2
                        );
                        $$ = $1;
                        TRACE;
                    }
               |
                    {
                        $$.expressions = vector_empty(&$$.length);
                        TRACE;
                    }
               ;

function_declaration: datatype TK_IDENTIFIER '(' parameter_list ')' body
                        {
                            $$.return_datatype = $1;
                            $$.name = $2;
                            $$.parameter_list = $4;
                            $$.body = $6;
                            TRACE;
                        }
                    | datatype TK_IDENTIFIER error parameter_list ')' body
                        {
                            $$.return_datatype = $1;
                            $$.name = $2;
                            $$.parameter_list = $4;
                            $$.body = $6;
                            err_recovery("missing opening parenthesis in parameter list");
                            TRACE;
                        }
                    | datatype TK_IDENTIFIER '(' parameter_list error body
                        {
                            $$.return_datatype = $1;
                            $$.name = $2;
                            $$.parameter_list = $4;
                            $$.body = $6;
                            err_recovery("missing closing parenthesis in parameter list");
                            TRACE;
                        }
                    | datatype TK_IDENTIFIER error parameter_list error body
                        {
                            $$.return_datatype = $1;
                            $$.name = $2;
                            $$.parameter_list = $4;
                            $$.body = $6;
                            err_recovery("missing parentheses in parameter list");
                            TRACE;
                        }
                    | datatype TK_IDENTIFIER error body
                        {
                            $$.return_datatype = $1;
                            $$.name = $2;
                            $$.parameter_list.length = 0;
                            $$.parameter_list.parameters = NULL;
                            $$.body = $4;
                            err_recovery("missing parameter list");
                            TRACE;
                        }
                    ;

parameter_list: parameter_list parameter
                    {
                        $1.parameters = vector_push(
                            $1.parameters,
                            sizeof($2),
                            &$1.length,
                            &$2
                        );
                        $$ = $1;
                        TRACE;
                    }
              |
                    {
                        $$.parameters = vector_empty(&$$.length);
                        TRACE;
                    }
              ;

parameter: datatype TK_IDENTIFIER
            {
                $$.datatype = $1;
                $$.line_number = getLineNumber();
                $$.name = $2;
                TRACE;
            }
         ;

body: '{' statement_list '}'
        {
            $$.statement_list = $2;
            TRACE;
        }
    | '{' statement_list error
        {
            $$.statement_list = $2;
            err_recovery("missing closing curly brace in statement body");
            TRACE;
        }
    | error statement_list '}'
        {
            $$.statement_list = $2;
            err_recovery("missing opening curly brace in statement body");
            TRACE;
        }
    | error statement_list error
        {
            $$.statement_list = $2;
            err_recovery("missing curly braces in statement body");
            TRACE;
        }
    ;

statement_list: statement_list ';' statement
                {
                    $1.statements = vector_push(
                        $1.statements,
                        sizeof($3),
                        &$1.length,
                        &$3
                    );
                    $$ = $1;
                    TRACE;
                }
              | statement_list ';'
                {
                    $$ = $1;
                    TRACE;
                }
              | statement
                {
                    $$.statements = vector_singleton(
                        sizeof($1),
                        &$$.length,
                        &$1
                    );
                    TRACE;
                }
              |
                {
                    $$.statements = vector_empty(&$$.length);
                    TRACE;
                }
             | statement_list error statement
                {
                    $1.statements = vector_push(
                        $1.statements,
                        sizeof($3),
                        &$1.length,
                        &$3
                    );
                    $$ = $1;
                    err_recovery("missing semicolon separating statements");
                    TRACE;
                }
              ;

optional_statement: statement
                    {
                        $$ = aborting_malloc(sizeof($1));
                        *$$ = $1;
                        TRACE;
                    }
                  |
                    {
                        $$ = NULL;
                        TRACE;
                    }
                  ;

statement: scalar_var_assign
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_SCALAR_VAR_ASSIGN;
                    $$.data.scalar_var_assign = $1;
                    TRACE;
                }
          | subscripted_assign
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_SUBSCRIPTED_ASSIGN;
                    $$.data.subscripted_assign = $1;
                    TRACE;
                }
         | if_
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_IF;
                    $$.data.if_ = $1;
                    TRACE;
                }
         | while_
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_WHILE;
                    $$.data.while_ = $1;
                    TRACE;
                }
         | KW_ESCREVA write_argument_list
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_WRITE;
                    $$.data.write.argument_list = $2;
                    TRACE;
                }
         | KW_RETORNE expression
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_RETURN;
                    $$.data.return_.returned_value = $2;
                    TRACE;
                }
         | body
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_BODY;
                    $$.data.body = $1;
                    TRACE;
                }
         | expression
                {
                    $$ = ast_statement_base_init();
                    $$.tag = AST_EXPRESSION_STATEMENT;
                    $$.data.expression = $1;
                    TRACE;
                }
         ;

scalar_var_assign: TK_IDENTIFIER '=' expression
                    {
                        $$.variable = $1;
                        $$.assigned_value = $3;
                        TRACE;
                    }
                 | error '=' expression
                    {
                        $$.variable = symbol_table_insert("@null");
                        $$.assigned_value = $3;
                        err_recovery("missing assignment target");
                        TRACE;
                    }
                 | TK_IDENTIFIER '=' error
                    {
                        $$.variable = $1;
                        $$.assigned_value.tag = AST_INPUT;
                        err_recovery("missing assigned value to scalar variable");
                        TRACE;
                    }
                 ;

subscripted_assign: TK_IDENTIFIER '[' expression ']' '=' expression
                    {
                        $$.variable = $1;
                        $$.index = $3;
                        $$.assigned_value = $6;
                        TRACE;
                    }
                 | TK_IDENTIFIER '[' expression error '=' expression
                    {
                        $$.variable = $1;
                        $$.index = $3;
                        $$.assigned_value = $6;
                        err_recovery("missing closing square bracket in vector write access");
                        TRACE;
                    }
                 | TK_IDENTIFIER error expression ']' '=' expression
                    {
                        $$.variable = $1;
                        $$.index = $3;
                        $$.assigned_value = $6;
                        err_recovery("missing opening square bracket in vector write access");
                        TRACE;
                    }
                 | TK_IDENTIFIER error expression error '=' expression
                    {
                        $$.variable = $1;
                        $$.index = $3;
                        $$.assigned_value = $6;
                        err_recovery("missing square brackets in vector write access");
                        TRACE;
                    }
                 | error '[' expression ']' '=' expression
                    {
                        $$.variable = symbol_table_insert("@null");
                        $$.index = $3;
                        $$.assigned_value.tag = AST_INPUT;
                        err_recovery("missing assigned vector variable");
                        TRACE;
                    }
                 | TK_IDENTIFIER error expression error '=' error
                    {
                        $$.variable = $1;
                        $$.index = $3;
                        $$.assigned_value.tag = AST_INPUT;
                        err_recovery("missing assigned value to vector variable");
                        TRACE;
                    }
                  ;

if_: KW_ENTAUM optional_statement KW_SE '(' expression ')'
        {
            $$.then = $2;
            $$.else_ = NULL;
            $$.condition = $5;
            TRACE;
        }
   | KW_ENTAUM optional_statement
     KW_SENAUM optional_statement KW_SE '(' expression ')'
        {
            $$.then = $2;
            $$.else_ = $4;
            $$.condition = $7;
            TRACE;
        }
   | KW_ENTAUM optional_statement KW_SE '(' expression error
        {
            $$.then = $2;
            $$.else_ = NULL;
            $$.condition = $5;
            err_recovery("missing closing parenthesis in `se` condition");
            TRACE;
        }
   | KW_ENTAUM optional_statement KW_SE error expression ')'
        {
            $$.then = $2;
            $$.else_ = NULL;
            $$.condition = $5;
            err_recovery("missing opening parenthesis in `se` condition");
            TRACE;
        }
   | KW_ENTAUM optional_statement KW_SE error expression error
        {
            $$.then = $2;
            $$.else_ = NULL;
            $$.condition = $5;
            err_recovery("missing parentheses in `se` condition");
            TRACE;
        }
   | KW_ENTAUM optional_statement
     KW_SENAUM optional_statement KW_SE '(' expression error
        {
            $$.then = $2;
            $$.else_ = $4;
            $$.condition = $7;
            err_recovery("missing closing parenthesis in `se` condition");
            TRACE;
        }
   | KW_ENTAUM optional_statement
     KW_SENAUM optional_statement KW_SE error expression ')'
        {
            $$.then = $2;
            $$.else_ = $4;
            $$.condition = $7;
            err_recovery("missing opening parenthesis in `se` condition");
            TRACE;
        }
   | KW_ENTAUM optional_statement
     KW_SENAUM optional_statement KW_SE error expression error
        {
            $$.then = $2;
            $$.else_ = $4;
            $$.condition = $7;
            err_recovery("missing parentheses in `se` condition");
            TRACE;
        }
   | KW_SENAUM optional_statement KW_SE error
        {
            $$.then = $2;
            $$.else_ = NULL;
            $$.condition.tag = AST_INPUT;
            err_recovery("missing conditon in `se`");
            TRACE;
        }
   | KW_ENTAUM optional_statement
     KW_SENAUM optional_statement KW_SE error
        {
            $$.then = $2;
            $$.else_ = $4;
            $$.condition.tag = AST_INPUT;
            err_recovery("missing conditon in `se`");
            TRACE;
        }
   ;

while_: optional_statement KW_ENQUANTO '(' expression ')'
            {
                $$.do_ = $1;
                $$.condition = $4;
                TRACE;
            }
      | optional_statement KW_ENQUANTO '(' expression error
            {
                $$.do_ = $1;
                $$.condition = $4;
                err_recovery("missing closing parenthesis in `enquanto` condition");
                TRACE;
            }
      | optional_statement KW_ENQUANTO error expression ')'
            {
                $$.do_ = $1;
                $$.condition = $4;
                err_recovery("missing opening parenthesis in `enquanto` condition");
                TRACE;
            }
      | optional_statement KW_ENQUANTO error expression error
            {
                $$.do_ = $1;
                $$.condition = $4;
                err_recovery("missing parentheses in `enquanto` condition");
                TRACE;
            }
      | optional_statement KW_ENQUANTO error
            {
                $$.do_ = $1;
                $$.condition.tag = AST_INPUT;
                err_recovery("missing condition in `enquanto`");
                TRACE;
            }
      ;

write_argument: expression
            {
                $$.tag = AST_WRITE_EXPRESSION;
                $$.data.expression = $1;
                TRACE;
            }
         | LIT_STRING
            {
                $$.tag = AST_WRITE_STRING_LIT;
                $$.data.string_literal = $1;
                TRACE;
            }
         ;

write_argument_list: write_argument_list write_argument
                    {
                        $1.write_arguments = vector_push(
                            $1.write_arguments,
                            sizeof($2),
                            &$1.length,
                            &$2
                        );
                        $$ = $1;
                        TRACE;
                    }
               |
                    {
                        $$.write_arguments = vector_empty(&$$.length);
                        TRACE;
                    }
               ;

expression: LIT_INTEIRO
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_INT_LITERAL;
                    $$.data.literal = $1;
                    TRACE;
                }
          | LIT_CHAR
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_CHAR_LITERAL;
                    $$.data.literal = $1;
                    TRACE;
                }
          | LIT_FLOAT
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_FLOAT_LITERAL;
                    $$.data.literal = $1;
                    TRACE;
                }
          | TK_IDENTIFIER '[' expression ']'
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_SUBSCRIPTION;
                    $$.data.subscription.variable = $1;
                    $$.data.subscription.index = aborting_malloc(sizeof($3));
                    *$$.data.subscription.index = $3;
                    TRACE;
                }
          | TK_IDENTIFIER
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_VARIABLE;
                    $$.data.variable.name = $1;
                    TRACE;
                }
          | expression '+' expression
                {
                    $$ = ast_create_binary_operation($1, AST_ADD, $3);
                    TRACE;
                }
          | expression '-' expression
                {
                    $$ = ast_create_binary_operation($1, AST_SUB, $3);
                    TRACE;
                }
          | expression '*' expression
                {
                    $$ = ast_create_binary_operation($1, AST_MUL, $3);
                    TRACE;
                }
          | expression '/' expression
                {
                    $$ = ast_create_binary_operation($1, AST_DIV, $3);
                    TRACE;
                }
          | expression '<' expression
                {
                    $$ = ast_create_binary_operation($1, AST_LESS_THAN, $3);
                    TRACE;
                }
          | expression '>' expression
                {
                    $$ = ast_create_binary_operation($1, AST_GREATER_THAN, $3);
                    TRACE;
                }
          | expression OPERATOR_LE expression
                {
                    $$ = ast_create_binary_operation(
                        $1,
                        AST_LESS_OR_EQUALS,
                        $3
                    );
                    TRACE;
                }
          | expression OPERATOR_GE expression
                {
                    $$ = ast_create_binary_operation(
                        $1,
                        AST_GREATER_OR_EQUALS,
                        $3
                    );
                    TRACE;
                }
          | expression OPERATOR_EQ expression
                {
                    $$ = ast_create_binary_operation($1, AST_EQUALS, $3);
                    TRACE;
                }
          | expression OPERATOR_DIF expression
                {
                    $$ = ast_create_binary_operation($1, AST_NOT_EQUAL, $3);
                    TRACE;
                }
          | expression '&' expression
                {
                    $$ = ast_create_binary_operation($1, AST_AND, $3);
                    TRACE;
                }
          | expression '|' expression
                {
                    $$ = ast_create_binary_operation($1, AST_OR, $3);
                    TRACE;
                }
          | '~' expression
                {
                    $$ = ast_create_unary_operation(AST_NOT, $2);
                    TRACE;
                }
          | '(' expression ')'
                {
                    $$ = $2;
                    TRACE;
                }
          | TK_IDENTIFIER '(' expression_list ')'
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_FUNCTION_CALL;
                    $$.data.function_call.function = $1;
                    $$.data.function_call.argument_list = $3;
                    TRACE;
                }
          | KW_ENTRADA
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_INPUT;
                    TRACE;
                }
          | TK_IDENTIFIER '[' expression error
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_SUBSCRIPTION;
                    $$.data.subscription.variable = $1;
                    $$.data.subscription.index = aborting_malloc(sizeof($3));
                    *$$.data.subscription.index = $3;
                    err_recovery("missing closing square bracket in vector read access");
                    TRACE;
                }
          | TK_IDENTIFIER error expression ']'
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_SUBSCRIPTION;
                    $$.data.subscription.variable = $1;
                    $$.data.subscription.index = aborting_malloc(sizeof($3));
                    *$$.data.subscription.index = $3;
                    err_recovery("missing closing square bracket in vector read access");
                    TRACE;
                }
          | '(' expression error
                {
                    $$ = $2;
                    err_recovery("missing closing parenthesis in expression nesting");
                    TRACE;
                }
          | error expression ')'
                {
                    $$ = $2;
                    err_recovery("missing opening parenthesis in expression nesting");
                    TRACE;
                }
          | TK_IDENTIFIER '(' expression_list error
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_FUNCTION_CALL;
                    $$.data.function_call.function = $1;
                    $$.data.function_call.argument_list = $3;
                    err_recovery("missing closing parenthesis in argument list");
                    TRACE;
                }
          | TK_IDENTIFIER error expression_list ')'
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_FUNCTION_CALL;
                    $$.data.function_call.function = $1;
                    $$.data.function_call.argument_list = $3;
                    err_recovery("missing opening parenthesis in argument list");
                    TRACE;
                }
          | TK_IDENTIFIER error expression_list error
                {
                    $$ = ast_expression_base_init();
                    $$.tag = AST_FUNCTION_CALL;
                    $$.data.function_call.function = $1;
                    $$.data.function_call.argument_list = $3;
                    err_recovery("missing parentheses in argument list");
                    TRACE;
                }
          ;
