#include <stddef.h>
#include "hex.h"

int hextoa(char* const a, size_t alen, const char* const hex, size_t hexlen) {
    size_t j = alen - (hexlen / 2);
    for (size_t i = 0; i < hexlen; ++i) {
        char c;
        if (hex[i] >= 0x61) { // Lowercase letter
            c = (char)((hex[i] - 0x57) << 4);
        } else if (hex[i] >= 0x41) { // Uppercase letter
            c = (char)((hex[i] - 0x37) << 4);
        } else { // Number
            c = (char)((hex[i] - 0x30) << 4);
        }

        ++i;

        if (hex[i] >= 0x61) {
            c += hex[i] - 0x57;
        } else if (hex[i] >= 0x41) {
            c += hex[i] - 0x37;
        } else {
            c += hex[i] - 0x30;
        }

        a[j] = c;
        ++j;
    }

    return 0;
}

int atohex(char* const hex, const char* const a, size_t len) {
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        // Skip if the byte is a leading 0 unless we're on the last byte.
        if (a[i] == 0 && j == 0 && (i != len - 1)) {
            continue;
        }

        char high = a[i] >> 4;
        char low = a[i] & 0x0F;
        if (high <= 0x9) {
            hex[j] = high + 0x30;
        } else {
            hex[j] = high + 0x57;
        }
        ++j;

        if (low <= 0x9) {
            hex[j] += low + 0x30;
        } else {
            hex[j] += low + 0x57;
        }
        ++j;
    }
    hex[j] = '\0';

    return 0;
}
