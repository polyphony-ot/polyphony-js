#include "decode.h"

// decode_cjson_op decodes a cJSON item into an op.
ot_err decode_cjson_op(cJSON* json, ot_op* op) {
    cJSON* error_code = cJSON_GetObjectItem(json, "errorCode");
    if (error_code != NULL) {
        return error_code->valueint;
    }

    cJSON* client_idf = cJSON_GetObjectItem(json, "clientId");
    if (client_idf == NULL) {
        return OT_ERR_CLIENT_ID_MISSING;
    }
    op->client_id = (uint32_t)client_idf->valueint;

    cJSON* parentf = cJSON_GetObjectItem(json, "parent");
    if (parentf == NULL) {
        return OT_ERR_PARENT_MISSING;
    }
    memset(op->parent, 0, 20);
    hextoa(op->parent, 20, parentf->valuestring, strlen(parentf->valuestring));

    cJSON* hashf = cJSON_GetObjectItem(json, "hash");
    if (hashf == NULL) {
        return OT_ERR_HASH_MISSING;
    }
    memset(op->hash, 0, 20);
    hextoa(op->hash, 20, hashf->valuestring, strlen(hashf->valuestring));

    cJSON* components = cJSON_GetObjectItem(json, "components");
    if (components == NULL) {
        return OT_ERR_COMPONENTS_MISSING;
    }

    int size = cJSON_GetArraySize(components);
    for (int i = 0; i < size; ++i) {
        cJSON* item = cJSON_GetArrayItem(components, i);
        char* type = cJSON_GetObjectItem(item, "type")->valuestring;
        if (memcmp(type, "skip", 4) == 0) {
            ot_comp* skip = array_append(&op->comps);
            skip->type = OT_SKIP;
            skip->value.skip.count =
                (uint32_t)cJSON_GetObjectItem(item, "count")->valueint;
        } else if (memcmp(type, "insert", 6) == 0) {
            ot_comp* insert = array_append(&op->comps);
            insert->type = OT_INSERT;

            char* text = cJSON_GetObjectItem(item, "text")->valuestring;
            size_t text_size = sizeof(char) * (strlen(text) + 1);
            insert->value.insert.text = malloc(text_size);
            memcpy(insert->value.insert.text, text, text_size);
        } else if (memcmp(type, "delete", 6) == 0) {
            ot_comp* delete = array_append(&op->comps);
            delete->type = OT_DELETE;
            delete->value.delete.count =
                (uint32_t)cJSON_GetObjectItem(item, "count")->valueint;
        } else if (memcmp(type, "openElement", 11) == 0) {
            ot_comp* open_elem = array_append(&op->comps);
            open_elem->type = OT_OPEN_ELEMENT;
            open_elem->value.open_element.elem =
                cJSON_GetObjectItem(item, "element")->valuestring;
        } else if (memcmp(type, "closeElement", 12) == 0) {
            ot_comp* open_elem = array_append(&op->comps);
            open_elem->type = OT_CLOSE_ELEMENT;
        } else if (memcmp(type, "formattingBoundary", 18) == 0) {
            ot_comp* open_elem = array_append(&op->comps);
            open_elem->type = OT_FORMATTING_BOUNDARY;
        } else {
            return OT_ERR_INVALID_COMPONENT;
        }
    }

    return OT_ERR_NONE;
}

// TODO: Finish implementing decoding of formatting boundaries.
ot_err ot_decode(ot_op* op, const char* json) {
    cJSON* root = cJSON_Parse(json);
    if (root == NULL) {
        return OT_ERR_INVALID_JSON;
    }

    ot_err err = decode_cjson_op(root, op);
    cJSON_Delete(root);
    return err;
}

ot_err ot_decode_doc(ot_doc* doc, const char* const json) {
    cJSON* root = cJSON_Parse(json);
    if (root == NULL) {
        return OT_ERR_INVALID_JSON;
    }

    int max = cJSON_GetArraySize(root);
    for (int i = 0; i < max; ++i) {
        cJSON* item = cJSON_GetArrayItem(root, i);

        ot_op* op = ot_new_op();
        ot_err err = decode_cjson_op(item, op);
        if (err != OT_ERR_NONE) {
            cJSON_Delete(root);
            return err;
        }

        err = ot_doc_append(doc, &op);
        if (err != OT_ERR_NONE) {
            cJSON_Delete(root);
            return err;
        }
    }

    cJSON_Delete(root);
    return OT_ERR_NONE;
}
