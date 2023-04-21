#ifndef TOKEN_DATA_H_
#define TOKEN_DATA_H_ 1

#include <stddef.h>

#define CHAR_LITERAL_EMIT_BUFSIZE 5

struct string_literal {
    char *buf;
    size_t length;
};

char char_literal_parse(char const *literal_content);

struct string_literal string_literal_parse(char const *literal_content);

void char_literal_emit(char character, char buf[CHAR_LITERAL_EMIT_BUFSIZE]);

char *string_literal_emit(struct string_literal literal);

void string_literal_free(struct string_literal literal);

#endif
