#ifndef HASHTABLE_H_
#define HASHTABLE_H_ 1

#include <stdio.h>

#include "types.h"
#include "token_data.h"

enum symbol_type {
    SYM_UNKNOWN,
    SYM_LIT_INT,
    SYM_LIT_FLOAT,
    SYM_LIT_CHAR,
    SYM_LIT_STR,
    SYM_UNKNOWN_IDENT,
    SYM_TMP_VAR,
    SYM_SCALAR_VAR,
    SYM_VECTOR_VAR,
    SYM_FUNCTION,
    SYM_LABEL,
    SYM_EXTERNAL,
    SYM_STR_ADDR,
    SYM_FLOAT_ADDR,
    SYM_ANNOTATION
};

struct sym_var_data {
    enum datatype type;
    size_t stack_frame_index;
    int in_scope;
    struct symbol *replacement;
};

struct str_lit_data {
    struct string_literal literal;
    struct symbol *identifier;
};

struct float_lit_data {
    double parsed;
    struct symbol *identifier;
};

struct symbol {
    char *content;
    enum symbol_type type;
    int line_number;
    union {
        long parsed_int;
        struct float_lit_data float_;
        char parsed_char;
        struct str_lit_data string; 
        struct sym_var_data variable;
        struct function_datatype function;
    } data;
};

void symbol_table_init(void);

void symbol_table_free(void);

struct symbol *symbol_table_insert(char const *content);

void symbol_table_debug(FILE *output);

char const *symbol_type_to_str(enum symbol_type type);

/* makeTemp */
struct symbol *symbol_table_create_tmp_scalar_var(enum datatype datatype);

/* makeLabel */
struct symbol *symbol_table_create_tmp_label(void);

struct symbol *symbol_table_create_char_lit(char value);

struct symbol *symbol_table_create_int_lit(long value);

struct symbol *symbol_table_create_float_lit(double value);

struct symbol *symbol_table_create_float_addr(void);

struct symbol *symbol_table_char_to_str_lit(char value);

struct symbol *symbol_table_rev_create_str_lit(struct string_literal literal);

struct symbol *symbol_table_create_str_lit(char const *zero_term_value);

struct symbol *symbol_table_create_str_addr(void);

int symbol_cmp(struct symbol *left, struct symbol *right);

#endif
