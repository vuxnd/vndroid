#ifndef VNDROID_H
#define VNDROID_H

#include "common.h"

#include "options.h"

enum vndroid_exit_code {
    // Normal program termination
    VNDROID_EXIT_SUCCESS,

    // No connection could be established
    VNDROID_EXIT_FAILURE,

    // Device was disconnected while running
    VNDROID_EXIT_DISCONNECTED,
};

enum vndroid_exit_code
vndroid(struct vndroid_options *options);

#endif
