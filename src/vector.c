#include "vector.h"
#include "alloc.h"
#include <stdlib.h>
#include <string.h>

void *vector_empty(size_t *length_out)
{
    *length_out = 0;
    return NULL;
}

void *vector_singleton(size_t elem_size, size_t *length_out, void *data)
{
    void *buf = aborting_malloc(elem_size);
    memcpy((unsigned char *) buf, data, elem_size);
    *length_out = 1;
    return buf;
}

void *vector_push(
    void *buf,
    size_t elem_size,
    size_t *length_in_out,
    void const *data
) {
    void *new_buf = aborting_realloc(buf, elem_size * (*length_in_out + 1));
    memcpy(
        (unsigned char *) new_buf + elem_size * *length_in_out,
        data,
        elem_size
    );
    *length_in_out += 1;
    return new_buf;
}

void *vector_pop(
    void *buf,
    size_t elem_size,
    size_t *length_in_out,
    void *data_out
)
{
    void *new_buf;
    memcpy(
        data_out,
        (unsigned char *) buf + elem_size * (*length_in_out - 1),
        elem_size
    );
    if (*length_in_out == 1) {
        free(buf);
        new_buf = NULL;
    } else {
        new_buf = aborting_realloc(buf, elem_size * (*length_in_out - 1));
    }
    *length_in_out -= 1;
    return new_buf;
}

void *vector_append(
    void *dest_buf,
    void *src_buf,
    size_t elem_size,
    size_t *dest_length_in_out,
    size_t src_length
)
{
    void *new_buf;
    if (*dest_length_in_out + src_length == 0) {
        new_buf = NULL;
    } else {
        new_buf = aborting_realloc(
            dest_buf,
            elem_size * (*dest_length_in_out + src_length)
        );
        memcpy(
            (unsigned char *) new_buf + elem_size * *dest_length_in_out,
            src_buf,
            elem_size * src_length
        );
    }
    *dest_length_in_out += src_length;
    free(src_buf);
    return new_buf;
}

void *vector_splice(
    void *buf,
    size_t elem_size,
    size_t *length_in_out,
    size_t start,
    size_t end,
    void const *replacement,
    size_t repl_length,
    void *old
)
{
    void *new_buf;
    size_t range_length = end - start;

    if (old != NULL) {
        memcpy(
            old,
            (unsigned char *) buf + start * *length_in_out,
            elem_size * range_length
        );
    }
    if (range_length >= repl_length) {
        memmove(
            (unsigned char *) buf + elem_size * (start + repl_length),
            (unsigned char *) buf + elem_size * end,
            elem_size * (*length_in_out - end)
        );
    }
    new_buf = aborting_realloc(
        buf,
        elem_size * (*length_in_out - range_length + repl_length)
    );
    if (range_length < repl_length) {
        memmove(
            (unsigned char *) new_buf + elem_size * (start + repl_length),
            (unsigned char *) new_buf + elem_size * end,
            elem_size * (*length_in_out - end)
        );
    }
    memcpy(
        (unsigned char *) new_buf + elem_size * start,
        replacement,
        elem_size * repl_length
    );
    *length_in_out = *length_in_out - range_length + repl_length;
    return new_buf;
}
