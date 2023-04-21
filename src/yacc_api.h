#ifndef YACC_API_H_
#define YACC_API_H_ 1

extern FILE *yyin;

extern char *yytext;

int yyparse(void);

int yylex(void);

void yyerror(char const *message);

#endif
