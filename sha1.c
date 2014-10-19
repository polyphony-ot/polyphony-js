#include "sha1.h"

const struct ltc_hash_descriptor sha1_desc = { "sha1",
                                               2,
                                               20,
                                               64,

                                               /* OID */
                                               { 1, 3, 14, 3, 2, 26, },
                                               6,
                                               &sha1_init,
                                               &sha1_process,
                                               &sha1_done,
                                               NULL,
                                               NULL };

#define F0(x, y, z) (z ^ (x&(y ^ z)))
#define F1(x, y, z) (x ^ y ^ z)
#define F2(x, y, z) ((x& y) | (z&(x | y)))
#define F3(x, y, z) (x ^ y ^ z)

static int sha1_compress(hash_state* md, char* buf) {
    uint32_t a, b, c, d, e, W[80], i;

    /* copy the state into 512-bits into W[0..15] */
    for (i = 0; i < 16; i++) {
        LOAD32H(W[i], buf + (4 * i));
    }

    /* copy state */
    a = md->sha1.state[0];
    b = md->sha1.state[1];
    c = md->sha1.state[2];
    d = md->sha1.state[3];
    e = md->sha1.state[4];

    /* expand it */
    for (i = 16; i < 80; i++) {
        W[i] = ROL(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);
    }

/* compress */
/* round one */
#define FF0(a, b, c, d, e, i)                                                  \
    e = (ROLc(a, 5) + F0(b, c, d) + e + W[i] + 0x5a827999UL);                  \
    b = ROLc(b, 30);
#define FF1(a, b, c, d, e, i)                                                  \
    e = (ROLc(a, 5) + F1(b, c, d) + e + W[i] + 0x6ed9eba1UL);                  \
    b = ROLc(b, 30);
#define FF2(a, b, c, d, e, i)                                                  \
    e = (ROLc(a, 5) + F2(b, c, d) + e + W[i] + 0x8f1bbcdcUL);                  \
    b = ROLc(b, 30);
#define FF3(a, b, c, d, e, i)                                                  \
    e = (ROLc(a, 5) + F3(b, c, d) + e + W[i] + 0xca62c1d6UL);                  \
    b = ROLc(b, 30);

    for (i = 0; i < 20;) {
        FF0(a, b, c, d, e, i++);
        FF0(e, a, b, c, d, i++);
        FF0(d, e, a, b, c, i++);
        FF0(c, d, e, a, b, i++);
        FF0(b, c, d, e, a, i++);
    }

    /* round two */
    for (; i < 40;) {
        FF1(a, b, c, d, e, i++);
        FF1(e, a, b, c, d, i++);
        FF1(d, e, a, b, c, i++);
        FF1(c, d, e, a, b, i++);
        FF1(b, c, d, e, a, i++);
    }

    /* round three */
    for (; i < 60;) {
        FF2(a, b, c, d, e, i++);
        FF2(e, a, b, c, d, i++);
        FF2(d, e, a, b, c, i++);
        FF2(c, d, e, a, b, i++);
        FF2(b, c, d, e, a, i++);
    }

    /* round four */
    for (; i < 80;) {
        FF3(a, b, c, d, e, i++);
        FF3(e, a, b, c, d, i++);
        FF3(d, e, a, b, c, i++);
        FF3(c, d, e, a, b, i++);
        FF3(b, c, d, e, a, i++);
    }

#undef FF0
#undef FF1
#undef FF2
#undef FF3

    /* store */
    md->sha1.state[0] = md->sha1.state[0] + a;
    md->sha1.state[1] = md->sha1.state[1] + b;
    md->sha1.state[2] = md->sha1.state[2] + c;
    md->sha1.state[3] = md->sha1.state[3] + d;
    md->sha1.state[4] = md->sha1.state[4] + e;

    return CRYPT_OK;
}

/**
   Initialize the hash state
   @param md   The hash state you wish to initialize
   @return CRYPT_OK if successful
*/
int sha1_init(hash_state* md) {
    LTC_ARGCHK(md != NULL);
    md->sha1.state[0] = 0x67452301UL;
    md->sha1.state[1] = 0xefcdab89UL;
    md->sha1.state[2] = 0x98badcfeUL;
    md->sha1.state[3] = 0x10325476UL;
    md->sha1.state[4] = 0xc3d2e1f0UL;
    md->sha1.curlen = 0;
    md->sha1.length = 0;
    return CRYPT_OK;
}

/**
   Process a block of memory though the hash
   @param md     The hash state
   @param in     The data to hash
   @param inlen  The length of the data (octets)
   @return CRYPT_OK if successful
*/
HASH_PROCESS(sha1_process, sha1_compress, sha1, 64)

/**
   Terminate the hash to get the digest
   @param md  The hash state
   @param out [out] The destination of the hash (20 bytes)
   @return CRYPT_OK if successful
*/
int sha1_done(hash_state* md, char* out) {
    int i;

    LTC_ARGCHK(md != NULL);
    LTC_ARGCHK(out != NULL);

    if (md->sha1.curlen >= sizeof(md->sha1.buf)) {
        return CRYPT_INVALID_ARG;
    }

    /* increase the length of the message */
    md->sha1.length += md->sha1.curlen * 8;

    /* append the '1' bit */
    md->sha1.buf[md->sha1.curlen++] = (char)0x80;

    /* if the length is currently above 56 bytes we append zeros
     * then compress.  Then we can fall back to padding zeros and length
     * encoding like normal.
     */
    if (md->sha1.curlen > 56) {
        while (md->sha1.curlen < 64) {
            md->sha1.buf[md->sha1.curlen++] = (char)0;
        }
        sha1_compress(md, md->sha1.buf);
        md->sha1.curlen = 0;
    }

    /* pad upto 56 bytes of zeroes */
    while (md->sha1.curlen < 56) {
        md->sha1.buf[md->sha1.curlen++] = (char)0;
    }

    /* store length */
    STORE64H(md->sha1.length, md->sha1.buf + 56);
    sha1_compress(md, md->sha1.buf);

    /* copy output */
    for (i = 0; i < 5; i++) {
        STORE32H(md->sha1.state[i], out + (4 * i));
    }

    return CRYPT_OK;
}

void hash_op(ot_op* op) {
    bool free_snapshot = true;
    char* snapshot = ot_snapshot(op);
    if (snapshot == NULL) {
        free_snapshot = false;
        snapshot = "";
    }

    hash_state md;
    sha1_init(&md);
    sha1_process(&md, snapshot, (uint32_t)strlen(snapshot));
    sha1_done(&md, (char*)&op->hash);

    if (free_snapshot) {
        free(snapshot);
    }
}
