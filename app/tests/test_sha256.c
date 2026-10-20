#include "common.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "util/sha256.h"

static void sha256_hex(const char *data, size_t chunk, char hex[65]) {
    struct sc_sha256 ctx;
    sc_sha256_init(&ctx);

    size_t len = strlen(data);
    if (!chunk) {
        chunk = len ? len : 1;
    }
    for (size_t i = 0; i < len; i += chunk) {
        size_t n = len - i < chunk ? len - i : chunk;
        sc_sha256_update(&ctx, data + i, n);
    }

    uint8_t digest[SC_SHA256_DIGEST_SIZE];
    sc_sha256_final(&ctx, digest);

    for (int i = 0; i < SC_SHA256_DIGEST_SIZE; ++i) {
        snprintf(hex + 2 * i, 3, "%02x", digest[i]);
    }
}

static void test_sha256_empty(void) {
    char hex[65];
    sha256_hex("", 0, hex);
    assert(!strcmp(hex, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495"
                        "991b7852b855"));
}

static void test_sha256_abc(void) {
    char hex[65];
    sha256_hex("abc", 0, hex);
    assert(!strcmp(hex, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410"
                        "ff61f20015ad"));
}

#define TWO_BLOCKS "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
#define TWO_BLOCKS_HEX \
    "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"

static void test_sha256_two_blocks(void) {
    char hex[65];
    sha256_hex(TWO_BLOCKS, 0, hex);
    assert(!strcmp(hex, TWO_BLOCKS_HEX));
}

static void test_sha256_chunked(void) {
    static const size_t chunks[] = {1, 3, 52};
    for (size_t i = 0; i < ARRAY_LEN(chunks); ++i) {
        char hex[65];
        sha256_hex(TWO_BLOCKS, chunks[i], hex);
        assert(!strcmp(hex, TWO_BLOCKS_HEX));
    }
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    test_sha256_empty();
    test_sha256_abc();
    test_sha256_two_blocks();
    test_sha256_chunked();
    return 0;
}
