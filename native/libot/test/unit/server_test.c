#include "../../server.h"
#include "unit.h"

static char* sent_msg = NULL;
static ot_event_type event_type = 0;
static ot_op* event_op = NULL;

static int send(const char* msg) {
    if (sent_msg != NULL) {
        free(sent_msg);
    }

    size_t size = strlen(msg) + 1;
    sent_msg = malloc(sizeof(char) * size);
    memcpy(sent_msg, msg, size);

    return 0;
}

static int event(ot_event_type t, ot_op* op) {
    event_type = t;

    if (event_op != NULL) {
        ot_free_op(event_op);
    }
    event_op = ot_dup_op(op);

    return 0;
}

static bool server_receive_fires_event_when_parent_cannot_be_found(char** msg) {
    ot_op* initial_op = ot_new_op();
    ot_insert(initial_op, "abc");

    ot_doc* doc = ot_new_doc();
    ot_doc_append(doc, &initial_op);

    ot_server* server = ot_new_server(send, event);
    ot_server_open(server, doc);

    const int NONEMPTY_HASH = 0xFF;
    ot_op* invalid_op = ot_new_op();
    ot_insert(invalid_op, "abc");
    memset(invalid_op->parent, NONEMPTY_HASH, 20);

    char* invalid_op_enc = ot_encode(invalid_op);
    ot_server_receive(server, invalid_op_enc);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, sent_msg);
    ASSERT_INT_EQUAL(OT_ERR_XFORM_FAILED, err, "Sent error was incorrect.",
                     msg);

    ot_free_op(dec);
    ot_free_op(invalid_op);
    ot_free_server(server);
    free(invalid_op_enc);
    return true;
}

static bool server_receive_fires_event_when_append_error_occurs(char** msg) {
    ot_op* initial_op = ot_new_op();
    ot_insert(initial_op, "abc");

    ot_doc* doc = ot_new_doc();
    ot_doc_append(doc, &initial_op);

    ot_server* server = ot_new_server(send, event);
    ot_server_open(server, doc);

    ot_op* invalid_op = ot_new_op();
    ot_insert(invalid_op, "abc");
    memcpy(invalid_op->parent, initial_op->hash, 20);

    char* invalid_op_enc = ot_encode(invalid_op);
    ot_server_receive(server, invalid_op_enc);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, sent_msg);
    ASSERT_INT_EQUAL(OT_ERR_APPEND_FAILED, err, "Sent error was incorrect.",
                     msg);

    ot_free_op(dec);
    ot_free_op(invalid_op);
    ot_free_server(server);
    free(invalid_op_enc);
    return true;
}

static bool server_receive_fires_event_when_xform_error_occurs(char** msg) {
    ot_op* initial_op = ot_new_op();
    ot_insert(initial_op, "abc");

    ot_doc* doc = ot_new_doc();
    ot_doc_append(doc, &initial_op);

    ot_server* server = ot_new_server(send, event);
    ot_server_open(server, doc);

    ot_op* invalid_op = ot_new_op();
    ot_skip(invalid_op, 1);

    char* invalid_op_enc = ot_encode(invalid_op);
    ot_server_receive(server, invalid_op_enc);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, sent_msg);
    ASSERT_INT_EQUAL(OT_ERR_XFORM_FAILED, err, "Sent error was incorrect.",
                     msg);

    ot_free_op(dec);
    ot_free_op(invalid_op);
    ot_free_server(server);
    free(invalid_op_enc);
    return true;
}

static bool server_receive_fires_event_when_a_decode_error_occurs(char** msg) {
    ot_server* server = ot_new_server(send, event);
    char* invalid_msg = "not json";
    ot_server_receive(server, invalid_msg);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, sent_msg);
    ASSERT_INT_EQUAL(OT_ERR_INVALID_JSON, err, "Sent error was incorrect.",
                     msg);

    ot_free_op(dec);
    ot_free_server(server);
    return true;
}

static bool server_receive_when_op_has_parent_and_doc_is_empty(char** msg) {
    ot_server* server = ot_new_server(send, event);
    ot_doc* doc = ot_new_doc();
    ot_server_open(server, doc);

    const int NONEMPTY_HASH = 0xFF;
    ot_op* invalid_op = ot_new_op();
    ot_insert(invalid_op, "abc");
    memset(invalid_op->parent, NONEMPTY_HASH, 20);

    char* invalid_op_enc = ot_encode(invalid_op);
    ot_server_receive(server, invalid_op_enc);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, sent_msg);
    ASSERT_INT_EQUAL(OT_ERR_XFORM_FAILED, err, "Sent error was incorrect.",
                     msg);

    ot_free_op(dec);
    ot_free_op(invalid_op);
    ot_free_server(server);
    free(invalid_op_enc);
    return true;
}

static bool server_receive_when_empty_doc_is_opened(char** msg) {
    ot_server* server = ot_new_server(send, event);
    ot_doc* doc = ot_new_doc();
    ot_server_open(server, doc);

    ot_op* op = ot_new_op();
    ot_insert(op, "abc");

    char* op_enc = ot_encode(op);
    ot_server_receive(server, op_enc);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, sent_msg);
    ASSERT_INT_EQUAL(OT_ERR_NONE, err, "Unexpected sent error.", msg);
    ASSERT_OP_EQUAL(op, server->doc->composed, "Document state was incorrect.",
                    msg);

    ot_free_op(dec);
    ot_free_op(op);
    ot_free_server(server);
    free(op_enc);
    return true;
}

results server_tests() {
    RUN_TEST(server_receive_fires_event_when_parent_cannot_be_found);
    RUN_TEST(server_receive_fires_event_when_append_error_occurs);
    RUN_TEST(server_receive_fires_event_when_xform_error_occurs);
    RUN_TEST(server_receive_fires_event_when_a_decode_error_occurs);
    RUN_TEST(server_receive_when_op_has_parent_and_doc_is_empty);
    RUN_TEST(server_receive_when_empty_doc_is_opened);

    return (results) { passed, failed };
}
