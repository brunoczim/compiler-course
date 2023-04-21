#ifndef ALLOC_H_
#define ALLOC_H_ 1

#include <stddef.h>

void *aborting_malloc(size_t size);

void *aborting_realloc(void *allocation, size_t size);

#endif
