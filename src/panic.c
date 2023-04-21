#include "panic.h"
#include <stdlib.h>
#include <stdio.h>

void PANIC_NO_RETURN vpanic(char const *msg, va_list vargs)
{
    fputs("Internal error: ", stderr);
    vfprintf(stderr, msg, vargs);
    fputs(", aborting...\n", stderr);
    abort();
    exit(-1);
}

void PANIC_NO_RETURN panic(char const *msg, ...)
{
    va_list vargs;
    va_start(vargs, msg);
    vpanic(msg, vargs);
    va_end(vargs);
}
