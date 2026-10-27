#include "update.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <SDL3/SDL.h>
#ifdef _WIN32
# include <io.h>
#else
# include <unistd.h>
#endif

#include "events.h"
#include "util/env.h"
#include "util/file.h"
#include "util/intr.h"
#include "util/log.h"
#include "util/process.h"
#include "util/process_intr.h"
#include "util/str.h"
#include "util/thread.h"

#ifdef SC_UPDATE_TARGET
# define UPDATE_TARGET SC_UPDATE_TARGET
#else
# define UPDATE_TARGET NULL
#endif

#ifdef _WIN32
# define NULDEV "NUL"
#else
# define NULDEV "/dev/null"
#endif

#define CHUNK_SIZE (64 * 1024)
#define SUMS_MAX_SIZE (8 * 1024)

enum update_job {
    JOB_CHECK,
    JOB_DOWNLOAD,
};

static struct {
    struct sc_intr intr;
    sc_thread thread;
    bool thread_running;
    bool initialized;
    enum update_job job;
    bool manual;
    bool newer;
    char *tag;
    const char *error; // string literals only
    char *asset_path;
    char *helper_path;
    char *target_dir;
    bool install_pending;
    char pid_str[16];
    SDL_Window *window;
    int argc;
    char *const *argv;
} update;

/* Worker thread */

static void
set_error(const char *error) {
    // Keep the first (most specific) error
    if (!update.error) {
        update.error = error;
    }
}

static void
file_remove(const char *path) {
#ifdef _WIN32
    wchar_t *wpath = sc_str_to_wchars(path);
    if (!wpath) {
        LOG_OOM();
        return;
    }
    _wremove(wpath);
    free(wpath);
#else
    remove(path);
#endif
}

// Create the file exclusively, the path is predictable
static FILE *
file_open(const char *path) {
    // Remove any stale file, errors are ignored
    file_remove(path);
#ifdef _WIN32
    wchar_t *wpath = sc_str_to_wchars(path);
    if (!wpath) {
        LOG_OOM();
        return NULL;
    }
    int fd = _wopen(wpath, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY,
                    _S_IREAD | _S_IWRITE);
    free(wpath);
    if (fd < 0) {
        return NULL;
    }
    FILE *f = _fdopen(fd, "wb");
#else
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        return NULL;
    }
    FILE *f = fdopen(fd, "wb");
#endif
    if (!f) {
#ifdef _WIN32
        _close(fd);
#else
        close(fd);
#endif
    }
    return f;
}

static bool
curl_start(const char *const argv[], sc_pid *pid, sc_pipe *pout) {
    enum sc_process_result r =
        sc_process_execute_p(argv, pid, 0, NULL, pout, NULL);
    if (r == SC_PROCESS_ERROR_MISSING_BINARY) {
        set_error("curl not found");
        return false;
    }
    return r == SC_PROCESS_SUCCESS;
}

// Wait for curl and close it. With `abort`, kill it first. Return true if it
// exited with code 0.
static bool
curl_finish(sc_pid pid, bool abort) {
    if (abort || sc_intr_is_interrupted(&update.intr)) {
        // Failed or already interrupted
        sc_process_terminate(pid);
        sc_process_close(pid);
        return false;
    }

    const char *name = "curl";
    return sc_process_check_success_intr(&update.intr, pid, name, true);
}

// Run curl and store its whole output in buf as a string
static bool
curl_capture(const char *const argv[], char *buf, size_t size) {
    sc_pid pid;
    sc_pipe pout;
    if (!curl_start(argv, &pid, &pout)) {
        return false;
    }

    ssize_t r = sc_pipe_read_all_intr(&update.intr, pid, pout, buf, size - 1);
    sc_pipe_close(pout);

    // A full buffer means the output may be truncated
    bool ok = r >= 0 && (size_t) r < size - 1;
    if (!curl_finish(pid, !ok) || !ok) {
        return false;
    }

    buf[r] = '\0';
    return true;
}

// Download url to path, and compute the SHA-256 of what was written
static bool
curl_download(const char *url, const char *path,
              uint8_t digest[SC_SHA256_DIGEST_SIZE]) {
    const char *const argv[] = {"curl", "-q", "-fsSL", "--connect-timeout",
                                "15", "--speed-time", "60", "--speed-limit",
                                "1", url, NULL};
    char *buf = malloc(CHUNK_SIZE);
    if (!buf) {
        LOG_OOM();
        return false;
    }

    FILE *f = file_open(path);
    if (!f) {
        LOGE("Could not open %s", path);
        free(buf);
        return false;
    }

    sc_pid pid;
    sc_pipe pout;
    if (!curl_start(argv, &pid, &pout)) {
        fclose(f);
        free(buf);
        return false;
    }

    struct sc_sha256 sha;
    bool ok = sc_sha256_init(&sha);
    if (!ok) {
        LOG_OOM();
    }

    while (ok) {
        ssize_t r = sc_pipe_read_intr(&update.intr, pid, pout, buf, CHUNK_SIZE);
        if (r <= 0) {
            // EOF. On Windows, a closed pipe reads as an error. A truncated
            // download is caught by the curl exit code and the SHA-256.
            break;
        }
        if (fwrite(buf, 1, r, f) != (size_t) r) {
            ok = false;
            break;
        }
        sc_sha256_update(&sha, buf, r);
    }
    sc_pipe_close(pout);
    free(buf);

    ok = curl_finish(pid, !ok) && ok;
    if (fclose(f)) {
        ok = false;
    }
    if (ok) {
        sc_sha256_final(&sha, digest);
    }
    sc_sha256_destroy(&sha);
    return ok;
}

static void
check_job(void) {
    const char *const argv[] = {"curl", "-q", "-s", "--connect-timeout", "15",
                                "--max-time", "30", "-o", NULDEV,
                                "-w", "%{redirect_url}",
                                SC_UPDATE_LATEST_URL, NULL};
    char url[512];
    if (!curl_capture(argv, url, sizeof(url))) {
        set_error("Could not reach GitHub");
        return;
    }

    update.tag = sc_update_parse_tag_url(url);
    if (!update.tag) {
        set_error("No release found");
        return;
    }

    int cmp;
    if (!sc_update_version_compare(VNDROID_VERSION, update.tag, &cmp)) {
        set_error("Unrecognized release version");
        return;
    }
    update.newer = cmp < 0;
}

static void
download_job(void) {
    char *asset = NULL;
    char *url = NULL;
    char *name = NULL;
    char *path = NULL;

    asset = sc_update_asset_name(UPDATE_TARGET, update.tag);
    if (!asset) {
        set_error("Download failed");
        goto end;
    }

    // Look up the expected checksum
    if (asprintf(&url, SC_UPDATE_DOWNLOAD_URL_PREFIX "%s/%s", update.tag,
                 SC_UPDATE_SUMS_NAME) == -1) {
        url = NULL;
        LOG_OOM();
        set_error("Download failed");
        goto end;
    }
    const char *const sums_argv[] = {"curl", "-q", "-fsSL", "--connect-timeout",
                                     "15", "--max-time", "60", url, NULL};
    char sums[SUMS_MAX_SIZE];
    uint8_t expected[SC_SHA256_DIGEST_SIZE];
    if (!curl_capture(sums_argv, sums, sizeof(sums))
            || !sc_update_find_checksum(sums, asset, expected)) {
        set_error("No checksum for this platform");
        goto end;
    }

    // Download the asset to the temp dir, prefixed with the PID so that other
    // instances never touch this file
    if (asprintf(&name, "%s-%s", update.pid_str, asset) == -1) {
        name = NULL;
        LOG_OOM();
        set_error("Download failed");
        goto end;
    }
#ifdef _WIN32
    char *tmp = sc_get_env("TEMP");
#else
    char *tmp = sc_get_env("TMPDIR");
    if (!tmp) {
        tmp = strdup("/tmp");
    }
#endif
    if (!tmp) {
        set_error("Download failed");
        goto end;
    }
    path = sc_file_build_path(tmp, name);
    free(tmp);
    free(url);
    url = NULL;
    if (!path || asprintf(&url, SC_UPDATE_DOWNLOAD_URL_PREFIX "%s/%s",
                          update.tag, asset) == -1) {
        url = NULL;
        LOG_OOM();
        set_error("Download failed");
        goto end;
    }

    LOGI("Downloading %s", url);
    uint8_t digest[SC_SHA256_DIGEST_SIZE];
    if (!curl_download(url, path, digest)) {
        file_remove(path);
        set_error("Download failed");
        goto end;
    }
    if (memcmp(digest, expected, sizeof(digest))) {
        file_remove(path);
        set_error("Checksum mismatch");
        goto end;
    }

    update.asset_path = path;
    path = NULL;

end:
    free(asset);
    free(url);
    free(name);
    free(path);
}

static int
run_update(void *data) {
    (void) data;

    // Pushing the event is the last action: the main thread joins this thread
    // on receiving it
    if (update.job == JOB_CHECK) {
        check_job();
        sc_push_event(SC_EVENT_UPDATE_CHECKED);
    } else {
        download_job();
        sc_push_event(SC_EVENT_UPDATE_DOWNLOADED);
    }
    return 0;
}

static bool
start_job(enum update_job job) {
    assert(!update.thread_running);

    update.job = job;
    update.error = NULL;
    if (job == JOB_CHECK) {
        free(update.tag);
        update.tag = NULL;
        update.newer = false;
    }

    bool ok = sc_thread_create(&update.thread, run_update, "vndroid-update",
                               NULL);
    if (!ok) {
        LOGE("Could not start update thread");
        return false;
    }
    update.thread_running = true;
    return true;
}

/* Main thread */

void
sc_update_set_args(int argc, char *const argv[]) {
    update.argc = argc;
    update.argv = argv;
}

bool
sc_update_init(SDL_Window *window) {
    if (!sc_intr_init(&update.intr)) {
        return false;
    }
#ifdef _WIN32
    snprintf(update.pid_str, sizeof(update.pid_str), "%lu",
             GetCurrentProcessId());
#else
    snprintf(update.pid_str, sizeof(update.pid_str), "%ld", (long) getpid());
#endif
    update.window = window;
    update.initialized = true;
    return true;
}

// Release a captured mouse so that the dialog can be used. It stays released:
// a click in the window captures it again.
static void
release_mouse(void) {
    if (update.window) {
        SDL_SetWindowRelativeMouseMode(update.window, false);
    }
}

static void
show_message(bool error, const char *message) {
    release_mouse();
    SDL_ShowSimpleMessageBox(error ? SDL_MESSAGEBOX_ERROR
                                   : SDL_MESSAGEBOX_INFORMATION,
                             "VNdroid", message, update.window);
}

static void
show_failure(const char *what, const char *reason) {
    char *message;
    if (asprintf(&message, "%s: %s", what, reason) == -1) {
        LOG_OOM();
        return;
    }
    show_message(true, message);
    free(message);
}

// Return true if the user clicked `yes`; Return, Escape and errors mean no
static bool
ask(const char *title, const char *message, const char *yes, const char *no) {
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT
            | SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, no},
        {0, 1, yes},
    };
    const SDL_MessageBoxData data = {
        .flags = SDL_MESSAGEBOX_INFORMATION,
        .window = update.window,
        .title = title,
        .message = message,
        .numbuttons = ARRAY_LEN(buttons),
        .buttons = buttons,
    };
    int id = 0;
    release_mouse();
    return SDL_ShowMessageBox(&data, &id) && id == 1;
}

void
sc_update_check(bool manual) {
    if (!update.initialized) {
        LOGI("Update check unavailable");
        return;
    }

    if (!UPDATE_TARGET) {
        if (manual) {
            show_message(false, "Updates are not available for this platform");
        }
        return;
    }

    if (update.thread_running) {
        if (manual && update.job == JOB_CHECK) {
            update.manual = true; // show the result of the running check
        } else {
            LOGI("An update job is already running");
        }
        return;
    }

    // Discard the file of a previous download that was not installed
    if (update.asset_path) {
        file_remove(update.asset_path);
        free(update.asset_path);
        update.asset_path = NULL;
    }

    update.manual = manual;
    if (!start_job(JOB_CHECK) && manual) {
        show_failure("Update check failed", "could not start the check");
    }
}

// Check that the running install can be replaced and find the helper. Return
// NULL on success, else the message to show.
static const char *
prepare_install(void) {
    const char *not_installed =
        "Update is only available for the installed app";
    char *exe = sc_file_get_executable_path();
    if (!exe) {
        return not_installed;
    }
#ifdef __APPLE__
    char *target_dir = sc_update_bundle_path(exe);
#else
    char *target_dir = sc_file_get_parent_dir(exe);
#endif
    free(exe);

    char *helper_path = sc_file_get_local_path(SC_UPDATE_HELPER);
    if (!target_dir || !helper_path || !sc_file_is_regular(helper_path)) {
        free(target_dir);
        free(helper_path);
        return not_installed;
    }

#ifdef __APPLE__
    // The helper replaces the bundle: check before the download
    char *parent_dir = sc_file_get_parent_dir(target_dir);
    bool writable = parent_dir && !access(parent_dir, W_OK);
    free(parent_dir);
    if (!writable) {
        free(target_dir);
        free(helper_path);
        return "Update needs write access to the folder containing VNdroid.app";
    }
#endif

    free(update.target_dir);
    free(update.helper_path);
    update.target_dir = target_dir;
    update.helper_path = helper_path;
    return NULL;
}

static void
handle_checked(void) {
    if (update.error) {
        if (update.manual) {
            show_failure("Update check failed", update.error);
        } else {
            LOGW("Update check failed: %s", update.error);
        }
        return;
    }

    if (!update.newer) {
        if (update.manual) {
            show_message(false, "VNdroid is up to date");
        } else {
            LOGI("VNdroid is up to date");
        }
        return;
    }

    LOGI("VNdroid %s is available", update.tag);
    char *message;
    if (asprintf(&message, "VNdroid %s is available", update.tag) == -1) {
        LOG_OOM();
        return;
    }
    bool accepted = ask("VNdroid", message, "Update", "Later");
    free(message);
    if (!accepted) {
        return;
    }

    const char *install_error = prepare_install();
    if (install_error) {
        show_message(true, install_error);
        return;
    }
    if (!start_job(JOB_DOWNLOAD)) {
        show_failure("Update failed", "could not start the download");
    }
}

static void
handle_downloaded(void) {
    if (update.error) {
        show_failure("Update failed", update.error);
        return;
    }

    char *message;
    if (asprintf(&message, "VNdroid %s will close and restart.",
                 update.tag) == -1) {
        LOG_OOM();
        return;
    }
    bool restart = ask("Update downloaded", message, "Restart now", "Later");
    free(message);

    if (restart) {
        update.install_pending = true;
        sc_push_event(SDL_EVENT_QUIT);
    } else {
        file_remove(update.asset_path);
        free(update.asset_path);
        update.asset_path = NULL;
    }
}

void
sc_update_handle_event(const SDL_Event *event) {
    assert(update.initialized);

    if (update.thread_running) {
        sc_thread_join(&update.thread, NULL);
        update.thread_running = false;
    }

    if (event->type == SC_EVENT_UPDATE_CHECKED) {
        handle_checked();
    } else {
        assert(event->type == SC_EVENT_UPDATE_DOWNLOADED);
        handle_downloaded();
    }
}

bool
sc_update_is_downloading(void) {
    return update.thread_running && update.job == JOB_DOWNLOAD;
}

// Start the helper, which waits for this process to exit then installs
static bool
start_helper(void) {
    // The helper restarts VNdroid with the arguments of this process
    int nargs = update.argc > 1 ? update.argc - 1 : 0;
    const char **argv = malloc((5 + nargs) * sizeof(*argv));
    if (!argv) {
        LOG_OOM();
        return false;
    }
    argv[0] = update.helper_path;
    argv[1] = update.asset_path;
    argv[2] = update.target_dir;
    argv[3] = update.pid_str;
    for (int i = 0; i < nargs; ++i) {
        argv[4 + i] = update.argv[1 + i];
    }
    argv[4 + nargs] = NULL;

    // On Windows, a console of its own: Ctrl+C or closing the console of
    // VNdroid must not interrupt the install
    unsigned flags = SC_PROCESS_NO_STDOUT | SC_PROCESS_NO_STDERR
                   | SC_PROCESS_NO_WINDOW;

    LOGI("Starting %s", update.helper_path);
    sc_pid pid;
    enum sc_process_result r = sc_process_execute(argv, &pid, flags);
    free(argv);
    if (r != SC_PROCESS_SUCCESS) {
        LOGE("Could not start %s", update.helper_path);
        return false;
    }
#ifdef _WIN32
    sc_process_close(pid);
#endif
    // On Unix, never wait: the helper waits for this process to exit
    return true;
}

void
sc_update_destroy(void) {
    if (!update.initialized) {
        return;
    }

    if (update.thread_running) {
        sc_intr_interrupt(&update.intr);
        sc_thread_join(&update.thread, NULL);
        update.thread_running = false;
    }

    if (update.install_pending && update.asset_path && update.helper_path
            && update.target_dir) {
        if (!start_helper()) {
            file_remove(update.asset_path);
        }
    } else if (update.asset_path) {
        // Downloaded but not installed
        file_remove(update.asset_path);
    }

    free(update.tag);
    free(update.asset_path);
    free(update.helper_path);
    free(update.target_dir);
    sc_intr_destroy(&update.intr);
    memset(&update, 0, sizeof(update));
}
