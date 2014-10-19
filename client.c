#include "client.h"

static ot_err append_op(ot_client* client, ot_op** op) {
    ot_doc* doc = client->doc;
    char* op_enc = ot_encode(*op);
    char* doc_enc = NULL;
    if (doc->composed == NULL) {
        fprintf(stderr, "[INFO] Appending operation to empty document.\n"
                        "\tOperation: %s\n",
                op_enc);
    } else {
        doc_enc = ot_encode(doc->composed);
        fprintf(stderr, "[INFO] Appending operation to document.\n"
                        "\tDocument: %s\n"
                        "\tOperation: %s\n",
                doc_enc, op_enc);
    }

    ot_err err = ot_doc_append(doc, op);
    if (err != OT_ERR_NONE) {
        if (doc->composed == NULL) {
            fprintf(stderr, "[ERROR %d] Appending operation to empty document "
                            "failed.\n"
                            "\tOperation: %s\n",
                    err, op_enc);
        } else {
            fprintf(stderr, "[ERROR %d] Appending operation to document failed."
                            "\n"
                            "\tDocument: %s\n"
                            "\tOperation: %s\n",
                    err, doc_enc, op_enc);
            free(doc_enc);
        }

        ot_free_op(*op);
        free(op_enc);
        return err;
    }

    if (doc_enc != NULL) {
        free(doc_enc);
    }
    free(op_enc);

    return OT_ERR_NONE;
}

static ot_err buffer_op(ot_client* client, ot_op* op) {
    if (client->buffer == NULL) {
        client->buffer = ot_dup_op(op);
        return OT_ERR_NONE;
    }

    char* buffer_enc = ot_encode(client->buffer);
    char* op_enc = ot_encode(op);
    fprintf(stderr, "[INFO] Composing buffer with applied operation.\n"
                    "\tBuffer: %s\n"
                    "\tApplied Operation: %s\n",
            buffer_enc, op_enc);

    ot_op* composed = ot_compose(client->buffer, op);
    if (composed == NULL) {
        fprintf(stderr, "[ERROR %d] Composition of buffer with applied "
                        "operation failed.\n"
                        "\tBuffer: %s\n"
                        "\tApplied Operation: %s\n",
                OT_ERR_COMPOSE_FAILED, buffer_enc, op_enc);
        free(buffer_enc);
        free(op_enc);
        return OT_ERR_BUFFER_FAILED;
    }
    free(buffer_enc);
    free(op_enc);

    char* composed_enc = ot_encode(composed);
    fprintf(stderr, "[INFO] Composition of buffer with applied operation "
                    "succeeded.\n"
                    "\tComposed Buffer: %s\n",
            composed_enc);
    free(composed_enc);

    ot_free_op(client->buffer);
    client->buffer = NULL;
    client->buffer = composed;

    return 0;
}

static void send_buffer(ot_client* client, const char* received_hash) {
    if (client->buffer == NULL) {
        if (client->anticipated != NULL) {
            ot_free_op(client->anticipated);
            client->anticipated = NULL;
        }
        return;
    }

    if (received_hash != NULL) {
        memcpy(client->buffer->parent, received_hash, 20);
    }

    char* enc_buf = ot_encode(client->buffer);
    client->send(enc_buf);
    fprintf(stderr, "[INFO] Sent message.\n\tJSON: %s\n", enc_buf);
    free(enc_buf);

    if (client->anticipated != NULL) {
        ot_free_op(client->anticipated);
        client->anticipated = NULL;
    }
    client->anticipated = ot_dup_op(client->buffer);

    ot_free_op(client->buffer);
    client->buffer = NULL;
    client->ack_required = true;
}

static void fire_op_event(ot_client* client, ot_event_type type, ot_op* op) {
    client->event(type, op);
}

// xform_anticipated calculates a new anticipated op by transforming the current
// anticipated op against an incoming op. It outputs an intermediate operation,
// inter, which can be transformed against the current buffer. This function
// will free the received op, provided that no error is returned. The outputted
// intermediate op must be freed by the caller.
static ot_err xform_anticipated(ot_client* client, ot_op* received,
                                ot_op** inter) {

    // We aren't anticipating any acknowledgment, so the buffer can be directly
    // transformed against the received op.
    if (client->anticipated == NULL) {
        *inter = received;
        return OT_ERR_NONE;
    }

    char* received_enc = ot_encode(received);
    char* anticipated_enc = ot_encode(client->anticipated);
    fprintf(stderr, "[INFO] Transforming received operation against anticipated"
                    " operation.\n"
                    "\tReceived Operation: %s\n"
                    "\tAnticipated Operation: %s\n",
            received_enc, anticipated_enc);
    ot_xform_pair p = ot_xform(received, client->anticipated);
    if (p.op1_prime == NULL || p.op2_prime == NULL) {
        fprintf(stderr, "[ERROR %d] Transformation of received operation "
                        "against anticipated operation failed.\n"
                        "\tReceived Operation: %s\n"
                        "\tAnticipated Operation: %s\n",
                OT_ERR_XFORM_FAILED, received_enc, anticipated_enc);

        free(received_enc);
        free(anticipated_enc);
        return OT_ERR_XFORM_FAILED;
    }
    free(received_enc);
    free(anticipated_enc);

    char* op1_prime_enc = ot_encode(p.op1_prime);
    char* op2_prime_enc = ot_encode(p.op2_prime);
    fprintf(stderr, "[INFO] Transformation of received operation against "
                    "anticipated operation succeeded.\n"
                    "\tReceived Operation': %s\n"
                    "\tAnticipated Operation': %s\n",
            op1_prime_enc, op2_prime_enc);
    free(op1_prime_enc);
    free(op2_prime_enc);

    *inter = p.op1_prime;

    ot_free_op(client->anticipated);
    client->anticipated = NULL;
    client->anticipated = p.op2_prime;

    ot_free_op(received);
    return OT_ERR_NONE;
}

static ot_err xform_buffer(ot_client* client, ot_op* inter, ot_op** apply) {
    // We haven't buffered any new ops, so the intermediate op can be directly
    // applied to the document.
    if (client->buffer == NULL) {
        *apply = inter;
        return OT_ERR_NONE;
    }

    char* buffer_enc = ot_encode(client->buffer);
    char* inter_enc = ot_encode(inter);
    fprintf(stderr, "[INFO] Transforming buffer against intermediate operation."
                    "\n"
                    "\tBuffer: %s\n"
                    "\tIntermediate Operation: %s\n",
            buffer_enc, inter_enc);

    ot_xform_pair p = ot_xform(client->buffer, inter);
    if (p.op1_prime == NULL || p.op2_prime == NULL) {
        fprintf(stderr, "[ERROR %d] Transformation of buffer against "
                        "intermediate operation failed.\n"
                        "\tBuffer: %s\n"
                        "\tIntermediate Operation: %s\n",
                OT_ERR_XFORM_FAILED, buffer_enc, inter_enc);
        free(buffer_enc);
        free(inter_enc);
        return OT_ERR_XFORM_FAILED;
    }
    free(buffer_enc);
    free(inter_enc);

    char* op1_prime_enc = ot_encode(p.op1_prime);
    char* op2_prime_enc = ot_encode(p.op2_prime);
    fprintf(stderr, "[INFO] Transformation of buffer against intermediate "
                    "operation succeeded.\n"
                    "\tBuffer': %s\n"
                    "\tIntermediate Operation': %s\n",
            op1_prime_enc, op2_prime_enc);
    free(op1_prime_enc);
    free(op2_prime_enc);

    *apply = p.op2_prime;
    ot_free_op(client->buffer);
    client->buffer = NULL;
    ot_free_op(inter);

    client->buffer = p.op1_prime;

    return OT_ERR_NONE;
}

ot_client* ot_new_client(send_func send, ot_event_func event) {
    ot_client* client = malloc(sizeof(ot_client));
    client->buffer = NULL;
    client->anticipated = NULL;
    client->send = send;
    client->event = event;
    client->doc = NULL;
    client->client_id = 0;
    client->ack_required = false;

    return client;
}

void ot_free_client(ot_client* client) {
    ot_doc* doc = client->doc;
    if (doc != NULL) {
        ot_free_doc(client->doc);
    }
    if (client->anticipated != NULL) {
        ot_free_op(client->anticipated);
    }
    if (client->buffer != NULL) {
        ot_free_op(client->buffer);
    }
    free(client);
}

void ot_client_open(ot_client* client, ot_doc* doc) { client->doc = doc; }

void ot_client_receive(ot_client* client, const char* op) {
    fprintf(stderr, "[INFO] Received message.\n\tJSON: %s\n", op);

    ot_op* dec = ot_new_op();
    ot_err err = ot_decode(dec, op);
    if (err != OT_ERR_NONE) {
        fprintf(stderr, "[ERROR %d] The decoded operation returned an error.\n"
                        "\tJSON: %s\n",
                err, op);
        ot_free_op(dec);
        fire_op_event(client, OT_ERROR, NULL);
        return;
    }

    if (dec->client_id == client->client_id) {
        char hex[41] = { 0 };
        atohex(hex, dec->hash, 20);
        fprintf(stderr, "[INFO] Operation was acknowledged.\n"
                        "\tHash: %s\n",
                hex);

        client->ack_required = false;
        send_buffer(client, dec->hash);

        ot_free_op(dec);
        return;
    }

    fire_op_event(client, OT_OP_INCOMING, NULL);

    ot_op* inter;
    err = xform_anticipated(client, dec, &inter);
    if (err != OT_ERR_NONE) {
        ot_free_op(dec);
        fire_op_event(client, OT_ERROR, NULL);
        return;
    }

    ot_op* apply;
    err = xform_buffer(client, inter, &apply);
    if (err != OT_ERR_NONE) {
        ot_free_op(inter);
        fire_op_event(client, OT_ERROR, NULL);
        return;
    }

    if (client->doc == NULL) {
        fputs("[INFO] Creating a new document.\n", stderr);
        client->doc = ot_new_doc();
    }
    append_op(client, &apply);
    fire_op_event(client, OT_OP_APPLIED, apply);
}

ot_err ot_client_apply(ot_client* client, ot_op** op) {
    char* op_enc = ot_encode(*op);
    fprintf(stderr, "[INFO] Editor applying operation.\n"
                    "\tOperation: %s\n",
            op_enc);
    free(op_enc);

    (*op)->client_id = client->client_id;

    if (client->doc == NULL) {
        fputs("[INFO] Creating a new document.\n", stderr);
        client->doc = ot_new_doc();
    }

    ot_err append_err = append_op(client, op);
    if (append_err != OT_ERR_NONE) {
        return append_err;
    }

    ot_err buf_err = buffer_op(client, *op);
    if (buf_err != OT_ERR_NONE) {
        return buf_err;
    }

    if (!client->ack_required) {
        send_buffer(client, NULL);
    }

    return OT_ERR_NONE;
}
