#include "process_intr.h"

#include "util/log.h"

ssize_t
sc_pipe_read_intr(struct sc_intr *intr, sc_pid pid, sc_pipe pipe, char *data,
                  size_t len) {
    if (intr && !sc_intr_set_process(intr, pid)) {
        // Already interrupted
        return -1;
    }

    ssize_t ret = sc_pipe_read(pipe, data, len);

    if (intr) {
        sc_intr_set_process(intr, SC_PROCESS_NONE);
    }

    return ret;
}

ssize_t
sc_pipe_read_all_intr(struct sc_intr *intr, sc_pid pid, sc_pipe pipe,
                      char *data, size_t len) {
    if (intr && !sc_intr_set_process(intr, pid)) {
        // Already interrupted
        return -1;
    }

    ssize_t ret = sc_pipe_read_all(pipe, data, len);

    if (intr) {
        sc_intr_set_process(intr, SC_PROCESS_NONE);
    }

    return ret;
}

static bool
process_check_success_internal(sc_pid pid, const char *name, bool close,
                               bool log_errors) {
    if (pid == SC_PROCESS_NONE) {
        if (log_errors) {
            LOGE("Could not execute \"%s\"", name);
        }
        return false;
    }
    sc_exit_code exit_code = sc_process_wait(pid, close);
    if (exit_code) {
        if (log_errors) {
            if (exit_code != SC_EXIT_CODE_NONE) {
                LOGE("\"%s\" returned with value %" SC_PRIexitcode, name,
                     exit_code);
            } else {
                LOGE("\"%s\" exited unexpectedly", name);
            }
        }
        return false;
    }
    return true;
}

bool
sc_process_check_success_intr(struct sc_intr *intr, sc_pid pid,
                              const char *name, bool log_errors) {
    if (intr && !sc_intr_set_process(intr, pid)) {
        // Already interrupted
        return false;
    }

    // Always pass close=false, interrupting would be racy otherwise
    bool ret = process_check_success_internal(pid, name, false, log_errors);

    if (intr) {
        sc_intr_set_process(intr, SC_PROCESS_NONE);
    }

    // Close separately
    sc_process_close(pid);

    return ret;
}
