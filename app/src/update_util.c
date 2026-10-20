#include "update.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/log.h"

#define VERSION_MAX_COMPONENTS 4
#define VERSION_MAX_DIGITS 9
#define TAG_MAX_LEN 64

static bool
parse_version(const char *s, long out[VERSION_MAX_COMPONENTS]) {
    if (*s == 'v') {
        ++s;
    }

    int count = 0;
    for (;;) {
        int digits = 0;
        long value = 0;
        while (*s >= '0' && *s <= '9') {
            value = value * 10 + (*s - '0');
            ++digits;
            ++s;
            if (digits > VERSION_MAX_DIGITS) {
                return false;
            }
        }
        if (!digits) {
            return false;
        }
        out[count++] = value;

        if (*s == '\0') {
            break;
        }
        if (*s != '.' || count == VERSION_MAX_COMPONENTS) {
            return false;
        }
        ++s;
    }

    while (count < VERSION_MAX_COMPONENTS) {
        out[count++] = 0;
    }
    return true;
}

bool
sc_update_version_compare(const char *a, const char *b, int *cmp) {
    long va[VERSION_MAX_COMPONENTS];
    long vb[VERSION_MAX_COMPONENTS];
    if (!parse_version(a, va) || !parse_version(b, vb)) {
        return false;
    }

    *cmp = 0;
    for (int i = 0; i < VERSION_MAX_COMPONENTS; ++i) {
        if (va[i] != vb[i]) {
            *cmp = va[i] < vb[i] ? -1 : 1;
            break;
        }
    }
    return true;
}

char *
sc_update_parse_tag_url(const char *url) {
    static const char MARKER[] = "/releases/tag/";

    const char *p = strstr(url, MARKER);
    if (!p) {
        return NULL;
    }
    p += sizeof(MARKER) - 1;

    size_t len = strlen(p);
    while (len && (p[len - 1] == '\r' || p[len - 1] == '\n')) {
        --len;
    }
    if (!len || len > TAG_MAX_LEN) {
        return NULL;
    }

    for (size_t i = 0; i < len; ++i) {
        unsigned char c = p[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            return NULL;
        }
    }

    char *tag = malloc(len + 1);
    if (!tag) {
        LOG_OOM();
        return NULL;
    }
    memcpy(tag, p, len);
    tag[len] = '\0';
    return tag;
}

char *
sc_update_asset_name(const char *target, const char *tag) {
    const char *ext = !strncmp(target, "macos-", 6) ? "dmg" : "zip";
    char *name;
    if (asprintf(&name, "vndroid-%s-%s.%s", target, tag, ext) == -1) {
        LOG_OOM();
        return NULL;
    }
    return name;
}

static int
hex_value(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

bool
sc_update_find_checksum(const char *sums, const char *name,
                        uint8_t digest[SC_SHA256_DIGEST_SIZE]) {
    size_t name_len = strlen(name);
    size_t hex_len = 2 * SC_SHA256_DIGEST_SIZE;

    while (*sums) {
        const char *eol = strchr(sums, '\n');
        size_t len = eol ? (size_t) (eol - sums) : strlen(sums);
        const char *next = sums + len + (eol ? 1 : 0);
        if (len && sums[len - 1] == '\r') {
            --len;
        }

        if (len == hex_len + 2 + name_len
                && !memcmp(sums + hex_len + 2, name, name_len)) {
            if (sums[hex_len] != ' '
                    || (sums[hex_len + 1] != ' ' && sums[hex_len + 1] != '*')) {
                return false;
            }
            for (size_t i = 0; i < SC_SHA256_DIGEST_SIZE; ++i) {
                int hi = hex_value(sums[2 * i]);
                int lo = hex_value(sums[2 * i + 1]);
                if (hi < 0 || lo < 0) {
                    return false;
                }
                digest[i] = hi << 4 | lo;
            }
            return true;
        }

        sums = next;
    }

    return false;
}

char *
sc_update_bundle_path(const char *exe_path) {
    static const char EXE_SUFFIX[] = "/Contents/MacOS/vndroid";
    static const char BUNDLE_SUFFIX[] = ".app";

    size_t len = strlen(exe_path);
    size_t exe_suffix_len = sizeof(EXE_SUFFIX) - 1;
    size_t bundle_suffix_len = sizeof(BUNDLE_SUFFIX) - 1;
    if (len < exe_suffix_len
            || strcmp(exe_path + len - exe_suffix_len, EXE_SUFFIX)) {
        return NULL;
    }

    size_t bundle_len = len - exe_suffix_len;
    if (bundle_len < bundle_suffix_len
            || memcmp(exe_path + bundle_len - bundle_suffix_len,
                      BUNDLE_SUFFIX, bundle_suffix_len)) {
        return NULL;
    }

    if (strstr(exe_path, "/AppTranslocation/")) {
        return NULL;
    }

    char *bundle = malloc(bundle_len + 1);
    if (!bundle) {
        LOG_OOM();
        return NULL;
    }
    memcpy(bundle, exe_path, bundle_len);
    bundle[bundle_len] = '\0';
    return bundle;
}

char *
sc_update_parent_dir(const char *path) {
    const char *sep = NULL;
    for (const char *p = path; *p; ++p) {
        if (*p == '/' || *p == '\\') {
            sep = p;
        }
    }
    if (!sep) {
        return NULL;
    }

    size_t len = sep - path;
    char *dir = malloc(len + 1);
    if (!dir) {
        LOG_OOM();
        return NULL;
    }
    memcpy(dir, path, len);
    dir[len] = '\0';
    return dir;
}
