#ifndef PARSER_H_
#define PARSER_H_ 1

#include "lexer.h"
#include "yacc_api.h"
#include "symboltable.h"
#include "ast.h"
#include "y.tab.h"

int parse(void);

#endif
