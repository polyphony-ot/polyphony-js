#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "minunit.h"
#include "../ot.h"
#include "../compose.h"
#include "../xform.h"
#include "../array.h"
#include "../hex.h"
#include "../decode.h"
#include "../encode.h"
#include "../sha1.h"
#include "../client.h"

/* otencode tests */

MU_TEST(test_serialize_empty_op) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[]}";
    ot_op* op = ot_new_op();

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    char msg[1024];
    sprintf(msg, "Serializing empty op did not create expected string. Expected = \"%s\". Actual = \"%s\".", EXPECTED, actual);
    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, msg);
}

MU_TEST(test_serialize_single_insert) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[{\"type\":\"insert\",\"text\":\"any string\"}]}";
    ot_op* op = ot_new_op();
    ot_insert(op, "any string");

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    char msg[1024];
    sprintf(msg, "Serializing a single insert did not create expected string. Expected = \"%s\". Actual = \"%s\".", EXPECTED, actual);
    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, msg);
}

MU_TEST(test_serialize_two_inserts) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[{\"type\":\"insert\",\"text\":\"any string any other string\"}]}";
    ot_op* op = ot_new_op();
    ot_insert(op, "any string");
    ot_insert(op, " any other string");

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing two inserts did not create expected string.");
}

MU_TEST(test_serialize_single_skip) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[{\"type\":\"skip\",\"count\":1}]}";
    ot_op* op = ot_new_op();
    ot_skip(op, 1);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single skip did not create expected string.");
}

MU_TEST(test_serialize_single_delete) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[{\"type\":\"delete\",\"count\":1}]}";
    ot_op* op = ot_new_op();
    ot_delete(op, 1);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single delete did not create expected string.");
}

MU_TEST(test_serialize_single_open_element) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[{\"type\":\"openElement\",\"element\":\"any string\"}]}";
    ot_op* op = ot_new_op();
    ot_open_element(op, "any string");

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single openElement did not create expected string.");
}

MU_TEST(test_serialize_single_close_element) {
    const char* const EXPECTED = "{\"clientId\":0,\"parent\":\"00\",\"hash\":\"00\",\"components\":[{\"type\":\"closeElement\"}]}";
    ot_op* op = ot_new_op();
    ot_close_element(op);

    char* actual = ot_encode(op);
    int cmp = strcmp(EXPECTED, (char*)actual);

    free(actual);
    ot_free_op(op);

    mu_assert(cmp == 0, "Serializing a single closeElement did not create expected string.");
}

MU_TEST(encode_empty_doc) {
    const char* const EXPECTED = "[]";
    ot_doc* doc = ot_new_doc();
    char* actual = ot_encode_doc(doc);

    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the encoded document to be \"%s\", but instead got \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_doc(doc);
    free(actual);
}

MU_TEST(encode_doc_with_multiple_ops) {
    const char* const EXPECTED = "[{\"clientId\":0,\"parent\":\"00\",\"hash\":\"a9993e364706816aba3e25717850c26c9cd0d89d\",\"components\":[{\"type\":\"insert\",\"text\":\"abc\"}]},"
                                 "{\"clientId\":0,\"parent\":\"a9993e364706816aba3e25717850c26c9cd0d89d\",\"hash\":\"1f8ac10f23c5b5bc1167bda84b833e5c057a77d2\",\"components\":[{\"type\":\"skip\",\"count\":3},{\"type\":\"insert\",\"text\":\"def\"}]},"
                                 "{\"clientId\":0,\"parent\":\"1f8ac10f23c5b5bc1167bda84b833e5c057a77d2\",\"hash\":\"a9993e364706816aba3e25717850c26c9cd0d89d\",\"components\":[{\"type\":\"skip\",\"count\":3},{\"type\":\"delete\",\"count\":3}]}]";
    ot_doc* doc = ot_new_doc();

    ot_op* op1 = ot_new_op();
    ot_insert(op1, "abc");
    ot_doc_append(doc, &op1);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, 3);
    ot_insert(op2, "def");
    ot_doc_append(doc, &op2);

    ot_op* op3 = ot_new_op();
    ot_skip(op3, 3);
    ot_delete(op3, 3);
    ot_doc_append(doc, &op3);

    char* actual = ot_encode_doc(doc);

    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the encoded document to be \"%s\", but instead got \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_doc(doc);
    free(actual);
}

MU_TEST_SUITE(otencode_test_suite) {
    MU_RUN_TEST(test_serialize_empty_op);
    MU_RUN_TEST(test_serialize_single_insert);
    MU_RUN_TEST(test_serialize_two_inserts);
    MU_RUN_TEST(test_serialize_single_skip);
    MU_RUN_TEST(test_serialize_single_delete);
    MU_RUN_TEST(test_serialize_single_open_element);
    MU_RUN_TEST(test_serialize_single_close_element);
    MU_RUN_TEST(encode_empty_doc);
    MU_RUN_TEST(encode_doc_with_multiple_ops);
}

/* hex tests */

MU_TEST(hextoa_does_not_write_to_array_when_hex_is_empty) {
    const char EXPECTED = 1;
    char a[] = { EXPECTED };

    hextoa(a, 1, "", 0);
    const char ACTUAL = a[0];

    mu_assert(EXPECTED == ACTUAL, "Converting an empty hex string to an array should not write anything to the array.");
}

MU_TEST(hextoa_decodes_single_byte) {
    const char* const EXPECTED = "a";

    char actual[1];
    hextoa(actual, 1, "61", 2);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    mu_assert(cmp == 0, "Decoding a single byte gave an incorrect result.");
}

MU_TEST(hextoa_decodes_multiple_bytes) {
    const char* const EXPECTED = "abc";

    char actual[3];
    hextoa(actual, 3, "616263", 6);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    mu_assert(cmp == 0, "Decoding multiple bytes gave an incorrect result.");
}

MU_TEST(hextoa_decodes_mixed_case_letters) {
    const char* const EXPECTED = "JZ";

    char actual[2];
    hextoa(actual, 2, "4A5a", 4);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    mu_assert(cmp == 0, "Decoding mixed-case letters gave an incorrect result.");
}

MU_TEST(hextoa_decodes_lowercase_letters) {
    const char* const EXPECTED = "JZ";

    char actual[2];
    hextoa(actual, 2, "4a5a", 4);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    mu_assert(cmp == 0, "Decoding lowercase letters gave an incorrect result.");
}

MU_TEST(hextoa_decodes_uppercase_letters) {
    const char* const EXPECTED = "JZ";

    char actual[2];
    hextoa(actual, 2, "4A5A", 4);
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    mu_assert(cmp == 0, "Decoding uppercase letters gave an incorrect result.");
}

MU_TEST(atohex_encodes_single_byte) {
    const char* const EXPECTED = "01";
    const char ARRAY[] = { 0x01 };

    char actual[3] = { 0 };
    atohex(actual, ARRAY, sizeof(ARRAY));
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    char msg[1024];
    sprintf(msg, "Encoding a single byte gave an incorrect result. Expected = \"%s\". Actual = \"%.2s\".", EXPECTED, actual);
    mu_assert(cmp == 0, msg);
}

MU_TEST(atohex_encodes_multiple_bytes) {
    const char* const EXPECTED = "01ff1a";
    const char ARRAY[] = { 0x01, 0xFF, 0x1A };

    char actual[7] = { 0 };
    atohex(actual, ARRAY, sizeof(ARRAY));
    int cmp = memcmp(EXPECTED, actual, sizeof(actual));

    char msg[1024];
    sprintf(msg, "Encoding multiple bytes gave an incorrect result. Expected = \"%s\". Actual = \"%.6s\".", EXPECTED, actual);
    mu_assert(cmp == 0, msg);
}

MU_TEST_SUITE(hex_test_suite) {
    MU_RUN_TEST(hextoa_does_not_write_to_array_when_hex_is_empty);
    MU_RUN_TEST(hextoa_decodes_single_byte);
    MU_RUN_TEST(hextoa_decodes_multiple_bytes);
    MU_RUN_TEST(hextoa_decodes_mixed_case_letters);
    MU_RUN_TEST(hextoa_decodes_lowercase_letters);
    MU_RUN_TEST(hextoa_decodes_uppercase_letters);
    MU_RUN_TEST(atohex_encodes_single_byte);
    MU_RUN_TEST(atohex_encodes_multiple_bytes);
}

/* sha1 tests */

typedef struct sha1_test {
    char* in;
    char* hash;
} sha1_test;

sha1_test sha1_tests[] = {
    (sha1_test) { "", (char[20]) { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 } },
    (sha1_test) { "hello world", (char[20]) { 0x2a, 0xae, 0x6c, 0x35, 0xc9, 0x4f, 0xcf, 0xb4, 0x15, 0xdb, 0xe9, 0x5f, 0x40, 0x8b, 0x9c, 0xe9, 0x1e, 0xe8, 0x46, 0xed } },
    (sha1_test) { "lorem ipsum dolor sit amet consetetur sadipscing elitr sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam", (char[20]) { 0x78, 0x34, 0x63, 0x8a, 0xa8, 0xeb, 0x18, 0xbb, 0x43, 0x95, 0x2d, 0x91, 0x9e, 0xfb, 0x49, 0x1e, 0xe4, 0x82, 0x56, 0x45 } }
};

MU_TEST(sha1) {
    size_t max = sizeof(sha1_tests) / sizeof(sha1_test);
    for (size_t i = 0; i < max; ++i) {
        sha1_test t = sha1_tests[i];
        hash_state hs;

        int result = sha1_init(&hs);
        mu_assert(result == CRYPT_OK, "An error occurred initializing the hash state.");

        result = sha1_process(&hs, t.in, (uint32_t)strlen(t.in));
        mu_assert(result == CRYPT_OK, "An error occurred processing the hash input.");

        char hash[20];
        result = sha1_done(&hs, hash);
        mu_assert(result == CRYPT_OK, "An error occurred getting the hash.");

        for (size_t j = 0; j < 20; ++j) {
            if (hash[j] != t.hash[j]) {
                mu_fail("Computed hash was incorrect.");
            }
        }
    }
}

MU_TEST_SUITE(sha1_test_suite) {
    MU_RUN_TEST(sha1);
}

/* doc tests */

MU_TEST(append_empty_op_does_not_segfault) {
    ot_doc* doc = ot_new_doc();
    ot_op* op = ot_new_op();

    ot_doc_append(doc, &op);

    ot_free_doc(doc);
}

MU_TEST(append_returns_error_when_max_size_is_reached) {
    ot_doc* doc = ot_new_doc();
    doc->max_size = 2;

    ot_op* op = ot_new_op();
    ot_insert(op, "abc");

    ot_err err = ot_doc_append(doc, &op);
    mu_assert(err == OT_ERR_MAX_SIZE, "Expected a OT_ERR_MAX_SIZE error.");

    ot_free_doc(doc);
}

MU_TEST_SUITE(doc_test_suite) {
    MU_RUN_TEST(append_empty_op_does_not_segfault);
    MU_RUN_TEST(append_returns_error_when_max_size_is_reached);
}

/* client tests */

char* sent_op = "";
int send_ret = 0;

static int send_stub(const char* op) {
    sent_op = strdup(op);
    return send_ret;
}

ot_event_type event_type = 0;
ot_op* event_op = NULL;
int event_ret = 0;

static int event_stub(ot_event_type t, ot_op* op) {
    event_type = (const ot_event_type)t;
    event_op = op;
    return event_ret;
}

MU_TEST(client_has_correct_state_and_event_when_receiving_op_for_empty_doc) {
    const uint32_t NONZERO = 1;
    const char* const EXPECTED = "any string";
    ot_client* client = ot_new_client(send_stub, event_stub);
    client->client_id = NONZERO;
    char* enc_op = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"any string\" } ] }";

    ot_client_receive(client, enc_op);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the document state to be \"%s\", but instead got \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    if (event_type != OT_OP_APPLIED) {
        char* msg;
        asprintf(&msg, "Expected an event of type OT_OP_APPLIED (%d), but instead got %d.", OT_OP_APPLIED, event_type);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
}

MU_TEST(client_has_correct_state_and_event_when_receiving_op_for_non_empty_doc) {
    const uint32_t NONZERO = 1;
    const char* const EXPECTED = "abcdef";
    const char* const ENC_OP1 = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"insert\", \"text\": \"abc\" } ] }";
    const char* const ENC_OP2 = "{ \"clientId\": 0, \"parent\": \"00\", \"hash\": \"00\", \"components\": [ { \"type\": \"skip\", \"count\": 3 }, { \"type\": \"insert\", \"text\": \"def\" } ] }";
    ot_client* client = ot_new_client(send_stub, event_stub);
    client->client_id = NONZERO;

    ot_client_receive(client, ENC_OP1);
    ot_client_receive(client, ENC_OP2);

    char* actual = ot_snapshot(client->doc->composed);
    const int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the document state to be \"%s\", but instead got \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    if (event_type != OT_OP_APPLIED) {
        char* msg;
        asprintf(&msg, "Expected an event of type OT_OP_APPLIED (%d), but instead got %d.", OT_OP_APPLIED, event_type);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
}

MU_TEST(client_receive_does_not_send_empty_buffer_after_acknowledgement) {
    ot_client* client = ot_new_client(send_stub, event_stub);
    char* op = "{ \"clientId\": 0, \"parent\": \"0\", \"components\": [ ] }";
    char* nothing = "NOTHING";
    sent_op = nothing;

    ot_client_receive(client, op);

    if (sent_op != nothing) {
        char* msg;
        asprintf(&msg, "Expected the client to not send anything, but it sent \"%s\".", sent_op);
        mu_fail(msg);
    }

    ot_free_client(client);
}

MU_TEST(client_apply_sends_op_if_not_waiting_for_acknowledgement) {
    ot_client* client = ot_new_client(send_stub, event_stub);
    ot_op* op = ot_new_op();
    ot_insert(op, "any string");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    ot_op* dec_sent_op = ot_new_op();
    ot_err derr = ot_decode(dec_sent_op, sent_op);

    mu_assert_int_eq(OT_ERR_NONE, derr);
    mu_assert(ot_equal(op, dec_sent_op), "Sent op wasn't equal to the applied op.");

    ot_free_op(dec_sent_op);
    ot_free_client(client);
    free(sent_op);
}

MU_TEST(client_receives_new_op_before_acknowledgement_starting_with_empty_doc) {
    const char* const EXPECTED = "server text client text";
    ot_client* client = ot_new_client(send_stub, event_stub);
    ot_op* op = ot_new_op();
    ot_insert(op, "client text");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    char* enc_serv_op = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"2a19fa462b801abce17bf5bb6c13f370c268d9b2\", \"components\": [ { \"type\": \"insert\", \"text\": \"server text \" } ] }";
    ot_client_receive(client, enc_serv_op);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the client's document to be \"%s\", but instead it's \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
    free(sent_op);
}

MU_TEST(client_receives_multiple_ops_before_acknowledgement_starting_with_empty_doc) {
    const char* const EXPECTED = "server text more server text client text";
    ot_client* client = ot_new_client(send_stub, event_stub);
    ot_op* op = ot_new_op();
    ot_insert(op, "client text");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    char* enc_serv_op = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"d82ac619d64a0883de5276f0f3e9a984c3e22620\", \"components\": [ { \"type\": \"insert\", \"text\": \"server text\" } ] }";
    char* enc_serv_op2 = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"66a02881b5b0d0d4e2e40b2bfa3d6e3ca710c85c\", \"components\": [ { \"type\": \"skip\", \"count\": 11 }, { \"type\": \"insert\", \"text\": \" more server text \" } ] }";
    ot_client_receive(client, enc_serv_op);
    ot_client_receive(client, enc_serv_op2);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the client's document to be \"%s\", but instead it's \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
    free(sent_op);
}

MU_TEST(client_receives_new_op_before_acknowledgement_and_then_applies_local_op) {
    const char* const EXPECTED = "server text client text more client text";
    ot_client* client = ot_new_client(send_stub, event_stub);
    ot_op* op = ot_new_op();
    ot_insert(op, "client text ");

    ot_err cerr = ot_client_apply(client, &op);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    char* enc_serv_op = "{ \"clientId\": 1, \"parent\": \"00\", \"hash\": \"d82ac619d64a0883de5276f0f3e9a984c3e22620\", \"components\": [ { \"type\": \"insert\", \"text\": \"server text \" } ] }";
    ot_client_receive(client, enc_serv_op);

    ot_op* op2 = ot_new_op();
    ot_skip(op2, 24);
    ot_insert(op2, "more client text");

    cerr = ot_client_apply(client, &op2);
    mu_assert_int_eq(OT_ERR_NONE, cerr);

    char* actual = ot_snapshot(client->doc->composed);
    int cmp = strcmp(EXPECTED, actual);
    if (cmp != 0) {
        char* msg;
        asprintf(&msg, "Expected the client's document to be \"%s\", but instead it's \"%s\".", EXPECTED, actual);
        mu_fail(msg);
    }

    ot_free_client(client);
    free(actual);
    free(sent_op);
}

MU_TEST_SUITE(client_test_suite) {
    MU_RUN_TEST(client_has_correct_state_and_event_when_receiving_op_for_empty_doc);
    MU_RUN_TEST(client_has_correct_state_and_event_when_receiving_op_for_non_empty_doc);
    MU_RUN_TEST(client_receive_does_not_send_empty_buffer_after_acknowledgement);
    MU_RUN_TEST(client_apply_sends_op_if_not_waiting_for_acknowledgement);
    MU_RUN_TEST(client_receives_new_op_before_acknowledgement_starting_with_empty_doc);
    MU_RUN_TEST(client_receives_multiple_ops_before_acknowledgement_starting_with_empty_doc);
    MU_RUN_TEST(client_receives_new_op_before_acknowledgement_and_then_applies_local_op);
}

int main() {
    // Close stderr to avoid littering test output with log messages. This line
    // can be removed to aid in debugging tests.
    fclose(stderr);

    MU_RUN_SUITE(ot_test_suite);
    MU_RUN_SUITE(compose_test_suite);
    MU_RUN_SUITE(xform_test_suite);
    MU_RUN_SUITE(otdecode_test_suite);
    MU_RUN_SUITE(otencode_test_suite);
    MU_RUN_SUITE(array_test_suite);
    MU_RUN_SUITE(hex_test_suite);
    MU_RUN_SUITE(sha1_test_suite);
    MU_RUN_SUITE(client_test_suite);
    MU_RUN_SUITE(doc_test_suite);
    MU_REPORT();
    return 0;
}
