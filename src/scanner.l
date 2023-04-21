%{
#include <limits.h>
#include "parser.h"

static int lineNumber = 0;
static int running = 1;
%}

%x MULTILINE_COMMENT

%%

"se"                        { return KW_SE; }
"entaum"                    { return KW_ENTAUM; }
"senaum"                    { return KW_SENAUM; }
"enquanto"                  { return KW_ENQUANTO; }
"escreva"                   { return KW_ESCREVA; }
"entrada"                   { return KW_ENTRADA; }
"retorne"                   { return KW_RETORNE; }
"inte"                      { return KW_INTE; }
"cara"                      { return KW_CARA; }
"real"                      { return KW_REAL; }
"<="                        { return OPERATOR_LE; }
">="                        { return OPERATOR_GE; }
"=="                        { return OPERATOR_EQ; }
"!="                        { return OPERATOR_DIF; }
[._a-z][._a-z0-9]*          {
                                yylval.symbol = symbol_table_insert(
                                    yytext
                                );
                                yylval.symbol->type = SYM_UNKNOWN_IDENT;
                                yylval.symbol->data.variable.in_scope = 0;
                                yylval.symbol->data.variable.stack_frame_index =
                                    SIZE_MAX;
                                return TK_IDENTIFIER;
                            }
[0-9]+\.[0-9]+              {
                                yylval.symbol = symbol_table_insert(yytext);
                                yylval.symbol->type = SYM_LIT_FLOAT;
                                yylval.symbol->data.float_.parsed =
                                    atof(yylval.symbol->content);
                                yylval.symbol->data.float_.identifier = NULL;
                                return LIT_FLOAT;
                            }
[0-9]+                      {
                                yylval.symbol = symbol_table_insert(yytext);
                                yylval.symbol->type = SYM_LIT_INT;
                                yylval.symbol->data.parsed_int =
                                    atol(yylval.symbol->content);
                                return LIT_INTEIRO;
                            }
\'([^\'\\]|\\.)\'           {
                                yylval.symbol = symbol_table_insert(yytext);
                                yylval.symbol->type = SYM_LIT_CHAR;
                                yylval.symbol->data.parsed_char =
                                    char_literal_parse(yylval.symbol->content);
                                return LIT_CHAR;
                            }
\"([^\"\\]|\\.)*\"          {
                                yylval.symbol = symbol_table_insert(yytext);
                                yylval.symbol->type = SYM_LIT_STR;
                                yylval.symbol->data.string.literal =
                                    string_literal_parse(
                                        yylval.symbol->content
                                    );
                                yylval.symbol->data.string.identifier = NULL;
                                return LIT_STRING;
                            }
[;\(\)\[\]{}=+\-*\/><>&|~]  { return yytext[0]; }
\n                          { lineNumber++; }
[ ,\t\r]
"\/\/\/"                    { BEGIN(MULTILINE_COMMENT); }
\/\/.*
.                           { return TOKEN_ERROR; }

<MULTILINE_COMMENT>"\\\\\\" { BEGIN(INITIAL); }
<MULTILINE_COMMENT>\n       { lineNumber++; }
<MULTILINE_COMMENT>.

%%

int yywrap(void)
{
    running = 0;
    return 1;
}

int getLineNumber(void)
{
    return lineNumber + 1;
}

int isRunning(void)
{
    return running;
}

void initMe(void)
{
    lineNumber = 0;
    running = 1;
    symbol_table_init();
}

void freeMe(void)
{
    symbol_table_free();
}
