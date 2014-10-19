#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ot.h"
#include "compose.h"
#include "decode.h"
#include "encode.h"

// seed is used to seed rand().
const int seed = 0;

// num_docs is the number of documents to generate.
const int num_docs = 128;

// max_ins is maximum number of characters that will be inserted with any insert
// component.
const int max_ins = 64;

// max_ops is the maximum number of operations that will be generated for a
// document.
const int max_ops = 64;

// min_docsize is the minimum number of characters the composed document can
// contain.
const int min_docsize = 8;

// max_docsize is the maximum number of characters the composed document can
// contain.
const int max_docsize = 1024;

// msg is used as scratch space for various log messages.
char msg[5242880];

// randstr generates a random string of alphanumeric characters and places them
// in str.
static void randstr(char* str, size_t len) {
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";

    for (size_t i = 0; i < len; ++i) {
        str[i] = alphanum[(size_t)rand() % (sizeof(alphanum) - 1)];
    }

    str[len] = 0;
}

// insert_substr inserts a substring into another string.
static char* insert_substr(char* str, const char* substr, size_t pos) {
    size_t str_len = strlen(str);
    size_t substr_len = strlen(substr);
    str = realloc(str, str_len + substr_len + 1);

    char* substr_start = str + pos;
    char* substr_end = str + pos + substr_len;

    // Make room for the substring by making a gap in the string.
    memmove(substr_end, substr_start, str_len - pos);
    // Copy the substring into the string.
    memcpy(substr_start, substr, substr_len);
    str[str_len + substr_len] = 0;

    return str;
}

// delete_substr deletes a substring starting at given position.
static char* delete_substr(char* str, size_t pos, size_t len) {
    size_t str_len = strlen(str);
    memmove(str + pos, str + pos + len, str_len - (pos + len) + 1);

    return str;
}

// assert_doc verifies that an expected document state is equal to an actual
// document state. It will exit with a non-zero exit code if the assertion
// fails.
static void assert_doc(const char* expected, const char* actual) {
    if (actual == NULL) {
        actual = "";
    }

    if (strcmp(expected, actual) == 0) {
        sprintf(msg, "[INFO] Document state verified. Expected = \"%s\". Actual"
                     " = \"%s\".",
                expected, actual);
        puts(msg);
        return;
    }

    sprintf(msg, "[FATAL] Unexpected document state. Expected = \"%s\". Actual "
                 "= \"%s\".",
            expected, actual);
    puts(msg);
    exit(1);
}

// gen_doc generates a new document string and a new document operation. It then
// generates a random number of components, modifying the document string as it
// goes. At the end, all of the generated operations are composed together and
// then applied to the original document operation. If the snapshot of the
// composed document operation isn't equal to the document string, then it exits
// with a non-zero exit code.
static void gen_doc(int docnum) {
    sprintf(msg, "[INFO] Generating document %d.", docnum + 1);
    puts(msg);

    // Generate an initial document, which is just a string. This document
    // string is used as the expected state.
    size_t init_len = rand() % max_ins + 1;
    char* docstr = malloc(sizeof(char) * init_len + 1);
    randstr(docstr, init_len);

    // Create the initial operation which is just an insert of the document
    // string.
    char parent[20] = { 0 };
    ot_op* docop = ot_new_op();
    ot_insert(docop, docstr);
    char* snapshot = ot_snapshot(docop);
    assert_doc(docstr, snapshot);
    free(snapshot);

    ot_op* composed = NULL;
    size_t num_skips = 0;
    size_t num_inserts = 0;
    size_t num_deletes = 0;
    size_t max_reached = 0;
    size_t num_ops = rand() % max_ops + 2;
    for (size_t i = 0; i < num_ops; ++i) {
        ot_op* op = ot_new_op();

        sprintf(msg, "[INFO] Generating operation %zu out of %zu.", i + 1,
                num_ops);
        puts(msg);

        // doclen is the length of the current (composed) document.
        size_t doclen = strlen(docstr);

        // pos is our current position in current (composed) document.
        size_t pos = 0;

        // newpos is our current position in the new (edited) document.
        size_t newpos = 0;

        // newlen is the length of the new (edited) document.
        size_t newlen = 0;

        // Generate components for the current operation until we've spanned the
        // entire document. An operation must span the entire document in order
        // for it to be valid.
        while (pos < doclen || newlen < min_docsize) {
            int type = rand() % 3;
            newlen = doclen + (newpos - pos);
            if (newlen > max_reached) {
                max_reached = newlen;
            }

            if (type == OT_SKIP && pos < doclen) {
                // We can only perform a skip if we haven't already spanned the
                // entire document.

                uint32_t count = (uint32_t)((uint)rand() % (doclen - pos)) + 1;
                ot_skip(op, count);

                sprintf(msg, "[INFO] Skipping %lld characters at position %zu.",
                        count, newpos);
                puts(msg);

                pos += (size_t)count;
                newpos += (size_t)count;
                num_skips += (size_t)count;
            } else if (type == OT_INSERT && newlen < max_docsize) {
                // We can only perform an insert if the new document length
                // doesn't exceed max_docsize.

                size_t count = (rand() % max_ins) + 1;
                if (newlen + count >= max_docsize) {
                    count = max_docsize - newlen;
                }

                char* substr = malloc(sizeof(char) * count + 1);
                randstr(substr, count);
                docstr = insert_substr(docstr, substr, newpos);
                ot_insert(op, substr);

                sprintf(msg, "[INFO] Inserting the characters \"%s\" at "
                             "position %zu.",
                        substr, newpos);
                puts(msg);

                newpos += count;
                num_inserts += count;
                free(substr);
            } else if (type == OT_DELETE && pos < doclen) {
                // We can only perform a delete if we haven't already spanned
                // the entire document.

                uint r = (uint)rand();
                uint32_t count = (uint32_t)(r % (doclen - pos)) + 1;
                ot_delete(op, count);
                docstr = delete_substr(docstr, newpos, (size_t)count);

                sprintf(msg, "[INFO] Deleting %lld characters at position %zu.",
                        count, newpos);
                puts(msg);

                pos += (size_t)count;
                num_deletes += (size_t)count;
            }
        }

        if (composed == NULL) {
            composed = op;
        } else {
            char* composed_json = ot_encode(composed);
            char* op_json = ot_encode(op);
            sprintf(msg, "[INFO] Composing \"%s\" and \"%s\".", composed_json,
                    op_json);
            free(composed_json);
            free(op_json);
            puts(msg);

            ot_op* new_composed = ot_compose(composed, op);
            composed_json = ot_encode(new_composed);
            sprintf(msg, "[INFO] Compose(op%zu, op%zu) succeeded: \"%s\".",
                    i - 1, i, composed_json);
            puts(msg);
            free(composed_json);
            ot_free_op(composed);
            ot_free_op(op);
            composed = new_composed;
            if (composed == NULL) {
                sprintf(msg, "[FATAL] Compose(op%zu, op%zu) failed.", i - 1, i);
                puts(msg);
                exit(1);
            }
        }
    }

    // Compose the original document operation with the composed operation. If
    // the snapshot of the result doesn't equal the original document string,
    // then we exit with an error.
    char* doc_json = ot_encode(docop);
    char* composed_json = ot_encode(composed);
    sprintf(msg, "[INFO] Composing document \"%s\" and \"%s\".", doc_json,
            composed_json);
    free(doc_json);
    free(composed_json);
    puts(msg);

    ot_op* new_docop = ot_compose(docop, composed);
    ot_free_op(docop);
    ot_free_op(composed);

    snapshot = ot_snapshot(new_docop);
    assert_doc(docstr, snapshot);
    if (snapshot != NULL) {
        free(snapshot);
    }

    free(docstr);
    ot_free_op(new_docop);

    sprintf(msg, "[INFO] Document #%d composed without any errors.",
            docnum + 1);
    puts(msg);
    sprintf(msg, "[INFO] Total characters skipped: %zu.", num_skips);
    puts(msg);
    sprintf(msg, "[INFO] Total characters inserted: %zu.", num_inserts);
    puts(msg);
    sprintf(msg, "[INFO] Total characters deleted: %zu.", num_deletes);
    puts(msg);
    sprintf(msg, "[INFO] Maximum document size reached: %zu.", max_reached);
    puts(msg);
}

int main() {
    srand(seed);

    for (int i = 0; i < num_docs; ++i) {
        gen_doc(i);
    }

    return 0;
}
