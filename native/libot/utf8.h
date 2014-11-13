#ifndef LIBOT_UTF8_H
#define LIBOT_UTF8_H

#include <inttypes.h>
#include <string.h>

uint32_t utf8_cps(const char byte);

uint32_t utf8_length(const char* str);

uint32_t utf8_bytes(const char* str, uint32_t length);

#endif
