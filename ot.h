#ifndef OT_H
#define OT_H

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "array.h"

typedef enum {
    OT_ERR_NONE = 0,

    // Couldn't decode an operation because its parent field was missing.
    OT_ERR_PARENT_MISSING = 1,

    // Couldn't decode an operation because its clientId fields was missing.
    OT_ERR_CLIENT_ID_MISSING = 2,

    // Couldn't decode an operation because its components field was missing.
    OT_ERR_COMPONENTS_MISSING = 3,

    // Couldn't decode an operation because one of its components was invalid.
    OT_ERR_INVALID_COMPONENT = 4,

    // Couldn't decode an operation because its hash field was missing.
    OT_ERR_HASH_MISSING = 5,

    // Couldn't parse a JSON string.
    OT_ERR_INVALID_JSON = 6,

    // Client couldn't buffer the applied operation, usually because it wasn't
    // composable with the buffer.
    OT_ERR_BUFFER_FAILED = 7,

    // Couldn't append an operation to a document, usually because it wasn't
    // composable with the current document state.
    OT_ERR_APPEND_FAILED = 8,

    // Couldn't transform two operations, usually because the two operations
    // weren't parented off of the same state.
    OT_ERR_XFORM_FAILED = 9,

    // Couldn't compose two operations, usually because the first op wasn't a
    // parent of the second op.
    OT_ERR_COMPOSE_FAILED = 10,

    // Couldn't append an operation to a document because it would cause the
    // document to go beyond its maximum size.
    OT_ERR_MAX_SIZE = 11
} ot_err;

typedef struct ot_fmt {
    char* name;
    char* value;
} ot_fmt;

typedef enum {
    OT_SKIP = 0,
    OT_INSERT = 1,
    OT_DELETE = 2,
    OT_OPEN_ELEMENT = 3,
    OT_CLOSE_ELEMENT = 4,
    OT_FORMATTING_BOUNDARY = 5
} ot_comp_type;

typedef struct ot_comp_skip {
    uint32_t count;
} ot_comp_skip;

typedef struct ot_comp_insert {
    char* text;
} ot_comp_insert;

typedef struct ot_comp_delete {
    uint32_t count;
} ot_comp_delete;

typedef struct ot_comp_open_element {
    char* elem;
} ot_comp_open_element;

typedef struct ot_comp_fmtbound {
    array start;
    array end;
} ot_comp_fmtbound;

typedef struct ot_comp {
    ot_comp_type type;
    union {
        ot_comp_skip skip;
        ot_comp_insert insert;
        ot_comp_delete delete;
        ot_comp_open_element open_element;
        ot_comp_fmtbound fmtbound;
    } value;
} ot_comp;

typedef struct ot_op {
    uint32_t client_id;
    char parent[20];
    char hash[20];
    array comps;
} ot_op;

typedef enum {
    OT_CONNECTED = 0,
    OT_DISCONNECTED = 1,
    OT_OP_APPLIED = 2,
    OT_OP_INCOMING = 3,
    OT_ERROR = 4
} ot_event_type;

typedef int (*send_func)(const char*);
typedef int (*ot_event_func)(ot_event_type, ot_op*);

ot_op* ot_new_op();
void ot_free_op(ot_op* op);
void ot_free_comp(ot_comp* comp);
ot_op* ot_dup_op(const ot_op* op);
bool ot_equal(const ot_op* op1, const ot_op* op2);
void ot_skip(ot_op* op, uint32_t count);

// Appends an insert component to an operation. text is copied and not freed, so
// the caller must free it manually.
//
// If op already ends with an insert component, this function will append text
// to the existing insert. Otherwise, it will create a new insert component and
// append it to op.
void ot_insert(ot_op* op, const char* text);
void ot_delete(ot_op* op, uint32_t count);
void ot_open_element(ot_op* op, const char* elem);
void ot_close_element(ot_op* op);
void ot_start_fmt(ot_op* op, const char* name, const char* value);
void ot_end_fmt(ot_op* op, const char* name, const char* value);
char* ot_snapshot(ot_op* op);
uint32_t ot_size(const ot_op* op);
uint32_t ot_comp_size(const ot_comp* comp);

ot_comp_fmtbound* ot_new_fmtbound();

typedef struct ot_iter {
    const ot_op* op; // Op to iterator over.
    size_t pos;      // Current component position.
    size_t offset;   // Offset within current component.
    bool started;    // Set true when ot_iter_next is called the first time.
} ot_iter;

// Initializes a new iterator pointing to the -1 position. This means that
// ot_iter_next must also be called before using the iterator.
//
// This was done on purpose so the iterator can be easily used in a while loop.
// I.e. - ot_iter_init(&iter); while(ot_iter_next(ot_iter* iter)) { .. }
void ot_iter_init(ot_iter* iter, const ot_op* op);
bool ot_iter_next(ot_iter* iter);
bool ot_iter_skip(ot_iter* iter, size_t count);

#endif
