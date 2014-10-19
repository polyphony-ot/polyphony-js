#include "../../decode.h"
#include "../../doc.h"
#include "../../encode.h"
#include "unit.h"

static bool param_decode_test(const char* const encoded, ot_op* expected,
                              char** msg) {
    ot_op* actual = ot_new_op();
    ot_err err = ot_decode(actual, encoded);
    ASSERT_INT_EQUAL(OT_ERR_NONE, err, "Decoding the JSON returned an error.",
                     msg);

    bool equal = ot_equal(expected, actual);
    char* actual_enc = ot_encode(actual);
    ASSERT_CONDITION(equal, encoded, actual_enc,
                     "Decoded operation wasn't correct.", msg);

    ot_free_op(expected);
    ot_free_op(actual);
    free(actual_enc);
    return true;
}

static bool decode_returns_op_with_correct_skip_component(char** msg) {
    const char* const ENCODED_JSON = "{\"clientId\":0,\"parent\":\"00\","
                                     "\"hash\":\"00\",\"components\":[{"
                                     "\"type\":\"skip\",\"count\":1}]}";

    ot_op* expected = ot_new_op();
    ot_skip(expected, 1);

    return param_decode_test(ENCODED_JSON, expected, msg);
}

static bool decode_returns_op_with_correct_client_id(char** msg) {
    const char* const ENCODED_JSON =
        "{\"clientId\":1,\"parent\":\"00\",\"hash\":\"00\",\"components\":[]}";

    ot_op* expected = ot_new_op();
    expected->client_id = 1;

    return param_decode_test(ENCODED_JSON, expected, msg);
}

static bool decode_returns_op_with_correct_parent(char** msg) {
    const char* const PARENT = "abcdefghijklmnopqrst";
    const char* const ENCODED_JSON = "{\"clientId\":0,\"parent\":"
                                     "\"6162636465666768696a6b6c6d6e6f707172737"
                                     "4\",\"hash\":\"00\",\"components\":[]}";

    ot_op* expected = ot_new_op();
    memcpy(expected->parent, PARENT, 20);

    return param_decode_test(ENCODED_JSON, expected, msg);
}

static bool decode_fails_if_client_id_is_missing(char** msg) {
    const char* const ENCODED_JSON = "{\"parent\":"
                                     "\"6162636465666768696a6b6c6d6e6f70717273"
                                     "74\",\"components\":[]}";

    ot_op* actual = ot_new_op();
    ot_err err = ot_decode(actual, ENCODED_JSON);
    ASSERT_INT_EQUAL(
        OT_ERR_CLIENT_ID_MISSING, err,
        "Decode didn't return the correct error for a missing clientId field.",
        msg);

    ot_free_op(actual);
    return true;
}

static bool decode_fails_if_parent_field_is_missing(char** msg) {
    const char* const ENCODED_JSON = "{\"clientId\":1234,\"components\":[]}";

    ot_op* actual = ot_new_op();
    ot_err err = ot_decode(actual, ENCODED_JSON);
    ASSERT_INT_EQUAL(
        OT_ERR_PARENT_MISSING, err,
        "Decode didn't return the correct error for a missing parent field.",
        msg);

    ot_free_op(actual);
    return true;
}

static bool decode_fails_if_components_field_is_missing(char** msg) {
    const char* const ENCODED_JSON =
        "{\"clientId\":1234,\"parent\":\"0\",\"hash\":\"00\"}";

    ot_op* actual = ot_new_op();
    ot_err err = ot_decode(actual, ENCODED_JSON);
    ASSERT_INT_EQUAL(OT_ERR_COMPONENTS_MISSING, err,
                     "Decode didn't return the correct error for a missing "
                     "components field.",
                     msg);

    ot_free_op(actual);
    return true;
}

static bool decode_empty_doc_returns_doc_with_no_components(char** msg) {
    const char* const ENCODED_JSON = "[]";
    const int EXPECTED_LENGTH = 0;
    const ot_op* EXPECTED_COMPOSED_OP = NULL;

    ot_doc* actual_doc = ot_new_doc();
    ot_err err = ot_decode_doc(actual_doc, ENCODED_JSON);

    ASSERT_INT_EQUAL(OT_ERR_NONE, err,
                     "Decoding the document returned an error.", msg);

    size_t actual_length = actual_doc->history.len;
    ASSERT_INT_EQUAL(EXPECTED_LENGTH, actual_length,
                     "The document's length wasn't 0.", msg);

    ot_op* actual_composed_op = actual_doc->composed;
    ASSERT_INT_EQUAL((int)EXPECTED_COMPOSED_OP, (int)actual_composed_op,
                     "The document's composed state wasn't NULL.", msg);

    ot_free_doc(actual_doc);
    return true;
}

static bool decode_doc_with_insert_skip_and_delete_components(char** msg) {
    const int EXPECTED_LENGTH = 3;
    const char* const ENCODED_JSON =
        "[{\"clientId\":0,\"parent\":\"00\",\"hash\":"
        "\"a9993e364706816aba3e25717850c26c9cd0d89d\",\"components\":[{"
        "\"type\":\"insert\",\"text\":\"abc\"}]},{\"clientId\":0,\"parent\":"
        "\"a9993e364706816aba3e25717850c26c9cd0d89d\",\"hash\":"
        "\"1f8ac10f23c5b5bc1167bda84b833e5c057a77d2\",\"components\":[{"
        "\"type\":\"skip\",\"count\":3},{\"type\":\"insert\",\"text\":\"def\"}]"
        "},{\"clientId\":0,\"parent\":"
        "\"1f8ac10f23c5b5bc1167bda84b833e5c057a77d2\",\"hash\":"
        "\"a9993e364706816aba3e25717850c26c9cd0d89d\",\"components\":[{"
        "\"type\":\"skip\",\"count\":3},{\"type\":\"delete\",\"count\":3}]}]";

    ot_op* expected_op1 = ot_new_op();
    ot_insert(expected_op1, "abc");
    hextoa(expected_op1->hash, 20, "a9993e364706816aba3e25717850c26c9cd0d89d",
           40);

    ot_op* expected_op2 = ot_new_op();
    ot_skip(expected_op2, 3);
    ot_insert(expected_op2, "def");
    hextoa(expected_op2->hash, 20, "1f8ac10f23c5b5bc1167bda84b833e5c057a77d2",
           40);
    hextoa(expected_op2->parent, 20, "a9993e364706816aba3e25717850c26c9cd0d89d",
           40);

    ot_op* expected_op3 = ot_new_op();
    ot_skip(expected_op3, 3);
    ot_delete(expected_op3, 3);
    hextoa(expected_op3->hash, 20, "a9993e364706816aba3e25717850c26c9cd0d89d",
           40);
    hextoa(expected_op3->parent, 20, "1f8ac10f23c5b5bc1167bda84b833e5c057a77d2",
           40);

    ot_doc* actual_doc = ot_new_doc();
    ot_err err = ot_decode_doc(actual_doc, ENCODED_JSON);

    ASSERT_INT_EQUAL(OT_ERR_NONE, err,
                     "Decoding the document returned an error.", msg);

    size_t actual_length = actual_doc->history.len;
    ASSERT_INT_EQUAL(EXPECTED_LENGTH, actual_length,
                     "The document's length wasn't 0.", msg);

    ot_op* history = (ot_op*)actual_doc->history.data;

    ot_op* actual_op1 = history;
    bool op1_equal = ot_equal(expected_op1, actual_op1);
    char* expected_op1_enc = ot_encode(expected_op1);
    char* actual_op1_enc = ot_encode(actual_op1);
    ASSERT_CONDITION(
        op1_equal, expected_op1_enc, actual_op1_enc,
        "The first decoded operation in the document wasn't correct.", msg);

    ot_op* actual_op2 = history + 1;
    bool op2_equal = ot_equal(expected_op2, actual_op2);
    char* expected_op2_enc = ot_encode(expected_op2);
    char* actual_op2_enc = ot_encode(actual_op2);
    ASSERT_CONDITION(
        op2_equal, expected_op2_enc, actual_op2_enc,
        "The second decoded operation in the document wasn't correct.", msg);

    ot_op* actual_op3 = history + 2;
    bool op3_equal = ot_equal(expected_op3, actual_op3);
    char* expected_op3_enc = ot_encode(expected_op3);
    char* actual_op3_enc = ot_encode(actual_op3);
    ASSERT_CONDITION(
        op3_equal, expected_op3_enc, actual_op3_enc,
        "The third decoded operation in the document wasn't correct.", msg);

    free(expected_op1_enc);
    free(expected_op2_enc);
    free(expected_op3_enc);
    free(actual_op1_enc);
    free(actual_op2_enc);
    free(actual_op3_enc);
    ot_free_op(expected_op1);
    ot_free_op(expected_op2);
    ot_free_op(expected_op3);
    ot_free_doc(actual_doc);
    return true;
}

static bool decode_returns_correct_error_code(char** msg) {
    const char* ENCODED_JSON = "{\"errorCode\":1}";
    ot_err err = ot_decode(NULL, ENCODED_JSON);

    ASSERT_INT_EQUAL(1, err, "The error code was incorrect.", msg);

    return true;
}

results decode_tests() {
    RUN_TEST(decode_returns_op_with_correct_skip_component);
    RUN_TEST(decode_returns_op_with_correct_client_id);
    RUN_TEST(decode_returns_op_with_correct_parent);
    RUN_TEST(decode_fails_if_client_id_is_missing);
    RUN_TEST(decode_fails_if_parent_field_is_missing);
    RUN_TEST(decode_fails_if_components_field_is_missing);
    RUN_TEST(decode_empty_doc_returns_doc_with_no_components);
    RUN_TEST(decode_doc_with_insert_skip_and_delete_components);
    RUN_TEST(decode_returns_correct_error_code);

    return (results) { passed, failed };
}
