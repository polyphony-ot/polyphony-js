#include "compose.h"

typedef struct pair {
    size_t first;
    size_t second;
} pair;

static size_t min(size_t s1, size_t s2) {
    if (s1 < s2) {
        return s1;
    } else {
        return s2;
    }
}

static pair ot_compose_skip_skip(ot_comp_skip skip1, size_t offset1,
                                 ot_comp_skip skip2, size_t offset2,
                                 ot_op* composed) {

    size_t skip1_count = (size_t)skip1.count - offset1;
    size_t skip2_count = (size_t)skip2.count - offset2;
    size_t min_len = min(skip1_count, skip2_count);

    ot_skip(composed, (uint32_t)min_len);

    return (pair) { min_len, min_len };
}

static pair ot_compose_skip_insert(ot_comp_skip skip, size_t skip_offset,
                                   ot_comp_insert insert, size_t insert_offset,
                                   ot_op* composed) {

    size_t insert_len = strlen(insert.text) - insert_offset;
    size_t skip_len = (size_t)skip.count - skip_offset;
    size_t min_len = min(skip_len, insert_len);

    char* substr = malloc(sizeof(char) * min_len + 1);
    memcpy(substr, insert.text + insert_offset, min_len);
    substr[min_len] = '\0';
    ot_insert(composed, substr);
    free(substr);

    return (pair) { 0, min_len };
}

static pair ot_compose_skip_delete(ot_comp_skip skip, size_t skip_offset,
                                   ot_comp_delete delete, size_t delete_offset,
                                   ot_op* composed) {

    size_t skip_len = (size_t)skip.count - skip_offset;
    size_t delete_len = (size_t) delete.count - delete_offset;
    size_t min_len = min(skip_len, delete_len);

    ot_delete(composed, (uint32_t)min_len);

    return (pair) { min_len, min_len };
}

static pair ot_compose_insert_skip(ot_comp_insert insert, size_t insert_offset,
                                   ot_comp_skip skip, size_t skip_offset,
                                   ot_op* composed) {

    size_t insert_len = strlen(insert.text) - insert_offset;
    size_t skip_len = (size_t)skip.count - skip_offset;
    size_t min_len = min(skip_len, insert_len);

    char* substr = malloc(sizeof(char) * min_len + 1);
    memcpy(substr, insert.text + insert_offset, min_len);
    substr[min_len] = '\0';
    ot_insert(composed, substr);
    free(substr);

    return (pair) { min_len, min_len };
}

static pair ot_compose_insert_insert(ot_comp_insert insert1, size_t offset1,
                                     ot_comp_insert insert2, size_t offset2,
                                     ot_op* composed) {

    size_t insert1_len = strlen(insert1.text) - offset1;
    size_t insert2_len = strlen(insert2.text) - offset2;
    size_t min_len = min(insert1_len, insert2_len);

    char* substr = malloc(sizeof(char) * min_len + 1);
    memcpy(substr, insert2.text + offset2, min_len);
    substr[min_len] = '\0';
    ot_insert(composed, substr);
    free(substr);

    return (pair) { 0, min_len };
}

static pair ot_compose_insert_delete(ot_comp_insert ins, size_t ins_offset,
                                     ot_comp_delete del, size_t del_offset) {

    size_t ins_len = strlen(ins.text) - ins_offset;
    size_t del_len = (size_t)del.count - del_offset;
    size_t min_len = min(ins_len, del_len);

    return (pair) { min_len, min_len };
}

static pair ot_compose_delete(ot_comp_delete del, size_t del_offset,
                              ot_op* composed) {

    size_t del_len = (size_t)del.count - del_offset;
    ot_delete(composed, (uint32_t)del_len);

    return (pair) { del_len, 0 };
}

ot_op* ot_compose(ot_op* op1, ot_op* op2) {
    ot_op* composed = ot_new_op();
    composed->client_id = op1->client_id;
    memcpy(composed->parent, op1->parent, 20);

    ot_comp* op1_comps = op1->comps.data;
    ot_comp* op2_comps = op2->comps.data;

    ot_iter op1_iter;
    ot_iter_init(&op1_iter, op1);

    ot_iter op2_iter;
    ot_iter_init(&op2_iter, op2);

    bool op1_next = ot_iter_next(&op1_iter);
    bool op2_next = ot_iter_next(&op2_iter);
    while (op1_next || op2_next) {
        ot_comp* op1_comp;
        if (op1_next) {
            op1_comp = op1_comps + op1_iter.pos;
        } else {
            op1_comp = NULL;
        }

        ot_comp* op2_comp;
        if (op2_next) {
            op2_comp = op2_comps + op2_iter.pos;
        } else {
            op2_comp = NULL;
        }

        if (op1_comp == NULL) {
            if (op2_comp == NULL) {
                assert(!"Both op components should never be NULL.");
            } else if (op2_comp->type == OT_INSERT) {
                ot_insert(composed, op2_comp->value.insert.text);
                size_t len = strlen(op2_comp->value.insert.text);
                op2_next = ot_iter_skip(&op2_iter, len);
            } else if (op2_comp->type == OT_OPEN_ELEMENT) {
                // TODO: Stub
            } else if (op2_comp->type == OT_CLOSE_ELEMENT) {
                // TODO: Stub
            } else if (op2_comp->type == OT_FORMATTING_BOUNDARY) {
                // TODO: Stub
            } else {
                // Error out since these two components are not composable. You
                // cannot skip or delete what doesn't exist in the first op.
                ot_free_op(composed);
                return NULL;
            }
        } else if (op2_comp == NULL) {
            if (op1_comp == NULL) {
                assert(!"Both op components should never be NULL.");
            } else if (op1_comp->type == OT_DELETE) {
                uint32_t count = op1_comp->value.delete.count;
                ot_delete(composed, count);
                op1_next = ot_iter_skip(&op1_iter, (size_t)count);
            } else {
                // Error out since these two components are not composable. The
                // second op must span the entire first op. Deletes are the only
                // component that the second op cannot span.
                ot_free_op(composed);
                return NULL;
            }
        } else if (op1_comp->type == OT_SKIP) {
            ot_comp_skip op1_skip = op1_comp->value.skip;

            if (op2_comp->type == OT_SKIP) {
                ot_comp_skip op2_skip = op2_comp->value.skip;

                pair p =
                    ot_compose_skip_skip(op1_skip, op1_iter.offset, op2_skip,
                                         op2_iter.offset, composed);

                op1_next = ot_iter_skip(&op1_iter, p.first);
                op2_next = ot_iter_skip(&op2_iter, p.second);
            } else if (op2_comp->type == OT_INSERT) {
                ot_comp_insert op2_insert = op2_comp->value.insert;

                pair p = ot_compose_skip_insert(op1_skip, op1_iter.offset,
                                                op2_insert, op2_iter.offset,
                                                composed);

                op1_next = ot_iter_skip(&op1_iter, p.first);
                op2_next = ot_iter_skip(&op2_iter, p.second);
            } else if (op2_comp->type == OT_DELETE) {
                ot_comp_delete op2_delete = op2_comp->value.delete;

                pair p = ot_compose_skip_delete(op1_skip, op1_iter.offset,
                                                op2_delete, op2_iter.offset,
                                                composed);

                op1_next = ot_iter_skip(&op1_iter, p.first);
                op2_next = ot_iter_skip(&op2_iter, p.second);
            }
        } else if (op1_comp->type == OT_INSERT) {
            ot_comp_insert op1_insert = op1_comp->value.insert;

            if (op2_comp->type == OT_SKIP) {
                ot_comp_skip op2_skip = op2_comp->value.skip;

                pair p =
                    ot_compose_insert_skip(op1_insert, op1_iter.offset,
                                           op2_skip, op2_iter.offset, composed);

                op1_next = ot_iter_skip(&op1_iter, p.first);
                op2_next = ot_iter_skip(&op2_iter, p.second);
            } else if (op2_comp->type == OT_INSERT) {
                ot_comp_insert op2_insert = op2_comp->value.insert;

                pair p = ot_compose_insert_insert(op1_insert, op1_iter.offset,
                                                  op2_insert, op2_iter.offset,
                                                  composed);

                op1_next = ot_iter_skip(&op1_iter, p.first);
                op2_next = ot_iter_skip(&op2_iter, p.second);
            } else if (op2_comp->type == OT_DELETE) {
                ot_comp_delete op2_delete = op2_comp->value.delete;

                pair p = ot_compose_insert_delete(op1_insert, op1_iter.offset,
                                                  op2_delete, op2_iter.offset);

                op1_next = ot_iter_skip(&op1_iter, p.first);
                op2_next = ot_iter_skip(&op2_iter, p.second);
            }
        } else if (op1_comp->type == OT_DELETE) {
            ot_comp_delete op1_delete = op1_comp->value.delete;

            pair p = ot_compose_delete(op1_delete, op1_iter.offset, composed);

            op1_next = ot_iter_skip(&op1_iter, p.first);
            op2_next = ot_iter_skip(&op2_iter, p.second);
        }
    }

    return composed;
}
