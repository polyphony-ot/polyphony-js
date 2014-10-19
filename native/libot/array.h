#ifndef LIBOT_ARRAY_H
#define LIBOT_ARRAY_H

#include <stddef.h>
#include <string.h>

// Implements a dynamically resizing array.
//
// An array has a length and a capacity. The length is the actual number of
// items in the array. The capacity is the maximum number of items the array can
// hold. Therefore, capacity >= len. The array capacity will grow by 2 * cap
// whenever the length reaches capacity.
typedef struct array {
    size_t len;  // The number of items.
    size_t cap;  // The max capacity.
    size_t size; // The size of each item.
    void* data;
} array;

// Initializes an array where size is the size of its items.
void array_init(array* arr, size_t size);

// Frees an initialized array.
void array_free(array* arr);

// Copies an array from src into dst. The copied array will have a capacity
// equal to the length of the source array.
void array_copy(array* dst, const array* src);

// Ensures that the array has enough capacity for another element.
void array_ensure_size(array* arr);

// Allocates space for another item at the end of the array and returns a
// pointer to it. The amount of space allocated is at least equal to size.
void* array_append(array* arr);

#endif
