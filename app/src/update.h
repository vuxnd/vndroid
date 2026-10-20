#ifndef SC_UPDATE_H
#define SC_UPDATE_H

#include "common.h"

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL_events.h>

#include "util/sha256.h"

#define SC_UPDATE_REPO "vuxnd/vndroid"
#define SC_UPDATE_LATEST_URL \
    "https://github.com/" SC_UPDATE_REPO "/releases/latest"
// followed by "<tag>/<asset>"
#define SC_UPDATE_DOWNLOAD_URL_PREFIX \
    "https://github.com/" SC_UPDATE_REPO "/releases/download/"
#define SC_UPDATE_SUMS_NAME "SHA256SUMS.txt"

// Helper script, next to the executable
#ifdef _WIN32
# define SC_UPDATE_HELPER "vndroid-update.bat"
#else
# define SC_UPDATE_HELPER "vndroid-update"
#endif

// Release asset target of this build. Left undefined on other platforms,
// which disables updates.
#if defined(__APPLE__)
# if defined(__aarch64__)
#  define SC_UPDATE_TARGET "macos-aarch64"
# elif defined(__x86_64__)
#  define SC_UPDATE_TARGET "macos-x86_64"
# endif
#elif defined(_WIN32)
# if defined(_M_ARM64) || defined(__aarch64__)
#  define SC_UPDATE_TARGET "winarm64"
# elif defined(_WIN64)
#  define SC_UPDATE_TARGET "win64"
# else
#  define SC_UPDATE_TARGET "win32"
# endif
#endif

/* Pure helpers (update_util.c) */

// Versions: optional leading 'v', then 1 to 4 numeric components (1 to 9
// digits each) separated by '.', nothing else. Missing components are 0.
// Return false if a or b is invalid; otherwise store <0, 0 or >0 in *cmp.
bool
sc_update_version_compare(const char *a, const char *b, int *cmp);

// Extract the tag from the redirect URL printed for /releases/latest:
// ".../releases/tag/v0.2.0" (trailing CR/LF ignored) -> "v0.2.0".
// Return NULL if "/releases/tag/" is absent, the tag is empty, longer than 64
// chars, or contains chars other than [A-Za-z0-9._-]. Free with free().
char *
sc_update_parse_tag_url(const char *url);

// "vndroid-<target>-<tag>.dmg" if target starts with "macos-", else ".zip".
// Free with free().
char *
sc_update_asset_name(const char *target, const char *tag);

// Find the line for exactly `name` in SHA256SUMS.txt content. Lines are
// "<64 hex digits>" + " " + (" " or "*") + "<name>", ending in LF or CRLF;
// hex is case-insensitive. Return false if absent or malformed.
bool
sc_update_find_checksum(const char *sums, const char *name,
                        uint8_t digest[SC_SHA256_DIGEST_SIZE]);

// "/Applications/VNdroid.app/Contents/MacOS/vndroid" -> "/Applications/VNdroid.app".
// NULL unless the path ends with "/Contents/MacOS/vndroid", the rest ends
// with ".app", and the path does not contain "/AppTranslocation/".
// Free with free().
char *
sc_update_bundle_path(const char *exe_path);

// Part before the last '/' or '\\': "C:\\VNdroid\\vndroid.exe" -> "C:\\VNdroid".
// NULL if there is no separator. Free with free().
char *
sc_update_parent_dir(const char *path);

/* Update flow (update.c), main thread only */

// Call once before the event loop
bool
sc_update_init(void);

// Start a check in a background thread. manual: show "up to date" and errors
// in dialogs (an automatic check only logs them). No-op with a log line if
// sc_update_init() was not called (OTG mode) or a job is running; a manual
// request during a running check upgrades it to manual.
void
sc_update_check(bool manual);

// Handle SC_EVENT_UPDATE_CHECKED and SC_EVENT_UPDATE_DOWNLOADED
void
sc_update_handle_event(const SDL_Event *event);

// Call after all other cleanup: interrupt and join the worker. If the user
// chose "Restart now", start the helper detached. Then release everything.
void
sc_update_destroy(void);

#endif
