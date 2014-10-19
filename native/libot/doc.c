#include "doc.h"

ot_doc* ot_new_doc(void) {
    ot_doc* doc = malloc(sizeof(ot_doc));
    array_init(&doc->history, sizeof(ot_op));
    doc->composed = NULL;
    doc->size = 0;
    doc->max_size = 0;
    return doc;
}

void ot_free_doc(ot_doc* doc) {
    // Free the components of every op in the document's history.
    ot_op* ops = doc->history.data;
    for (size_t i = 0; i < doc->history.len; ++i) {
        ot_op* op = ops + i;
        ot_comp* comps = op->comps.data;
        for (size_t j = 0; j < op->comps.len; ++j) {
            ot_free_comp(comps + j);
        }
        array_free(&op->comps);
    }

    // Free the history array, which frees the all of ops.
    array_free(&doc->history);

    // When the history length is less than 2, then the composed op is just
    // pointing to the first op in the history (which has already been freed).
    if (doc->history.len > 1) {
        ot_free_op(doc->composed);
    }

    free(doc);
}

ot_err ot_doc_append(ot_doc* doc, ot_op** op) {
    if (doc->max_size > 0 && ot_size(*op) + doc->size > doc->max_size) {
        return OT_ERR_MAX_SIZE;
    }

    // Move the op into the document's history array.
    ot_op* head = array_append(&doc->history);
    memcpy(head, *op, sizeof(ot_op));

    size_t len = doc->history.len;
    if (len > 1) {
        ot_op* history = (ot_op*)doc->history.data;
        ot_op* prev = &history[len - 2];
        memcpy(head->parent, prev->hash, 20);
    } else {
        char zero[20] = { 0 };
        memcpy(head->parent, zero, 20);
    }

    // If we're appending the first op, then the composed state will simply be
    // the first op in the history. If we're appending the second op, then we
    // must ensure that the composed op still points to the first op in the
    // history in case its location changed after calling array_append (which
    // may have reallocated the array to a different location in memory).
    if (doc->history.len <= 2) {
        doc->composed = (ot_op*)doc->history.data;
    }

    // If we're appending any op after the first, then we must compose the new
    // op with the currently composed state to get the new composed state.
    if (doc->history.len > 1) {
        ot_op* new_composed = ot_compose(doc->composed, head);
        if (new_composed == NULL) {
            doc->history.len--;
            return OT_ERR_APPEND_FAILED;
        }

        // Only free the previously composed op if this is at least the 2nd
        // composition (aka 3rd op in the history). This is because the first
        // composed op for a document points to the first op in the doc's
        // history, and we don't want to free anything in the history.
        if (doc->history.len > 2) {
            ot_free_op(doc->composed);
        }
        doc->composed = new_composed;
    }

    // Don't use ot_free_op because we only want to free the ot_op struct, not
    // its components. We must also only free op if the composition was a
    // success.
    free(*op);
    *op = head;

    // The newly composed operation wil have the same hash as the appended op,
    // so we can get away with calculating the hash once and then copying it.
    hash_op(doc->composed);
    memcpy(head->hash, doc->composed->hash, 20);
    doc->size = ot_size(doc->composed);

    return OT_ERR_NONE;
}

ot_op* ot_doc_compose_after(const ot_doc* doc, const char* after) {
    array history = doc->history;
    if (history.len == 0) {
        return NULL;
    }

    bool after_null = true;
    for (int i = 0; i < 20; ++i) {
        if (after[i] != 0) {
            after_null = false;
            break;
        }
    }

    bool found = false;
    size_t start = 0;
    ot_op* ops = (ot_op*)history.data;
    if (!after_null) {
        for (size_t i = history.len - 1; i < history.len; --i) {
            ot_op* op = ops + i;
            if (memcmp(op->hash, after, sizeof(char) * 20) == 0) {
                start = i + 1;
                found = true;
                break;
            }
        }

        if (!found) {
            return NULL;
        }
    }

    ot_op* composed = ot_dup_op(ops + start);
    ot_op* temp;
    for (size_t i = start + 1; i < history.len; ++i) {
        temp = ot_compose(composed, ops + i);
        ot_free_op(composed);
        composed = temp;
        if (composed == NULL) {
            return NULL;
        }
    }

    return composed;
}

ot_op* ot_doc_last(const ot_doc* doc) {
    size_t i = doc->history.len - 1;
    ot_op* history = (ot_op*)doc->history.data;
    return history + i;
}
