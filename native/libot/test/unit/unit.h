/*
    This header contains a minimal unit testing framework.
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../../encode.h"
#include "../../ot.h"
#include "../common.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

static char* msg = NULL;
static int passed = 0;
static int failed = 0;

typedef struct results {
    int passed;
    int failed;
} results;

#define RUN_TEST(f)                                                            \
    bool f(char**);                                                            \
    msg = NULL;                                                                \
    printf("Running %s... ", #f);                                              \
    if (f(&msg)) {                                                             \
        puts("passed.");                                                       \
        ++passed;                                                              \
    } else {                                                                   \
        puts("failed.");                                                       \
        ++failed;                                                              \
    }                                                                          \
    if (msg) {                                                                 \
        printf("\t%s\n", msg);                                                 \
    }

#define RUN_SUITE(f)                                                           \
    {                                                                          \
        results res = f();                                                     \
        passed += res.passed;                                                  \
        failed += res.failed;                                                  \
    }

#define ASSERT_CONDITION(condition, expected, actual, detail, msg)             \
    if (!assert_condition(condition, expected, actual, detail,                 \
                          "Line #" STRLINE, msg)) {                            \
        return false;                                                          \
    }

#define ASSERT_INT_EQUAL(expected, actual, detail, msg)                        \
    if (!assert_int_equal(expected, actual, detail, "Line #" STRLINE, msg)) {  \
        return false;                                                          \
    }

#define ASSERT_STR_EQUAL(expected, actual, detail, msg)                        \
    if (!assert_str_equal(expected, actual, detail, "Line #" STRLINE, msg)) {  \
        return false;                                                          \
    }

#define ASSERT_OP_EQUAL(expected, actual, detail, msg)                         \
    if (!assert_op_equal(expected, actual, detail, "Line #" STRLINE, msg)) {   \
        return false;                                                          \
    }

#define FAIL(detail, msg)                                                      \
    write_msg(msg, "Line #" STRLINE ": %s", detail);                           \
    return false

static bool assert_condition(bool condition, const char* expected,
                             const char* actual, const char* detail,
                             const char* loc, char** msg) {

    if (condition) {
        return true;
    }

    write_msg(msg, "%s: %s\n"
                   "\tActual: %s\n"
                   "\tExpected: %s",
              loc, detail, actual, expected);
    return false;
}

static bool assert_int_equal(int expected, int actual, char* detail, char* loc,
                             char** msg) {

    if (expected == actual) {
        return true;
    }

    write_msg(msg, "%s: %s\n"
                   "\tActual: %d\n"
                   "\tExpected: %d",
              loc, detail, actual, expected);
    return false;
}

static bool assert_str_equal(const char* expected, const char* actual,
                             const char* detail, const char* loc, char** msg) {

    int cmp = strcmp(expected, actual);
    if (cmp == 0) {
        return true;
    }

    write_msg(msg, "%s: %s\n"
                   "\tActual: %s\n"
                   "\tExpected: %s",
              loc, detail, actual, expected);
    return false;
}

static bool assert_op_equal(const ot_op* const expected,
                            const ot_op* const actual, const char* detail,
                            const char* loc, char** msg) {

    bool equal = ot_equal(expected, actual);
    if (equal) {
        return true;
    }

    char* expected_enc = ot_encode(expected);
    char* actual_enc = ot_encode(actual);
    write_msg(msg, "%s: %s\n"
                   "\tActual: %s\n"
                   "\tExpected: %s",
              loc, detail, actual_enc, expected_enc);

    free(expected_enc);
    free(actual_enc);
    return false;
}

#pragma clang diagnostic pop
