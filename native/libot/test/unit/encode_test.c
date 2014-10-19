#include "unit.h"

static bool encode_err_returns_correct_json(char** msg) {
    const char* const EXPECTED = "{\"errorCode\":1}";
    char* actual = ot_encode_err(1);

    ASSERT_STR_EQUAL(EXPECTED, actual, "Encoded error JSON was incorrect.",
                     msg);

    free(actual);
    return true;
}

results encode_tests() {
    RUN_TEST(encode_err_returns_correct_json);

    return (results) { passed, failed };
}
