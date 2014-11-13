#include "../../xform.h"
#include "unit.h"

static bool param_xform_test(ot_op* initial, ot_op* op1, ot_op* op2,
                             ot_op* expected, char** msg) {
    ot_xform_pair xform = ot_xform(op1, op2);
    if (xform.op1_prime == NULL || xform.op2_prime == NULL) {
        FAIL("Transformation failed.", msg);
    }

    ot_op* composed1 = ot_compose(initial, op1);
    if (composed1 == NULL) {
        FAIL("Couldn't compose the first op.", msg);
    }
    ot_op* actual1 = ot_compose(composed1, xform.op2_prime);
    ASSERT_OP_EQUAL(expected, actual1, "op2' was incorrect.", msg);

    ot_op* composed2 = ot_compose(initial, op2);
    if (composed2 == NULL) {
        FAIL("Couldn't compose the second op.", msg);
    }
    ot_op* actual2 = ot_compose(composed2, xform.op1_prime);
    ASSERT_OP_EQUAL(expected, actual2, "op1' was incorrect.", msg);

    ot_free_op(initial);
    ot_free_op(op1);
    ot_free_op(op2);
    ot_free_op(expected);
    ot_free_op(xform.op1_prime);
    ot_free_op(xform.op2_prime);
    ot_free_op(actual1);
    ot_free_op(actual2);
    ot_free_op(composed1);
    ot_free_op(composed2);
    return true;
}

static bool xform_skip_skip(char** msg) {
    const char* const NONEMPTY_STRING = "abcこんにちは";
    const int string_length = utf8_length(NONEMPTY_STRING);

    ot_op* initial = ot_new_op();
    ot_insert(initial, NONEMPTY_STRING);

    ot_op* op1 = ot_new_op();
    ot_skip(op1, string_length);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, string_length);

    ot_op* expected = ot_new_op();
    ot_insert(expected, NONEMPTY_STRING);

    return param_xform_test(initial, op1, op2, expected, msg);
}

static bool xform_skip_insert(char** msg) {
    const char* const INSERT1 = "こんにちはdef";
    const char* const INSERT2 = "こんにちはabc";
    const char* const EXPECTED_INSERT = "こんにちはabcこんにちはdef";
    const int insert1_length = utf8_length(INSERT1);

    ot_op* initial = ot_new_op();
    ot_insert(initial, INSERT1);

    ot_op* op1 = ot_new_op();
    ot_skip(op1, insert1_length);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, INSERT2);
    ot_skip(op2, insert1_length);

    ot_op* expected = ot_new_op();
    ot_insert(expected, EXPECTED_INSERT);

    return param_xform_test(initial, op1, op2, expected, msg);
}

static bool xform_skip_delete(char** msg) {
    const char* const NONEMPTY_STRING = "abcこんにちは";
    const int string_length = utf8_length(NONEMPTY_STRING);

    ot_op* initial = ot_new_op();
    ot_insert(initial, NONEMPTY_STRING);

    ot_op* op1 = ot_new_op();
    ot_skip(op1, string_length);

    ot_op* op2 = ot_new_op();
    ot_delete(op2, string_length);

    ot_op* expected = ot_new_op();

    return param_xform_test(initial, op1, op2, expected, msg);
}

static bool xform_insert_insert(char** msg) {
    const char* const INSERT1 = "こんにちはabc";
    const char* const INSERT2 = "こんにちはdef";
    const char* const EXPECTED_INSERT = "こんにちはabcこんにちはdef";

    ot_op* initial = ot_new_op();

    ot_op* op1 = ot_new_op();
    ot_insert(op1, INSERT1);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, INSERT2);

    ot_op* expected = ot_new_op();
    ot_insert(expected, EXPECTED_INSERT);

    return param_xform_test(initial, op1, op2, expected, msg);
}

static bool xform_insert_delete(char** msg) {
    const char* const INITIAL = "こんにちはabc";
    const char* const INSERT = "こんにちはdef";
    const int initial_length = utf8_length(INITIAL);
    const int insert_length = utf8_length(INSERT);

    ot_op* initial = ot_new_op();
    ot_insert(initial, INITIAL);

    ot_op* op1 = ot_new_op();
    ot_insert(op1, INSERT);
    ot_skip(op1, insert_length);

    ot_op* op2 = ot_new_op();
    ot_delete(op2, initial_length);

    ot_op* expected = ot_new_op();
    ot_insert(expected, INSERT);

    return param_xform_test(initial, op1, op2, expected, msg);
}

static bool xform_delete_delete(char** msg) {
    const char* const INITIAL = "abcこんにちは";
    const int initial_length = utf8_length(INITIAL);

    ot_op* initial = ot_new_op();
    ot_insert(initial, INITIAL);

    ot_op* op1 = ot_new_op();
    ot_delete(op1, initial_length);

    ot_op* op2 = ot_new_op();
    ot_delete(op2, initial_length);

    ot_op* expected = ot_new_op();

    return param_xform_test(initial, op1, op2, expected, msg);
}

static bool xform_returned_ops_have_correct_clients_and_parents(char** msg) {
    const int NONZERO_INT = 1;
    const int OP1_NONEMPTY_HASH = 0xFF;
    const int OP2_NONEMPTY_HASH = 0xAA;

    ot_op* op1 = ot_new_op();
    op1->client_id = NONZERO_INT;
    memset(op1->hash, OP1_NONEMPTY_HASH, 20);

    ot_op* op2 = ot_new_op();
    op2->client_id = NONZERO_INT + 1;
    memset(op2->hash, OP2_NONEMPTY_HASH, 20);

    ot_xform_pair xform = ot_xform(op1, op2);

    ASSERT_INT_EQUAL(op1->client_id, xform.op1_prime->client_id,
                     "op1' client ID was incorrect.", msg);
    bool parents_equal = (memcmp(op2->hash, xform.op1_prime->parent, 20) == 0);
    if (!parents_equal) {
        FAIL("op1' parent was incorrect.", msg);
    }

    ASSERT_INT_EQUAL(op2->client_id, xform.op2_prime->client_id,
                     "op2' client ID was incorrect.", msg);
    parents_equal = (memcmp(op1->hash, xform.op2_prime->parent, 20) == 0);
    if (!parents_equal) {
        FAIL("op2' parent was incorrect.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    ot_free_op(xform.op1_prime);
    ot_free_op(xform.op2_prime);
    return true;
}

static bool xform_returns_null_when_xform_fails(char** msg) {
    ot_op* op1 = ot_new_op();
    ot_skip(op1, 1);

    ot_op* op2 = ot_new_op();
    ot_skip(op1, 2);

    ot_xform_pair pair = ot_xform(op1, op2);
    if (pair.op1_prime != NULL || pair.op2_prime != NULL) {
        FAIL("Transform didn't fail.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

results xform_tests() {
    RUN_TEST(xform_skip_skip);
    RUN_TEST(xform_skip_insert);
    RUN_TEST(xform_skip_delete);
    RUN_TEST(xform_insert_insert);
    RUN_TEST(xform_insert_delete);
    RUN_TEST(xform_delete_delete);
    RUN_TEST(xform_returned_ops_have_correct_clients_and_parents);
    RUN_TEST(xform_returns_null_when_xform_fails);

    return (results) { passed, failed };
}
