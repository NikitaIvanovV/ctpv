#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <magic.h>

#include "error.h"
#include "utils.h"
#include "previews.h"

#define ANY_TYPE "*"

static const char any_type[] = ANY_TYPE;

static magic_t magic;

static struct {
    enum {
        MODE_PREVIEW,
        MODE_SERVER,
        MODE_LIST,
    } mode;
} ctpv = { MODE_PREVIEW };

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

static void init_previews_v(void)
{
    init_previews(previews, LEN(previews));
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

#define GET_PARG(a, i) (a) = argc > (i) ? argv[i] : NULL

static int preview(int argc, char *argv[])
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

    init_previews_v();

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

static int server(void)
{
    /* TODO */
    return OK;
}

static int list(void)
{
    init_previews_v();

    size_t len;
    Preview p, **list = get_previews_list(&len);
    const char *t, *s;

    puts("List of available previews:");

    for (size_t i = 0; i < len; i++) {
        p = *list[i];
        t = p.type;
        s = p.subtype;

        if (!t) {
            t = any_type;
            s = any_type;
        } else if (!s) {
            s = any_type;
        }

        printf("\t%s/%s\n", t, s);
    }

    puts("\nNote: '" ANY_TYPE "' means that it matches any mimetype.");
    return OK;
}

int main(int argc, char *argv[])
{
    program = argc > 0 ? argv[0] : "ctpv";

    int c;
    while ((c = getopt(argc, argv, "sl")) != -1) {
        switch (c) {
        case 's':
            ctpv.mode = MODE_SERVER;
            break;
        case 'l':
            ctpv.mode = MODE_LIST;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    int ret;
    switch (ctpv.mode) {
        case MODE_PREVIEW:
            ret = preview(argc, &argv[optind]);
            break;
        case MODE_SERVER:
            ret = server();
            break;
        case MODE_LIST:
            ret = list();
            break;
        default:
            print_errorf("unknowm mode: %d", ctpv.mode);
            ret = ERR;
            break;
    }

    cleanup();

    return ret == OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
