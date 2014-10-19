/*
    This header contains common functions and types used by tests.
*/

#ifndef LIBOT_TEST_COMMON_H
#define LIBOT_TEST_COMMON_H

#include <stdlib.h>
#include <stdarg.h>

// Debugging macros for creating a string containing the current line number.
// These are used by assertions so they can output where a test fails.
#define S(x) #x
#define S_(x) S(x)
#define STRLINE S_(__LINE__)

static void write_msg(char** msg, char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, ap) + 1;
    va_end(ap);

    *msg = malloc(size);

    va_start(ap, fmt);
    vsnprintf(*msg, size, fmt, ap);
    va_end(ap);
}

#endif
