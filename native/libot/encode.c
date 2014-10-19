#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "encode.h"
#include "hex.h"

static cJSON* cjson_op(const ot_op* const op) {
    ot_comp* comps = op->comps.data;
    cJSON* components = cJSON_CreateArray();
    for (size_t i = 0; i < op->comps.len; ++i) {
        ot_comp_type t = comps[i].type;
        cJSON* component = cJSON_CreateObject();
        if (t == OT_SKIP) {
            cJSON_AddStringToObject(component, "type", "skip");
            uint32_t count = comps[i].value.skip.count;
            cJSON_AddNumberToObject(component, "count", count);
        } else if (t == OT_INSERT) {
            cJSON_AddStringToObject(component, "type", "insert");
            char* text = comps[i].value.insert.text;
            cJSON_AddStringToObject(component, "text", text);
        } else if (t == OT_DELETE) {
            cJSON_AddStringToObject(component, "type", "delete");
            uint32_t count = comps[i].value.delete.count;
            cJSON_AddNumberToObject(component, "count", count);
        } else if (t == OT_OPEN_ELEMENT) {
            cJSON_AddStringToObject(component, "type", "openElement");
            char* elem = comps[i].value.open_element.elem;
            cJSON_AddStringToObject(component, "element", elem);
        } else if (t == OT_CLOSE_ELEMENT) {
            cJSON_AddStringToObject(component, "type", "closeElement");
        } else if (t == OT_FORMATTING_BOUNDARY) {
            // TODO: Implement encoding formatting boundaries.
        }
        cJSON_AddItemToArray(components, component);
    }

    char parent[41] = { 0 };
    atohex(parent, op->parent, 20);

    char hash[41] = { 0 };
    atohex(hash, op->hash, 20);

    cJSON* root;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "clientId", op->client_id);
    cJSON_AddStringToObject(root, "parent", parent);
    cJSON_AddStringToObject(root, "hash", hash);
    cJSON_AddItemToObject(root, "components", components);

    return root;
}

char* ot_encode(const ot_op* const op) {
    cJSON* cjson = cjson_op(op);
    char* enc = cJSON_PrintUnformatted(cjson);
    cJSON_Delete(cjson);

    return enc;
}

char* ot_encode_doc(const ot_doc* const doc) {
    cJSON* root = cJSON_CreateArray();

    ot_op* history = (ot_op*)doc->history.data;
    for (size_t i = 0; i < doc->history.len; ++i) {
        cJSON_AddItemToArray(root, cjson_op(history + i));
    }

    char* enc = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return enc;
}

char* ot_encode_err(ot_err err) {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "errorCode", err);

    char* enc = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return enc;
}
