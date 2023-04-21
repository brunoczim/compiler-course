#include "alloc.h"
#include "panic.h"
#include <stdlib.h>
#include <stdio.h>

void *aborting_malloc(size_t size)
{
    void *allocation = malloc(size);
    if (allocation == NULL && size != 0) {
        panic("failed to allocate %zu bytes", size);
    }
    return allocation;
}

void *aborting_realloc(void *allocation, size_t size)
{
    void *reallocation = realloc(allocation, size);
    if (reallocation == NULL && size != 0) {
        panic("failed to reallocate %zu bytes", size);
    }
    return reallocation;
}
