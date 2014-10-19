#ifndef LIBOT_HEX_H
#define LIBOT_HEX_H

#include <stdint.h>

// Provides functions for encoding and decoding hex strings.

// Converts a hex string to an array. The hex string is decoded and copied into
// a. a must have at least len/2 space allocated to hold the hex value. alen is
// the length of the array and hexlen is the length of the hex string.
//
// Warning: This function does not perform any sort of input validation, so it's
// possible to pass in an invalid hex string and get an undefined result.
int hextoa(char* const a, size_t alen, const char* const hex, size_t hexlen);

// Converts an array to a hex string. The array is encoded and copied into hex.
// hex must have at least len*2 space allocated to hold the encoded hex value.
// len is the length of the array.
int atohex(char* const hex, const char* const a, size_t len);

#endif
