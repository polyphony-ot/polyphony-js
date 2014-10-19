#ifndef LIBOT_DOC_H
#define LIBOT_DOC_H

#include <stdlib.h>
#include <string.h>
#include "array.h"
#include "compose.h"
#include "sha1.h"
#include "ot.h"

// Implements an OT document, which is effectively an array of composable
// operations.
typedef struct ot_doc {
    array history;
    ot_op* composed;
    uint32_t size;
    uint32_t max_size;
} ot_doc;

// Creates and returns a new document. It must be freed by the caller using
// ot_free_doc.
ot_doc* ot_new_doc(void);

// Frees a document that was created with ot_new_doc.
void ot_free_doc(ot_doc* doc);

// Appends an operation to a document. The operation must be composable with the
// current state of the document. Once an operation has been appended to a
// document, it is moved into the document's history and op is updated to point
// to its new location.
ot_err ot_doc_append(ot_doc* doc, ot_op** op);

// Composes a half-closed range of operations in the document's history. That
// is, every operation after (but not including) "after" is composed with every
// operation up to and including the most recent operation (after, latest].
ot_op* ot_doc_compose_after(const ot_doc* doc, const char* after);

// ot_doc_last returns the last op (which is also the most recent op) in the
// document's history.
ot_op* ot_doc_last(const ot_doc* doc);

#endif
