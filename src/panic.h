#ifndef PANIC_H_
#define PANIC_H_ 1
#include <stdarg.h>

#if __STDC_VERSION__ >= 201112L
#   define PANIC_NO_RETURN _Noreturn
#else
#   define PANIC_NO_RETURN
#endif

void PANIC_NO_RETURN vpanic(char const *msg, va_list vargs);

void PANIC_NO_RETURN panic(char const *msg, ...);

#endif
