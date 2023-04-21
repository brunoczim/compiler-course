#ifndef VECTOR_H_
#define VECTOR_H_ 1

#include <stddef.h>

void *vector_empty(size_t *length_out);

void *vector_singleton(size_t elem_size, size_t *length_out, void *data);

void *vector_push(
    void *buf,
    size_t elem_size,
    size_t *length_in_out,
    void const *data
);

void *vector_pop(
    void *buf,
    size_t elem_size,
    size_t *length_in_out,
    void *data_out
);

void *vector_append(
    void *dest_buf,
    void *src_buf,
    size_t elem_size,
    size_t *dest_length_in_out,
    size_t src_length
);

void *vector_splice(
    void *buf,
    size_t elem_size,
    size_t *length_in_out,
    size_t start,
    size_t end,
    void const *replacement,
    size_t repl_length,
    void *old
);

#endif
