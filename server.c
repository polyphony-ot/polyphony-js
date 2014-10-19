#include "server.h"

static void send(ot_server* server, const char* json) {
    server->send(json);
    fprintf(stderr, "[INFO] Sent message.\n\tJSON: %s\n", json);
}

static void send_err(ot_server* server, ot_err err) {
    char* enc = ot_encode_err(err);
    send(server, enc);
    free(enc);
}

static bool can_append(const ot_doc* doc, const ot_op* op) {
    const char* parent = op->parent;

    if (doc->history.len == 0) {
        for (int i = 0; i < 20; ++i) {
            if (parent[i] != 0) {
                return false;
            }
        }

        return true;
    }

    if (memcmp(doc->composed->hash, parent, sizeof(char) * 20) == 0) {
        return true;
    }

    return false;
}

static ot_err append_op(ot_server* server, ot_op* op) {
    ot_doc* doc = server->doc;

    if (doc->composed == NULL) {
        char* op_enc = ot_encode(op);
        fprintf(stderr, "[INFO] Appending operation to empty document.\n"
                        "\tOperation: %s\n",
                op_enc);
        free(op_enc);
    } else {
        char* doc_enc = ot_encode(doc->composed);
        char* op_enc = ot_encode(op);
        fprintf(stderr, "[INFO] Appending operation to document.\n"
                        "\tDocument: %s\n"
                        "\tOperation: %s\n",
                doc_enc, op_enc);
        free(doc_enc);
        free(op_enc);
    }

    ot_err err = ot_doc_append(doc, &op);
    if (err != OT_ERR_NONE) {
        char* doc_enc = ot_encode(doc->composed);
        char* op_enc = ot_encode(op);
        fprintf(stderr, "[ERROR %d] Appending operation to document failed.\n"
                        "\tDocument: %s\n"
                        "\tOperation: %s\n",
                err, doc_enc, op_enc);
        ot_free_op(op);
        free(doc_enc);
        free(op_enc);
        return err;
    }

    char* append_enc = ot_encode(op);
    send(server, append_enc);
    free(append_enc);
    return OT_ERR_NONE;
}

static ot_op* xform(const ot_doc* doc, ot_op* op) {
    char* op_enc = ot_encode(op);
    ot_op* composed = ot_doc_compose_after(doc, op->parent);
    if (composed == NULL) {
        fprintf(stderr, "[ERROR %d] Couldn't find the operation's parent.\n"
                        "\tOperation: %s\n",
                OT_ERR_COMPOSE_FAILED, op_enc);
        free(op_enc);
        return NULL;
    }

    char* composed_enc = ot_encode(composed);
    fprintf(stderr, "[INFO] Transforming received operation.\n"
                    "\tServer Operation: %s\n"
                    "\tClient Operation: %s\n",
            composed_enc, op_enc);

    ot_xform_pair p = ot_xform(composed, op);
    if (p.op1_prime == NULL) {
        fprintf(stderr, "[ERROR %d] Transformation failed.\n"
                        "\tServer Operation: %s\n"
                        "\tClient Operation: %s\n",
                OT_ERR_XFORM_FAILED, composed_enc, op_enc);
        free(composed_enc);
        free(op_enc);
        ot_free_op(composed);
        return NULL;
    }

    char* op1_prime_enc = ot_encode(p.op1_prime);
    char* op2_prime_enc = ot_encode(p.op2_prime);

    fprintf(stderr, "Transformation succeeded.\n"
                    "\tServer Operation': %s\n"
                    "\tClient Operation': %s\n",
            op1_prime_enc, op2_prime_enc);

    free(composed_enc);
    free(op_enc);
    free(op1_prime_enc);
    free(op2_prime_enc);
    ot_free_op(p.op1_prime);
    ot_free_op(composed);

    return p.op2_prime;
}

ot_server* ot_new_server(send_func send, ot_event_func event) {
    ot_server* server = malloc(sizeof(ot_server));
    server->send = send;
    server->event = event;
    server->doc = NULL;

    return server;
}

void ot_free_server(ot_server* server) {
    if (server->doc != NULL) {
        ot_free_doc(server->doc);
    }

    free(server);
}

void ot_server_open(ot_server* server, ot_doc* doc) { server->doc = doc; }

void ot_server_receive(ot_server* server, const char* op) {
    fprintf(stderr, "[INFO] Received message.\n\tJSON: %s\n", op);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, op);
    if (err != OT_ERR_NONE) {
        fprintf(stderr, "[ERROR %d] Couldn't decode the received operation.\n"
                        "\tJSON: %s\n",
                err, op);
        send_err(server, err);
        ot_free_op(dec);
        return;
    }

    ot_doc* doc = server->doc;
    err = OT_ERR_NONE;
    if (doc == NULL) {
        fputs("[INFO] Creating a new document.\n", stderr);
        server->doc = ot_new_doc();
        doc = server->doc;
        err = append_op(server, dec);
    } else if (can_append(doc, dec)) {
        err = append_op(server, dec);
    } else {
        ot_op* op_prime = xform(doc, dec);
        ot_free_op(dec);
        if (op_prime == NULL) {
            err = OT_ERR_XFORM_FAILED;
        } else {
            err = append_op(server, op_prime);
        }
    }

    char* doc_enc;
    if (doc->composed == NULL) {
        doc_enc = "EMPTY";
    } else {
        doc_enc = ot_encode(doc->composed);
    }

    if (err == OT_ERR_NONE) {
        fprintf(stderr, "[INFO] Document updated.\n"
                        "\tDocument: %s\n",
                doc_enc);
    } else {
        send_err(server, err);
        fprintf(stderr, "[INFO] Document unchanged due to an error.\n"
                        "\tError: %d\n"
                        "\tDocument: %s\n",
                err, doc_enc);
    }

    if (doc->composed != NULL) {
        free(doc_enc);
    }
}
