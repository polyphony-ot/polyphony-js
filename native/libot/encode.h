#ifndef LIBOT_ENCODE_H
#define LIBOT_ENCODE_H

#include <inttypes.h>
#include "ot.h"
#include "doc.h"
#include "cjson/cJSON.h"

// Encodes an operation as a UTF-8 JSON string.
char* ot_encode(const ot_op* const op);

// ot_doc_encode encodes a document as a UTF-8 JSON string.
char* ot_encode_doc(const ot_doc* const doc);

// Encodes an error as a UTF-8 JSON string.
char* ot_encode_err(ot_err err);

#endif
