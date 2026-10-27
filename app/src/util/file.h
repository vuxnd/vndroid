#ifndef SC_FILE_H
#define SC_FILE_H

#include "common.h"

#include <stdbool.h>

#ifdef _WIN32
# define SC_PATH_SEPARATOR '\\'
#else
# define SC_PATH_SEPARATOR '/'
#endif

#ifndef _WIN32
/**
 * Indicate if an executable exists using $PATH
 *
 * In practice, it is only used to know if a package manager is available on
 * the system. It is not implemented on Windows.
 */
bool
sc_file_executable_exists(const char *file);
#endif

/**
 * Return the absolute path of the executable (the VNdroid binary)
 *
 * The result must be freed by the caller using free(). It may return NULL on
 * error.
 */
char *
sc_file_get_executable_path(void);

/**
 * Return the part of the path before the last path separator
 *
 * For example, "/a/b" gives "/a".
 *
 * The result must be freed by the caller using free(). It returns NULL if the
 * path contains no separator, or on error.
 */
char *
sc_file_get_parent_dir(const char *path);

/**
 * Return the absolute path of a file in the same directory as the executable
 *
 * The result must be freed by the caller using free(). It may return NULL on
 * error.
 */
char *
sc_file_get_local_path(const char *name);

/**
 * Return the concatenation of dir, the path separator and the filename.
 *
 * The result must be freed by the caller using free(). It may return NULL on
 * error.
 */
char *
sc_file_build_path(const char *dir, const char *filename);

/**
 * Indicate if the file exists and is not a directory
 */
bool
sc_file_is_regular(const char *path);

#endif
