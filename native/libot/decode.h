#ifndef LIBOT_DECODE_H
#define LIBOT_DECODE_H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "ot.h"
#include "array.h"
#include "hex.h"
#include "cjson/cJSON.h"
#include "doc.h"

// Decodes an operation from a UTF-8 JSON string.
ot_err ot_decode(ot_op* op, const char* const json);

// ot_decode_doc decodes a document from a UTF-8 JSON string.
ot_err ot_decode_doc(ot_doc* doc, const char* const json);

#endif
