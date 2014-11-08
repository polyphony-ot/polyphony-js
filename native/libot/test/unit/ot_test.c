#include "../../ot.h"
#include "../../decode.h"
#include "unit.h"

static bool start_fmt_appends_correct_comp_type(char** msg) {
    const ot_comp_type EXPECTED_TYPE = OT_FORMATTING_BOUNDARY;
    const char* const ANY_NAME = "any name";
    const char* const ANY_VALUE = "any value";

    ot_op* op = ot_new_op();
    ot_start_fmt(op, ANY_NAME, ANY_VALUE);

    ot_comp* comps = op->comps.data;
    ot_comp_type actual_type = comps[0].type;

    ASSERT_INT_EQUAL(EXPECTED_TYPE, actual_type, "Starting a format did not "
                                                 "append a component of type "
                                                 "OT_FORMATTING_BOUNDARY.",
                     msg);

    ot_free_op(op);
    return true;
}

static bool start_fmt_appends_correct_name_and_value(char** msg) {
    const char* const EXPECTED_NAME = "any name";
    const char* const EXPECTED_VALUE = "any value";

    ot_op* op = ot_new_op();
    ot_start_fmt(op, EXPECTED_NAME, EXPECTED_VALUE);

    ot_comp* comps = op->comps.data;
    ot_fmt* start_fmts = comps[0].value.fmtbound.start.data;
    char* actual_name = start_fmts[0].name;
    char* actual_value = start_fmts[0].value;

    ASSERT_STR_EQUAL(EXPECTED_NAME, actual_name,
                     "Appended format did not have the correct name.", msg);
    ASSERT_STR_EQUAL(EXPECTED_VALUE, actual_value,
                     "Appended format did not have the correct value.", msg);

    ot_free_op(op);
    return true;
}

static bool start_fmt_merges_fmtbound_with_previous_fmtbound(char** msg) {
    const size_t EXPECTED_COMP_COUNT = 1;
    const char* const ANY_NAME = "any name";
    const char* const ANY_VALUE = "any value";

    ot_op* op = ot_new_op();
    ot_start_fmt(op, ANY_NAME, ANY_VALUE);
    ot_start_fmt(op, ANY_NAME, ANY_VALUE);

    size_t actual_comp_count = op->comps.len;

    ASSERT_INT_EQUAL(EXPECTED_COMP_COUNT, actual_comp_count,
                     "Appended format boundary wasn't merged with the previous "
                     "format boundary.",
                     msg);

    ot_free_op(op);
    return true;
}

static bool end_fmt_appends_correct_name_and_value(char** msg) {
    char* EXPECTED_NAME = "any name";
    char* EXPECTED_VALUE = "any value";

    ot_op* op = ot_new_op();
    ot_end_fmt(op, EXPECTED_NAME, EXPECTED_VALUE);

    ot_comp* comps = op->comps.data;
    ot_fmt* end_fmts = comps[0].value.fmtbound.end.data;
    char* actual_name = end_fmts[0].name;
    char* actual_value = end_fmts[0].value;

    ASSERT_STR_EQUAL(EXPECTED_NAME, actual_name,
                     "Appended format did not have the correct name.", msg);
    ASSERT_STR_EQUAL(EXPECTED_VALUE, actual_value,
                     "Appended format did not have the correct value.", msg);

    ot_free_op(op);
    return true;
}

static bool end_fmt_merges_fmtbound_with_previous_fmtbound(char** msg) {
    const size_t EXPECTED_COMP_COUNT = 1;
    const char* const ANY_NAME = "any name";
    const char* const ANY_VALUE = "any value";

    ot_op* op = ot_new_op();
    ot_end_fmt(op, ANY_NAME, ANY_VALUE);
    ot_end_fmt(op, ANY_NAME, ANY_VALUE);

    size_t actual_comp_count = op->comps.len;

    ASSERT_INT_EQUAL(EXPECTED_COMP_COUNT, actual_comp_count,
                     "Appended format boundary wasn't merged with the previous "
                     "format boundary.",
                     msg);

    ot_free_op(op);
    return true;
}

static bool iter_next_on_empty_op(char** msg) {
    ot_op* op = ot_new_op();
    ot_iter iter;

    ot_iter_init(&iter, op);
    bool actual = ot_iter_next(&iter);

    if (actual != false) {
        FAIL("Expected number of iterations did not equal actual number of "
             "iterations.",
             msg);
        return false;
    }

    ot_free_op(op);
    return true;
}

static bool iter_next_iterates_once_over_skip_with_count_one(char** msg) {
    const size_t EXPECTED_ITERATIONS = 1;
    ot_op* op = ot_new_op();
    ot_skip(op, 1);
    ot_iter iter;

    ot_iter_init(&iter, op);
    size_t actual_iterations = 0;
    while (ot_iter_next(&iter)) {
        actual_iterations++;
    }

    ASSERT_INT_EQUAL(EXPECTED_ITERATIONS, actual_iterations,
                     "Expected number of iterations didn't equal actual number "
                     "of iterations.",
                     msg)

    ot_free_op(op);
    return true;
}

static bool iter_next_iterates_skip_with_count_greater_than_one(char** msg) {
    const size_t EXPECTED_ITERATIONS = 2;
    ot_op* op = ot_new_op();
    ot_skip(op, 2);
    ot_iter iter;

    ot_iter_init(&iter, op);
    size_t actual_iterations = 0;
    while (ot_iter_next(&iter)) {
        actual_iterations++;
    }

    ASSERT_INT_EQUAL(EXPECTED_ITERATIONS, actual_iterations,
                     "Expected number of iterations didn't equal actual number "
                     "of iterations.",
                     msg)

    ot_free_op(op);
    return true;
}

static bool iter_next_iterates_over_single_insert_component(char** msg) {
    ot_op* op = ot_new_op();
    ot_insert(op, "012");
    ot_iter iter;

    ot_iter_init(&iter, op);
    ot_iter_next(&iter);
    ASSERT_INT_EQUAL(0, iter.pos, "Iterator position was incorrect.", msg);
    ASSERT_INT_EQUAL(0, iter.offset, "Iterator offset was incorrect.", msg);

    ot_iter_next(&iter);
    ASSERT_INT_EQUAL(0, iter.pos, "Iterator position was incorrect.", msg);
    ASSERT_INT_EQUAL(1, iter.offset, "Iterator offset was incorrect.", msg);

    ot_iter_next(&iter);
    ASSERT_INT_EQUAL(0, iter.pos, "Iterator position was incorrect.", msg);
    ASSERT_INT_EQUAL(2, iter.offset, "Iterator offset was incorrect.", msg);

    ot_free_op(op);
    return true;
}

static bool equal_returns_true_for_two_equal_inserts(char** msg) {
    const char* const NONEMPTY_STRING = "abc";

    ot_op* op1 = ot_new_op();
    ot_insert(op1, NONEMPTY_STRING);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, NONEMPTY_STRING);

    bool equal = ot_equal(op1, op2);
    if (!equal) {
        FAIL("Expected the operations to be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_true_for_two_equal_skips(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* op1 = ot_new_op();
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, NONZERO_INT);

    bool equal = ot_equal(op1, op2);
    if (!equal) {
        FAIL("Expected the operations to be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_false_for_skips_with_different_counts(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* op1 = ot_new_op();
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, NONZERO_INT + 1);

    bool equal = ot_equal(op1, op2);
    if (equal) {
        FAIL("Expected the operations to not be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_true_for_two_empty_operations(char** msg) {
    ot_op* op1 = ot_new_op();
    ot_op* op2 = ot_new_op();

    bool equal = ot_equal(op1, op2);
    if (!equal) {
        FAIL("Expected the operations to be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_false_for_ops_with_different_clients(char** msg) {
    const int NONZERO_INT = 1;

    ot_op* op1 = ot_new_op();
    op1->client_id = NONZERO_INT;

    ot_op* op2 = ot_new_op();
    op2->client_id = NONZERO_INT + 1;

    bool equal = ot_equal(op1, op2);
    if (equal) {
        FAIL("Expected the operations to not be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_true_for_two_operations_with_same_parent(char** msg) {
    const char* const NONEMPTY_HASH = "cafebabe";

    ot_op* op1 = ot_new_op();
    strcpy(op1->parent, NONEMPTY_HASH);

    ot_op* op2 = ot_new_op();
    strcpy(op2->parent, NONEMPTY_HASH);

    bool equal = ot_equal(op1, op2);
    if (!equal) {
        FAIL("Expected the operations to be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_true_for_operations_with_insert_and_skip(char** msg) {
    const int NONZERO_INT = 1;
    const char* const NONEMPTY_STRING = "abc";

    ot_op* op1 = ot_new_op();
    ot_insert(op1, NONEMPTY_STRING);
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, NONEMPTY_STRING);
    ot_skip(op2, NONZERO_INT);

    bool equal = ot_equal(op1, op2);
    if (!equal) {
        FAIL("Expected the operations to be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool equal_returns_false_for_ops_with_different_lengths(char** msg) {
    const int NONZERO_INT = 1;
    const char* const NONEMPTY_STRING = "abc";

    ot_op* op1 = ot_new_op();
    ot_insert(op1, NONEMPTY_STRING);
    ot_skip(op1, NONZERO_INT);

    ot_op* op2 = ot_new_op();
    ot_insert(op2, NONEMPTY_STRING);

    bool equal = ot_equal(op1, op2);
    if (equal) {
        FAIL("Expected the operations to not be equal.", msg);
    }

    ot_free_op(op1);
    ot_free_op(op2);
    return true;
}

static bool dup_duplicates_op_with_one_component(char** msg) {
    const char* const NONEMPTY_STRING = "abc";

    ot_op* orig = ot_new_op();
    ot_insert(orig, NONEMPTY_STRING);

    ot_op* dup = ot_dup_op(orig);

    bool equal = ot_equal(orig, dup);
    if (!equal) {
        FAIL("Duplicated op wasn't equal to the original op.", msg);
    }

    ot_free_op(orig);
    ot_free_op(dup);
    return true;
}

static bool size_with_one_insert(char** msg) {
    const char* const NONEMPTY_STRING = "abc";
    const int EXPECTED_SIZE = 3;

    ot_op* op = ot_new_op();
    ot_insert(op, NONEMPTY_STRING);

    uint32_t actual_size = ot_size(op);

    ASSERT_INT_EQUAL(EXPECTED_SIZE, actual_size, "Unexpected op size.", msg);

    ot_free_op(op);
    return true;
}

static bool size_with_empty_op(char** msg) {
    const int EXPECTED_SIZE = 0;

    ot_op* op = ot_new_op();
    uint32_t actual_size = ot_size(op);

    ASSERT_INT_EQUAL(EXPECTED_SIZE, actual_size, "Unexpected op size.", msg);

    ot_free_op(op);
    return true;
}

static bool size_of_op_with_only_inserts_equals_length_of_snapshot(char** msg) {
    const char* const NONEMPTY_STRING = "abc";

    ot_op* op = ot_new_op();
    ot_insert(op, NONEMPTY_STRING);

    char* snapshot = ot_snapshot(op);
    uint32_t snapshot_len = strlen(snapshot);
    uint32_t size = ot_size(op);

    ASSERT_INT_EQUAL(snapshot_len, size,
                     "Size was not equal to the length of the op's snapshot.",
                     msg);

    ot_free_op(op);
    free(snapshot);
    return true;
}

static bool size_with_delete(char** msg) {
    const int EXPECTED_SIZE = -1;
    const int DELETE_COUNT = 1;

    ot_op* op = ot_new_op();
    ot_delete(op, DELETE_COUNT);
    uint32_t actual_size = ot_size(op);

    ASSERT_INT_EQUAL(EXPECTED_SIZE, actual_size, "Unexpected op size.", msg);

    ot_free_op(op);
    return true;
}

static bool size_with_delete_and_insert(char** msg) {
    const int EXPECTED_SIZE = 0;
    const int DELETE_COUNT = 1;
    const char* const INSERT_TEXT = "a";

    ot_op* op = ot_new_op();
    ot_insert(op, INSERT_TEXT);
    ot_delete(op, DELETE_COUNT);
    uint32_t actual_size = ot_size(op);

    ASSERT_INT_EQUAL(EXPECTED_SIZE, actual_size, "Unexpected op size.", msg);

    ot_free_op(op);
    return true;
}

results ot_tests() {
    RUN_TEST(start_fmt_appends_correct_comp_type);
    RUN_TEST(start_fmt_appends_correct_name_and_value);
    RUN_TEST(start_fmt_merges_fmtbound_with_previous_fmtbound);
    RUN_TEST(end_fmt_appends_correct_name_and_value);
    RUN_TEST(end_fmt_merges_fmtbound_with_previous_fmtbound);
    RUN_TEST(iter_next_on_empty_op);
    RUN_TEST(iter_next_iterates_once_over_skip_with_count_one);
    RUN_TEST(iter_next_iterates_skip_with_count_greater_than_one);
    RUN_TEST(iter_next_iterates_over_single_insert_component);
    RUN_TEST(equal_returns_true_for_two_equal_inserts);
    RUN_TEST(equal_returns_true_for_two_equal_skips);
    RUN_TEST(equal_returns_false_for_skips_with_different_counts);
    RUN_TEST(equal_returns_true_for_two_empty_operations);
    RUN_TEST(equal_returns_false_for_ops_with_different_clients);
    RUN_TEST(equal_returns_true_for_two_operations_with_same_parent);
    RUN_TEST(equal_returns_true_for_operations_with_insert_and_skip);
    RUN_TEST(equal_returns_false_for_ops_with_different_lengths);
    RUN_TEST(dup_duplicates_op_with_one_component);
    RUN_TEST(size_with_one_insert);
    RUN_TEST(size_with_empty_op);
    RUN_TEST(size_of_op_with_only_inserts_equals_length_of_snapshot);
    RUN_TEST(size_with_delete);
    RUN_TEST(size_with_delete_and_insert);

    return (results) { passed, failed };
}
