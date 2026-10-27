#ifndef SC_SHA256_H
#define SC_SHA256_H

#include "common.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SC_SHA256_DIGEST_SIZE 32

struct AVSHA;

struct sc_sha256 {
    struct AVSHA *av_sha;
};

// Return false on out-of-memory
bool
sc_sha256_init(struct sc_sha256 *ctx);

void
sc_sha256_update(struct sc_sha256 *ctx, const void *data, size_t len);

// Write the digest and release the context
void
sc_sha256_final(struct sc_sha256 *ctx, uint8_t digest[SC_SHA256_DIGEST_SIZE]);

// Release the context without computing the digest. Safe after
// sc_sha256_final() or a failed sc_sha256_init().
void
sc_sha256_destroy(struct sc_sha256 *ctx);

#endif
