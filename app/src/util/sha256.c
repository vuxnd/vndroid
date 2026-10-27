#include "sha256.h"

#include <libavutil/mem.h>
#include <libavutil/sha.h>

bool
sc_sha256_init(struct sc_sha256 *ctx) {
    ctx->av_sha = av_sha_alloc();
    if (!ctx->av_sha) {
        return false;
    }

    av_sha_init(ctx->av_sha, 256);
    return true;
}

void
sc_sha256_update(struct sc_sha256 *ctx, const void *data, size_t len) {
    av_sha_update(ctx->av_sha, data, len);
}

void
sc_sha256_final(struct sc_sha256 *ctx, uint8_t digest[SC_SHA256_DIGEST_SIZE]) {
    av_sha_final(ctx->av_sha, digest);
    sc_sha256_destroy(ctx);
}

void
sc_sha256_destroy(struct sc_sha256 *ctx) {
    av_free(ctx->av_sha);
    ctx->av_sha = NULL;
}
