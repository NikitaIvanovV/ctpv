#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <magic.h>

#include "error.h"
#include "utils.h"
#include "previews.h"

static magic_t magic;

static struct {
    int server;
} ctpv;

static void cleanup(void) {
    cleanup_previews();
    if (magic != NULL)
        magic_close(magic);
}

static int init_magic() {
    magic = magic_open(MAGIC_MIME_TYPE);
    ERRCHK_RET(!magic, "magic_open() failed");

    ERRCHK_RET(magic_load(magic, NULL) != 0, "magic_load() failed: %s",
               magic_error(magic));

    return OK;
}

static const char *get_mimetype(char const *path) {
    const char *r = magic_file(magic, path);
    if (!r) {
        print_errorf("magic_file() failed: %s", magic_error(magic));
        return NULL;
    }

    return r;
}

static const char *get_ext(char const *path) {
    const char *r = strrchr(path, '.');
    if (!r)
        return NULL;

    return &r[1];
}

static int server(void)
{
    /* TODO */ 
    return OK;
}

#define GET_PARG(a, i) (a) = argc > (i) ? argv[i] : NULL

static int client(int argc, char *argv[])
{
    char *f, *w, *h, *x, *y;
    GET_PARG(f, 0);
    GET_PARG(w, 1);
    GET_PARG(h, 2);
    GET_PARG(x, 3);
    GET_PARG(y, 4);

    ERRCHK_RET(!f, "file not given");
    ERRCHK_RET(access(f, R_OK) != 0, "failed to access '%s': %s", f,
               strerror(errno));

    ERRCHK_RET_OK(init_magic());

    init_previews(previews, LEN(previews));

    const char *mimetype = get_mimetype(f);
    ERRCHK_RET(!mimetype);

    Preview *p = find_preview(get_ext(f), mimetype);
    if (!p) {
        puts("no preview found");
        return OK;
    }

    PreviewArgs args = { .f = f, .w = w, .h = h, .x = x, .y = y };

    ERRCHK_RET_OK(run_preview(p, &args));

    return OK;
}

int main(int argc, char *argv[])
{
    program = argc > 0 ? argv[0] : "ctpv";

    int c;
    while ((c = getopt(argc, argv, "s")) != -1) {
        switch (c) {
        case 's':
            ctpv.server = 1;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    int ret;
    if (ctpv.server)
        ret = server();
    else
        ret = client(argc, &argv[optind]);

    cleanup();

    return ret == OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
