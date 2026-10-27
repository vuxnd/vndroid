#include "common.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "update.h"

static int compare(const char *a, const char *b) {
    int cmp = 12345;
    bool ok = sc_update_version_compare(a, b, &cmp);
    assert(ok);
    return cmp;
}

static void test_version_compare(void) {
    assert(compare("0.1", "v0.1.0") == 0);
    assert(compare("0.1", "v0.2.0") < 0);
    assert(compare("v0.10.0", "0.9") > 0);
    assert(compare("1.0.0.1", "1") > 0);
}

static void test_version_compare_invalid(void) {
    static const char *const invalid[] = {
        "", "v", "0.1-rc1", "1..2", "a.b", "1.2.3.4.5",
    };
    for (size_t i = 0; i < ARRAY_LEN(invalid); ++i) {
        int cmp = 12345;
        assert(!sc_update_version_compare(invalid[i], "1.0", &cmp));
        assert(!sc_update_version_compare("1.0", invalid[i], &cmp));
    }
}

static void test_parse_tag_url(void) {
    char *tag = sc_update_parse_tag_url(
        "https://github.com/vuxnd/vndroid/releases/tag/v0.2.0");
    assert(tag);
    assert(!strcmp(tag, "v0.2.0"));
    free(tag);

    tag = sc_update_parse_tag_url(
        "https://github.com/vuxnd/vndroid/releases/tag/v0.2.0\r\n");
    assert(tag);
    assert(!strcmp(tag, "v0.2.0"));
    free(tag);
}

static void test_parse_tag_url_invalid(void) {
    static const char *const invalid[] = {
        "https://github.com/vuxnd/vndroid/releases",
        "",
        "https://github.com/vuxnd/vndroid/releases/tag/",
        "https://github.com/vuxnd/vndroid/releases/tag/v0.2.0/x",
        "https://github.com/vuxnd/vndroid/releases/tag/v0;x",
    };
    for (size_t i = 0; i < ARRAY_LEN(invalid); ++i) {
        assert(!sc_update_parse_tag_url(invalid[i]));
    }
}

static void test_asset_name(void) {
    char *name = sc_update_asset_name("macos-aarch64", "v0.2.0");
    assert(name);
    assert(!strcmp(name, "vndroid-macos-aarch64-v0.2.0.dmg"));
    free(name);

    name = sc_update_asset_name("win64", "v0.2.0");
    assert(name);
    assert(!strcmp(name, "vndroid-win64-v0.2.0.zip"));
    free(name);
}

#define HEX_A "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
#define HEX_B "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff"
#define HEX_C "fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210"

static const uint8_t DIGEST_A[SC_SHA256_DIGEST_SIZE] = {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static const uint8_t DIGEST_B[SC_SHA256_DIGEST_SIZE] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
};

#define SUMS \
    HEX_A "  vndroid-macos-aarch64-v0.2.0.dmg\n" \
    HEX_B "  vndroid-win64-v0.2.0.zip\n" \
    HEX_C "  vndroid-win32-v0.2.0.zip\n"

static void test_find_checksum(void) {
    uint8_t digest[SC_SHA256_DIGEST_SIZE];

    bool ok = sc_update_find_checksum(SUMS, "vndroid-macos-aarch64-v0.2.0.dmg",
                                      digest);
    assert(ok);
    assert(!memcmp(digest, DIGEST_A, sizeof(digest)));

    ok = sc_update_find_checksum(SUMS, "vndroid-win64-v0.2.0.zip", digest);
    assert(ok);
    assert(!memcmp(digest, DIGEST_B, sizeof(digest)));
}

static void test_find_checksum_prefix(void) {
    uint8_t digest[SC_SHA256_DIGEST_SIZE];
    assert(!sc_update_find_checksum(SUMS, "vndroid-win64-v0.2.0.zi", digest));
    assert(!sc_update_find_checksum(SUMS, "vndroid-win64-v0.2.0.zipx",
                                    digest));
}

static void test_find_checksum_crlf(void) {
    const char *sums =
        HEX_A "  vndroid-macos-aarch64-v0.2.0.dmg\r\n"
        HEX_B "  vndroid-win64-v0.2.0.zip\r\n";
    uint8_t digest[SC_SHA256_DIGEST_SIZE];

    bool ok = sc_update_find_checksum(sums, "vndroid-win64-v0.2.0.zip", digest);
    assert(ok);
    assert(!memcmp(digest, DIGEST_B, sizeof(digest)));
}

static void test_find_checksum_star(void) {
    const char *sums = HEX_A " *vndroid-win64-v0.2.0.zip\n";
    uint8_t digest[SC_SHA256_DIGEST_SIZE];

    bool ok = sc_update_find_checksum(sums, "vndroid-win64-v0.2.0.zip", digest);
    assert(ok);
    assert(!memcmp(digest, DIGEST_A, sizeof(digest)));
}

static void test_find_checksum_uppercase(void) {
    const char *sums =
        "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"
        "  vndroid-win64-v0.2.0.zip";
    uint8_t digest[SC_SHA256_DIGEST_SIZE];

    // no trailing newline on the last line
    bool ok = sc_update_find_checksum(sums, "vndroid-win64-v0.2.0.zip", digest);
    assert(ok);
    assert(!memcmp(digest, DIGEST_A, sizeof(digest)));
}

static void test_find_checksum_invalid(void) {
    uint8_t digest[SC_SHA256_DIGEST_SIZE];

    // 63 hex digits
    const char *short_hex =
        "0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcde"
        "  vndroid-win64-v0.2.0.zip\n";
    assert(!sc_update_find_checksum(short_hex, "vndroid-win64-v0.2.0.zip",
                                    digest));

    // not a hex digit
    const char *bad_hex =
        "g123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef"
        "  vndroid-win64-v0.2.0.zip\n";
    assert(!sc_update_find_checksum(bad_hex, "vndroid-win64-v0.2.0.zip",
                                    digest));

    // missing name
    assert(!sc_update_find_checksum(SUMS, "vndroid-winarm64-v0.2.0.zip",
                                    digest));
    assert(!sc_update_find_checksum("", "vndroid-win64-v0.2.0.zip", digest));
}

static void test_bundle_path(void) {
    char *path = sc_update_bundle_path(
        "/Applications/VNdroid.app/Contents/MacOS/vndroid");
    assert(path);
    assert(!strcmp(path, "/Applications/VNdroid.app"));
    free(path);
}

static void test_bundle_path_invalid(void) {
    assert(!sc_update_bundle_path("/usr/local/bin/vndroid"));
    assert(!sc_update_bundle_path("/Contents/MacOS/vndroid"));
    assert(!sc_update_bundle_path(
        "/private/var/folders/xx/AppTranslocation/ABC/d/VNdroid.app"
        "/Contents/MacOS/vndroid"));
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    test_version_compare();
    test_version_compare_invalid();
    test_parse_tag_url();
    test_parse_tag_url_invalid();
    test_asset_name();
    test_find_checksum();
    test_find_checksum_prefix();
    test_find_checksum_crlf();
    test_find_checksum_star();
    test_find_checksum_uppercase();
    test_find_checksum_invalid();
    test_bundle_path();
    test_bundle_path_invalid();
    return 0;
}
