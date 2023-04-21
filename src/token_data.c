#include "token_data.h"
#include "alloc.h"
#include <stdlib.h>
#include <string.h>

static char escape_char(char code);

static int emit_char(char character, char *buf, size_t bufsize, size_t *length);

char char_literal_parse(char const *literal_content)
{
    char parsed;
    if (literal_content[1] == '\\') {
        parsed = escape_char(literal_content[2]);
    } else {
        parsed = literal_content[1];
    }
    return parsed;
}

struct string_literal string_literal_parse(char const *literal_content)
{
    size_t i;
    size_t size;
    struct string_literal literal;

    size = strlen(literal_content);
    if (size >= 2) {
        size -= 2;
    }

    literal.length = 0;
    literal.buf = aborting_malloc(size + 1);
    literal.buf[0] = 0;

    i = 1;

    while (literal_content[i] != '"') {
        literal.length += 1;
        if (literal.length > size) {
            literal.buf = aborting_realloc(
                literal.buf,
                size + 1
            );
            literal.buf[size] = 0;
            size += 1;
            literal.length += 1;
        }
        if (literal_content[i] == '\\') {
            i++;
            literal.buf[literal.length - 1] = escape_char(literal_content[i]);
        } else {
            literal.buf[literal.length - 1] = literal_content[i];
        }
        i++;
    }

    return literal;
}

void char_literal_emit(char character, char buf[CHAR_LITERAL_EMIT_BUFSIZE])
{
    size_t length;
    buf[0] = '\'';
    length = 1;
    emit_char(character, buf, CHAR_LITERAL_EMIT_BUFSIZE, &length);
    buf[length] = '\'';
    buf[length + 1] = 0;
}

char *string_literal_emit(struct string_literal literal)
{
    size_t size, length, i;
    char *buf;
    size = literal.length + 1;
    buf = aborting_malloc(size + 2);
    buf[0] = '"';
    buf[1] = '"';
    buf[2] = 0;
    
    i = 0;
    length = 1;

    while (i < literal.length) {
        while (!emit_char(literal.buf[i], buf, size, &length)) {
            size += 1;
            buf = aborting_realloc(buf, size + 2);
            buf[size] = 0;
        }
        buf[size] = '"';
        buf[size + 1] = 0;
        i++;
    }

    return buf;
}

void string_literal_free(struct string_literal literal)
{
    free(literal.buf);
}

static char escape_char(char code)
{
    switch (code) {
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '0':
            return 0;
        default:
            return code;
    }
}

static int emit_char(char character, char *buf, size_t bufsize, size_t *length)
{
    switch (character) {
        case '\n':
            if (bufsize < *length + 2) {
                return 0;
            }
            buf[*length] = '\\';
            buf[*length + 1] = 'n';
            *length += 2;
            return 1;
        case '\t':
            if (bufsize < *length + 2) {
                return 0;
            }
            buf[*length] = '\\';
            buf[*length + 1] = 't';
            *length += 2;
            return 1;
        case '\r':
            if (bufsize < *length + 2) {
                return 0;
            }
            buf[*length] = '\\';
            buf[*length + 1] = 'r';
            *length += 2;
            return 1;
        case '\'':
            if (bufsize < *length + 2) {
                return 0;
            }
            buf[*length] = '\\';
            buf[*length + 1] = character;
            *length += 2;
            return 1;
        case '"':
            if (bufsize < *length + 2) {
                return 0;
            }
            buf[*length] = '\\';
            buf[*length + 1] = '"';
            *length += 2;
            return 1;
        case 0:
            if (bufsize < *length + 2) {
                return 0;
            }
            buf[*length] = '\\';
            buf[*length + 1] = '0';
            *length += 2;
            return 1;
        default:
            if (bufsize < *length + 1) {
                return 0;
            }
            buf[*length] = character;
            *length += 1;
            return 1;
    }
}
