#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#ifdef HAVE_V4L2
# include <libavdevice/avdevice.h>
#endif
#include <SDL3/SDL.h>

#include "cli.h"
#include "events.h"
#include "options.h"
#include "vndroid.h"
#ifdef HAVE_USB
# include "usb/vndroid_otg.h"
#endif
#include "util/log.h"
#include "util/net.h"
#include "util/term.h"
#include "version.h"

#ifdef _WIN32
#include <windows.h>
#include "util/str.h"
#endif

static int
main_vndroid(int argc, char *argv[]) {
#ifdef _WIN32
    // disable buffering, we want logs immediately
    // even line buffering (setvbuf() with mode _IOLBF) is not sufficient
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);
#endif

    printf("VNdroid " VNDROID_VERSION "\n");

    struct vndroid_cli_args args = {
        .opts = vndroid_options_default,
        .help = false,
        .version = false,
        .pause_on_exit = SC_PAUSE_ON_EXIT_UNDEFINED,
    };

#ifndef NDEBUG
    args.opts.log_level = SC_LOG_LEVEL_DEBUG;
#endif

    enum vndroid_exit_code ret;

    bool term_title_saved = false;

    if (!vndroid_parse_args(&args, argc, argv)) {
        ret = VNDROID_EXIT_FAILURE;
        goto end;
    }

    sc_set_log_level(args.opts.log_level);

    if (args.opts.update_terminal_title) {
        sc_term_save_title();
        sc_term_set_title("VNdroid");
        term_title_saved = true;
    }

    if (args.help) {
        vndroid_print_usage(argv[0]);
        ret = VNDROID_EXIT_SUCCESS;
        goto end;
    }

    if (args.version) {
        vndroid_print_version();
        ret = VNDROID_EXIT_SUCCESS;
        goto end;
    }

#ifdef HAVE_V4L2
    if (args.opts.v4l2_device) {
        avdevice_register_all();
    }
#endif

    if (!net_init()) {
        ret = VNDROID_EXIT_FAILURE;
        goto end;
    }

    sc_log_configure();

    if (!sc_main_thread_init()) {
        ret = VNDROID_EXIT_FAILURE;
        goto net_cleanup;
    }

#ifdef HAVE_USB
    ret = args.opts.otg ? vndroid_otg(&args.opts) : vndroid(&args.opts);
#else
    ret = vndroid(&args.opts);
#endif

    sc_main_thread_destroy();

net_cleanup:
    net_cleanup();

end:
    if (args.pause_on_exit == SC_PAUSE_ON_EXIT_TRUE ||
            (args.pause_on_exit == SC_PAUSE_ON_EXIT_IF_ERROR &&
                ret != VNDROID_EXIT_SUCCESS)) {
        printf("Press Enter to continue...\n");
        getchar();
    }

    if (term_title_saved) {
        sc_term_set_title(""); // fallback if restore is ignored
        sc_term_restore_title();
    }

    return ret;
}

int
main(int argc, char *argv[]) {
#ifndef _WIN32
    return main_vndroid(argc, argv);
#else
    (void) argc;
    (void) argv;
    int wargc;
    wchar_t **wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
    if (!wargv) {
        LOG_OOM();
        return VNDROID_EXIT_FAILURE;
    }

    char **argv_utf8 = malloc((wargc + 1) * sizeof(*argv_utf8));
    if (!argv_utf8) {
        LOG_OOM();
        LocalFree(wargv);
        return VNDROID_EXIT_FAILURE;
    }

    argv_utf8[wargc] = NULL;

    for (int i = 0; i < wargc; ++i) {
        argv_utf8[i] = sc_str_from_wchars(wargv[i]);
        if (!argv_utf8[i]) {
            LOG_OOM();
            for (int j = 0; j < i; ++j) {
                free(argv_utf8[j]);
            }
            LocalFree(wargv);
            free(argv_utf8);
            return VNDROID_EXIT_FAILURE;
        }
    }

    LocalFree(wargv);

    int ret = main_vndroid(wargc, argv_utf8);

    for (int i = 0; i < wargc; ++i) {
        free(argv_utf8[i]);
    }
    free(argv_utf8);

    return ret;
#endif
}
