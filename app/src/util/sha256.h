#ifndef SC_SHA256_H
#define SC_SHA256_H

#include "common.h"

#include <stddef.h>
#include <stdint.h>

#define SC_SHA256_DIGEST_SIZE 32

struct sc_sha256 {
    uint32_t state[8];
    uint64_t length; // total bytes hashed
    uint8_t buffer[64];
    size_t buffer_len;
};

void
sc_sha256_init(struct sc_sha256 *ctx);

void
sc_sha256_update(struct sc_sha256 *ctx, const void *data, size_t len);

void
sc_sha256_final(struct sc_sha256 *ctx, uint8_t digest[SC_SHA256_DIGEST_SIZE]);

#endif
