#include "common.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util/file.h"

#ifdef _WIN32
# define SEP "\\"
#else
# define SEP "/"
#endif

// Replace the platform-specific implementation, util/file.c does not need more
static const char *executable_path;

char *
sc_file_get_executable_path(void) {
    return executable_path ? strdup(executable_path) : NULL;
}

static void check_parent_dir(const char *path, const char *expected) {
    char *dir = sc_file_get_parent_dir(path);
    assert(dir);
    assert(!strcmp(dir, expected));
    free(dir);
}

static void test_parent_dir(void) {
    assert(SEP[0] == SC_PATH_SEPARATOR);

    check_parent_dir(SEP "a" SEP "b", SEP "a");
    check_parent_dir("a" SEP "b" SEP "c", "a" SEP "b");
    check_parent_dir("X:" SEP "VNdroid" SEP "vndroid.exe", "X:" SEP "VNdroid");
}

static void test_parent_dir_no_separator(void) {
    assert(!sc_file_get_parent_dir("vndroid.exe"));
    assert(!sc_file_get_parent_dir(""));
}

static void test_parent_dir_root(void) {
    // The root is not special-cased: the part before the separator is empty
    check_parent_dir(SEP "vndroid", "");
    check_parent_dir(SEP, "");
}

static void test_parent_dir_trailing_separator(void) {
    // Only the last separator counts
    check_parent_dir(SEP "a" SEP "b" SEP, SEP "a" SEP "b");
    check_parent_dir("a" SEP SEP, "a" SEP);
}

static void test_local_path(void) {
    executable_path = "X:" SEP "VNdroid" SEP "vndroid.exe";
    char *path = sc_file_get_local_path("vndroid-update");
    assert(path);
    assert(!strcmp(path, "X:" SEP "VNdroid" SEP "vndroid-update"));
    free(path);

    executable_path = SEP "vndroid";
    path = sc_file_get_local_path("vndroid-update");
    assert(path);
    assert(!strcmp(path, SEP "vndroid-update"));
    free(path);

    executable_path = "vndroid"; // no separator
    assert(!sc_file_get_local_path("vndroid-update"));

    executable_path = NULL;
    assert(!sc_file_get_local_path("vndroid-update"));
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;

    test_parent_dir();
    test_parent_dir_no_separator();
    test_parent_dir_root();
    test_parent_dir_trailing_separator();
    test_local_path();
    return 0;
}
