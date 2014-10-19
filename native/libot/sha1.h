#ifndef SHA1_H_
#define SHA1_H_

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "ot.h"

/* error codes [will be expanded in future releases] */
enum {
    CRYPT_OK = 0,      /* Result OK */
    CRYPT_INVALID_ARG, /* Generic invalid argument */
};

struct sha1_state {
    uint64_t length;
    uint32_t state[5], curlen;
    char buf[64];
};

typedef union Hash_state {
    char dummy[1];
    struct sha1_state sha1;
    void* data;
} hash_state;

/** hash descriptor */
extern struct ltc_hash_descriptor {
    /** name of hash */
    char* name;
    /** internal ID */
    char ID;
    /** Size of digest in octets */
    uint32_t hashsize;
    /** Input block size in octets */
    uint32_t blocksize;
    /** ASN.1 OID */
    uint32_t OID[16];
    /** Length of DER encoding */
    uint32_t OIDlen;

    /** Init a hash state
     @param hash   The hash to initialize
     @return CRYPT_OK if successful
     */
    int (*init)(hash_state* hash);
    /** Process a block of data
     @param hash   The hash state
     @param in     The data to hash
     @param inlen  The length of the data (octets)
     @return CRYPT_OK if successful
     */
    int (*process)(hash_state* hash, const char* in, uint32_t inlen);
    /** Produce the digest and store it
     @param hash   The hash state
     @param out    [out] The destination of the digest
     @return CRYPT_OK if successful
     */
    int (*done)(hash_state* hash, char* out);
    /** Self-test
     @return CRYPT_OK if successful, CRYPT_NOP if self-tests have been disabled
     */
    int (*test)(void);

    /* accelerated hmac callback: if you need to-do multiple packets just use
     * the generic hmac_memory and provide a hash callback */
    int (*hmac_block)(const char* key, uint32_t keylen, const char* in,
                      uint32_t inlen, char* out, uint32_t* outlen);

} hash_descriptor[];

int sha1_init(hash_state* md);
int sha1_process(hash_state* md, const char* in, uint32_t inlen);
int sha1_done(hash_state* md, char* hash);
void hash_op(ot_op* op);
extern const struct ltc_hash_descriptor sha1_desc;

#define LOAD32H(x, y)                                                          \
    {                                                                          \
        x = ((uint32_t)((y)[0] & 255) << 24) |                                 \
            ((uint32_t)((y)[1] & 255) << 16) |                                 \
            ((uint32_t)((y)[2] & 255) << 8) | ((uint32_t)((y)[3] & 255));      \
    }

#define STORE32H(x, y)                                                         \
    {                                                                          \
        (y)[0] = (char)(((x) >> 24) & 255);                                    \
        (y)[1] = (char)(((x) >> 16) & 255);                                    \
        (y)[2] = (char)(((x) >> 8) & 255);                                     \
        (y)[3] = (char)((x) & 255);                                            \
    }

#define STORE64H(x, y)                                                         \
    {                                                                          \
        (y)[0] = (char)(((x) >> 56) & 255);                                    \
        (y)[1] = (char)(((x) >> 48) & 255);                                    \
        (y)[2] = (char)(((x) >> 40) & 255);                                    \
        (y)[3] = (char)(((x) >> 32) & 255);                                    \
        (y)[4] = (char)(((x) >> 24) & 255);                                    \
        (y)[5] = (char)(((x) >> 16) & 255);                                    \
        (y)[6] = (char)(((x) >> 8) & 255);                                     \
        (y)[7] = (char)((x) & 255);                                            \
    }

#define ROL(x, y)                                                              \
    ((((uint32_t)(x) << (uint32_t)((y) & 31)) |                                \
      (((uint32_t)(x) & 0xFFFFFFFFUL) >> (uint32_t)(32 - ((y) & 31)))) &       \
     0xFFFFFFFFUL)

#define ROLc(x, y)                                                             \
    ((((uint32_t)(x) << (uint32_t)((y) & 31)) |                                \
      (((uint32_t)(x) & 0xFFFFFFFFUL) >> (uint32_t)(32 - ((y) & 31)))) &       \
     0xFFFFFFFFUL)

#define LTC_ARGCHK(x) assert((x))

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define HASH_PROCESS(func_name, compress_name, state_var, block_size)          \
    int func_name(hash_state* md, const char* in, uint32_t inlen) {            \
        uint32_t n;                                                            \
        int err;                                                               \
        LTC_ARGCHK(md != NULL);                                                \
        LTC_ARGCHK(in != NULL);                                                \
        if (md->state_var.curlen > sizeof(md->state_var.buf)) {                \
            return CRYPT_INVALID_ARG;                                          \
        }                                                                      \
        while (inlen > 0) {                                                    \
            if (md->state_var.curlen == 0 && inlen >= block_size) {            \
                if ((err = compress_name(md, (char*)in)) != CRYPT_OK) {        \
                    return err;                                                \
                }                                                              \
                md->state_var.length += block_size * 8;                        \
                in += block_size;                                              \
                inlen -= block_size;                                           \
            } else {                                                           \
                n = MIN(inlen, (block_size - md->state_var.curlen));           \
                memcpy(md->state_var.buf + md->state_var.curlen, in,           \
                       (size_t)n);                                             \
                md->state_var.curlen += n;                                     \
                in += n;                                                       \
                inlen -= n;                                                    \
                if (md->state_var.curlen == block_size) {                      \
                    if ((err = compress_name(md, md->state_var.buf)) !=        \
                        CRYPT_OK) {                                            \
                        return err;                                            \
                    }                                                          \
                    md->state_var.length += 8 * block_size;                    \
                    md->state_var.curlen = 0;                                  \
                }                                                              \
            }                                                                  \
        }                                                                      \
        return CRYPT_OK;                                                       \
    }

#endif
