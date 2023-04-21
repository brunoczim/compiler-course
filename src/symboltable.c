#include "panic.h"
#include "symboltable.h"
#include "token_data.h"
#include "lexer.h"
#include "alloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define HASHTABLE_CAPACITY 16411

struct bucket_node {
    struct symbol symbol;
    struct bucket_node *next;
};

struct symbol_table {
    struct bucket_node *buckets[HASHTABLE_CAPACITY];
};

struct symbol_table g_symbol_table;

static size_t hash_string(char const *content);

static struct bucket_node **findbucket_node(char const *content);

void symbol_table_init(void)
{
    size_t i;
    for (i = 0; i < HASHTABLE_CAPACITY; i++) {
        g_symbol_table.buckets[i] = NULL;
    }
}

void symbol_table_free(void)
{
    size_t i;
    struct bucket_node *node_ptr;
    struct bucket_node *next;

    for (i = 0; i < HASHTABLE_CAPACITY; i++) {
        node_ptr = g_symbol_table.buckets[i];
        while (node_ptr != NULL) {
            switch (node_ptr->symbol.type) {
                case SYM_LIT_STR:
                    string_literal_free(node_ptr->symbol.data.string.literal);
                    break;
                case SYM_FUNCTION:
                    free(node_ptr->symbol.data.function.parameter_types.types);
                    break;
                case SYM_UNKNOWN:
                case SYM_UNKNOWN_IDENT:
                case SYM_SCALAR_VAR:
                case SYM_TMP_VAR:
                case SYM_VECTOR_VAR:
                case SYM_LIT_INT:
                case SYM_LIT_FLOAT:
                case SYM_LIT_CHAR:
                case SYM_LABEL:
                case SYM_EXTERNAL:
                case SYM_STR_ADDR:
                case SYM_FLOAT_ADDR:
                case SYM_ANNOTATION:
                    break;
                default:
                    panic(
                       "symbol type %i not handled at symbol table's destroyal",
                       node_ptr->symbol.type
                    );
            }
            free(node_ptr->symbol.content);
            next = node_ptr->next;
            free(node_ptr);
            node_ptr = next;
        }
    }
}

struct symbol *symbol_table_insert(char const *content)
{
    struct bucket_node **bucket_node;

    bucket_node = findbucket_node(content);

    if (*bucket_node == NULL) {
        *bucket_node = aborting_malloc(sizeof(struct bucket_node));
        (*bucket_node)->next = NULL;
        (*bucket_node)->symbol.content = aborting_malloc(
            strlen(content) + 1
        );
        strcpy((*bucket_node)->symbol.content, content);
        (*bucket_node)->symbol.type = SYM_UNKNOWN;
        (*bucket_node)->symbol.data.variable.replacement = NULL;
        (*bucket_node)->symbol.line_number = getLineNumber();
    } 

    return &(*bucket_node)->symbol;
}

static struct bucket_node **findbucket_node(char const *content)
{
    long unsigned hash = hash_string(content);
    size_t index = hash % HASHTABLE_CAPACITY;
    struct bucket_node **bucket_ptr = &g_symbol_table.buckets[index];
    while (
        *bucket_ptr != NULL
        && strcmp(content, (*bucket_ptr)->symbol.content) != 0
    ) {
        bucket_ptr = &(*bucket_ptr)->next;
    }
    return bucket_ptr;
}

static long unsigned hash_string(char const *content)
{
    long unsigned hash = 4294967279ul;
    long unsigned index = 0ul;

    while (content[index]) {
        hash = hash * 2684354527;
        hash ^= ((unsigned char) content[index]) * 4294967231;
        hash ^= hash << 19;
        hash ^= index * 2684354399;
        index++;
    }

    hash ^= index * 4294967291;

    return hash;
}

void symbol_table_debug(FILE *output)
{
    size_t i;
    struct bucket_node *node;

    for (i = 0; i < HASHTABLE_CAPACITY; i++) {
        for (node = g_symbol_table.buckets[i]; node != NULL; node = node->next)
        {
            fprintf(
                output,
                "SYMBOLS[%zu] = (%d, %s)\n",
                i,
                node->symbol.type,
                node->symbol.content
            );
        }
    }
}

char const *symbol_type_to_str(enum symbol_type type)
{
    switch (type) {
        case SYM_UNKNOWN: return "unknown symbol";
        case SYM_LABEL: return "label";
        case SYM_EXTERNAL: return "external";
        case SYM_STR_ADDR: return "string address";
        case SYM_FLOAT_ADDR: return "float address";
        case SYM_ANNOTATION: return "annotation";
        case SYM_LIT_INT: return "integer literal";
        case SYM_LIT_FLOAT: return "float literal";
        case SYM_LIT_CHAR: return "character literal";
        case SYM_LIT_STR: return "string literal";
        case SYM_UNKNOWN_IDENT: return "unknown identifier";
        case SYM_SCALAR_VAR: return "scalar variable";
        case SYM_VECTOR_VAR: return "vector variable";
        case SYM_FUNCTION: return "function identifier";
        case SYM_TMP_VAR: return "temporary scalar variable";
    }
    panic("symbol type %i's to string not implemented", type);
}

struct symbol *symbol_table_create_tmp_scalar_var(enum datatype datatype)
{
    static unsigned long id = 0;
    struct symbol *symbol;
    char buf[100];

    snprintf(buf, sizeof(buf), "@scalar_%lu", id);
    id++;
    symbol = symbol_table_insert(buf);
    symbol->type = SYM_TMP_VAR;
    symbol->data.variable.type = datatype;
    symbol->data.variable.stack_frame_index = SIZE_MAX;
    return symbol;
}

struct symbol *symbol_table_create_tmp_label(void)
{
    static unsigned long id = 0;
    struct symbol *symbol;
    char buf[100];

    snprintf(buf, sizeof(buf), "@label_%lu", id);
    id++;
    symbol = symbol_table_insert(buf);
    symbol->type = SYM_LABEL;
    return symbol;
}

struct symbol *symbol_table_create_char_lit(char value)
{
    struct symbol *symbol;
    char buf[CHAR_LITERAL_EMIT_BUFSIZE];
    char_literal_emit(value, buf);

    symbol = symbol_table_insert(buf);
    symbol->type = SYM_LIT_CHAR;
    symbol->data.parsed_char = value;
    return symbol;
}

struct symbol *symbol_table_create_int_lit(long value)
{
    struct symbol *symbol;
    char buf[100];
    snprintf(buf, sizeof(buf), "%li", value);
    symbol = symbol_table_insert(buf);
    symbol->type = SYM_LIT_INT;
    symbol->data.parsed_int = value;
    return symbol;
}

struct symbol *symbol_table_create_float_lit(double value)
{
    size_t i;
    struct symbol *symbol;
    char buf[1 + 309 + 1 + 1076 + 1] = { 0 };
    i = snprintf(buf, sizeof(buf), "%.1074f", value);

    while (buf[i - 2] != '.' && (buf[i - 1] == 0 || buf[i - 1] == '0')) {
        i--;
        buf[i] = 0;
    }

    symbol = symbol_table_insert(buf);
    if (symbol->type == SYM_UNKNOWN) {
        symbol->type = SYM_LIT_FLOAT;
        symbol->data.float_.parsed = value;
        symbol->data.float_.identifier = NULL;
    }
    return symbol;
}

struct symbol *symbol_table_char_to_str_lit(char value)
{
    char buf[2] = { value, 0 };
    struct string_literal literal;
    literal.length = 1;
    literal.buf = buf;
    return symbol_table_rev_create_str_lit(literal);
}

struct symbol *symbol_table_rev_create_str_lit(struct string_literal literal)
{
    char *buf_emited;
    struct symbol *symbol;

    buf_emited = string_literal_emit(literal);

    symbol = symbol_table_insert(buf_emited);
    if (symbol->type == SYM_UNKNOWN) {
        symbol->type = SYM_LIT_STR;
        symbol->data.string.literal = string_literal_parse(symbol->content);
        symbol->data.string.identifier = NULL;
    }
    free(buf_emited);

    return symbol;
}

struct symbol *symbol_table_create_str_lit(char const *zero_term_value)
{
    struct string_literal literal;
    size_t length = strlen(zero_term_value);

    literal.length = length;
    literal.buf = (char *) zero_term_value;

    return symbol_table_rev_create_str_lit(literal);
}

struct symbol *symbol_table_create_str_addr(void)
{
    static unsigned long id = 0;
    struct symbol *symbol;
    char buf[100];

    snprintf(buf, sizeof(buf), "@string_%lu", id);
    id++;
    symbol = symbol_table_insert(buf);
    symbol->type = SYM_STR_ADDR;
    return symbol;
}

struct symbol *symbol_table_create_float_addr(void)
{
    static unsigned long id = 0;
    struct symbol *symbol;
    char buf[100];

    snprintf(buf, sizeof(buf), "@float_%lu", id);
    id++;
    symbol = symbol_table_insert(buf);
    symbol->type = SYM_FLOAT_ADDR;
    return symbol;
}

int symbol_cmp(struct symbol *left, struct symbol *right)
{
    size_t i;
    int cmp = 0;
    double fcmp;
    int null = 0;
    if (left == NULL) {
        cmp++;
        null = 1;
    }
    if (right == NULL) {
        cmp--;
        null = 1;
    }
    if (null) {
        return cmp;
    }
    cmp = left->type - right->type;
    if (cmp != 0) {
        return cmp;
    }

    switch (left->type) {
        case SYM_UNKNOWN:
            return 0;
        case SYM_LIT_INT:
            return left->data.parsed_int - right->data.parsed_int;
        case SYM_LIT_FLOAT:
            fcmp = left->data.float_.parsed - right->data.float_.parsed;
            return (0 < fcmp) - (fcmp < 0);
        case SYM_LIT_CHAR:
            return left->data.parsed_char - right->data.parsed_char;
        case SYM_LIT_STR:
            i = 0;
            while (
                i < left->data.string.literal.length
                && i < right->data.string.literal.length
                && cmp == 0
            ) {
                cmp = left->data.string.literal.buf[i]
                    - right->data.string.literal.buf[i];
                i++;
            }
            if (cmp == 0) {
                cmp = (int) left->data.string.literal.length
                    - right->data.string.literal.length;
            }
            return cmp;
        case SYM_UNKNOWN_IDENT:
        case SYM_TMP_VAR:
        case SYM_SCALAR_VAR:
        case SYM_VECTOR_VAR:
        case SYM_FUNCTION:
        case SYM_LABEL:
        case SYM_EXTERNAL:
        case SYM_STR_ADDR:
        case SYM_FLOAT_ADDR:
        case SYM_ANNOTATION:
            return strcmp(left->content, right->content);
        default:
            panic("symbol type %i's equality not implemented", left->type);
    }
}
