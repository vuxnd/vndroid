#ifndef SC_PROCESS_INTR_H
#define SC_PROCESS_INTR_H

#include "common.h"

#include "util/intr.h"
#include "util/process.h"

ssize_t
sc_pipe_read_intr(struct sc_intr *intr, sc_pid pid, sc_pipe pipe, char *data,
                  size_t len);

ssize_t
sc_pipe_read_all_intr(struct sc_intr *intr, sc_pid pid, sc_pipe pipe,
                      char *data, size_t len);

/**
 * Wait for the process (interruptible if `intr` is not NULL), then close it
 *
 * Return true if it exited with code 0. Failures are logged (using `name`)
 * unless `log_errors` is false.
 */
bool
sc_process_check_success_intr(struct sc_intr *intr, sc_pid pid,
                              const char *name, bool log_errors);

#endif
