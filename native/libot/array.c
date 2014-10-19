#include <stdlib.h>
#include "array.h"

void array_init(array* arr, size_t size) {
    arr->len = 0;
    arr->cap = 0;
    arr->size = size;
    arr->data = NULL;
}

void array_free(array* arr) { free(arr->data); }

void array_copy(array* dst, const array* src) {
    dst->len = src->len;
    dst->cap = src->len;
    dst->size = src->size;

    size_t datalen = src->size * src->len;
    dst->data = malloc(datalen);
    memcpy(dst->data, src->data, datalen);
}

void array_ensure_size(array* arr) {
    if (arr->len == 0) {
        arr->cap = 1;
        arr->data = malloc(arr->size);
    } else if (arr->len >= arr->cap) {
        arr->cap *= 2;
        arr->data = realloc(arr->data, arr->size * arr->cap);
    }
}

void* array_append(array* arr) {
    array_ensure_size(arr);
    size_t temp = arr->len;
    arr->len++;

    return ((char*)arr->data) + (arr->size * temp);
}
