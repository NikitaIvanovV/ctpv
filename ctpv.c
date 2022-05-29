#include <stdio.h>
#include <magic.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/md5.h>

#include "error.h"
#include "server.h"
#include "preview.h"
#include "previews.h"

#define ANY_TYPE "*"

static const char any_type[] = ANY_TYPE;

static magic_t magic;

static struct {
    enum {
        MODE_PREVIEW,
        MODE_SERVER,
        MODE_CLEAR,
        MODE_END,
        MODE_LIST,
        MODE_MIME,
        MODE_CHECK_CACHE,
    } mode;
    char *server_id_s;
    char *check_file;
    char *ctpv_path;
} ctpv = { .mode = MODE_PREVIEW };

static void cleanup(void) {
    cleanup_previews();
    if (magic != NULL)
        magic_close(magic);
}

static int init_magic()
{
    ERRCHK_RET(!(magic = magic_open(MAGIC_MIME_TYPE)), FUNCFAILED("magic_open"),
               magic_error(magic));

    ERRCHK_RET(magic_load(magic, NULL) != 0, FUNCFAILED("magic_load"),
               magic_error(magic));

    return OK;
}

static void init_previews_v(void)
{
    init_previews(previews, LEN(previews));
}

static const char *get_mimetype(char const *path)
{
    const char *r = magic_file(magic, path);
    if (!r) {
        PRINTINTERR(FUNCFAILED("magic_file"), magic_error(magic));
        return NULL;
    }

    return r;
}

static const char *get_ext(char const *path)
{
    const char *base;

    if ((base = strrchr(path, '/')))
        base += sizeof(*base);
    else
        base = path;

    const char *dot = strchr(base, '.');
    if (!dot || dot == base)
        return NULL;

    return &dot[1];
}

static int check_file(char const *f)
{
    if (!f) {
        print_error("file not given");
        return ERR;
    }

    if (access(f, R_OK) != 0) {
        print_errorf("failed to access '%s': %s", f, ERRNOS);
        return ERR;
    }

    return OK;
}

#define GET_PARG(a, i) (a) = argc > (i) ? argv[i] : NULL

static int preview(int argc, char *argv[])
{
    if (!ctpv.ctpv_path) {
        print_error("argument 0 is null");
        return ERR;
    }

    char *f, *w, *h, *x, *y, *id;
    GET_PARG(f, 0);
    GET_PARG(w, 1);
    GET_PARG(h, 2);
    GET_PARG(x, 3);
    GET_PARG(y, 4);
    GET_PARG(id, 5);

    ERRCHK_RET_OK(check_file(f));

    ERRCHK_RET_OK(init_magic());

    init_previews_v();

    const char *mimetype;
    ERRCHK_RET(!(mimetype = get_mimetype(f)));

    PreviewArgs args = {
        .ctpv = ctpv.ctpv_path,
        .f = f, .w = w, .h = h, .x = x, .y = y, .id = id
    };

    return run_preview(get_ext(f), mimetype, &args);
}

static int server(void)
{
    return server_listen(ctpv.server_id_s);
}

static int clear(void)
{
    return server_clear(ctpv.server_id_s);
}

static int end(void)
{
    return server_end(ctpv.server_id_s);
}

static int list(void)
{
    init_previews_v();

    size_t len;
    Preview p, **list = get_previews_list(&len);
    const char *e, *t, *s;

    puts("List of available previews:");

    for (size_t i = 0; i < len; i++) {
        p = *list[i];
        e = p.ext;
        t = p.type;
        s = p.subtype;

        if (!e)
            e = any_type;

        if (!t) {
            t = any_type;
            s = any_type;
        } else if (!s) {
            s = any_type;
        }

        printf("\t%-15s .%-6s %s/%s\n", p.name, e, t, s);
    }

    puts("\nNote: '" ANY_TYPE "' means that it matches any.");
    return OK;
}

static int mime(int argc, char *argv[])
{
    char const *f, *mimetype;

    for (int i = 0; i < argc; i++) {
        f = argv[i];
        ERRCHK_RET_OK(check_file(f));

        ERRCHK_RET_OK(init_magic());

        mimetype = get_mimetype(f);
        ERRCHK_RET(!mimetype);

        if (argc > 1)
            printf("%s: ", f);

        printf(".%s ", get_ext(f));
        puts(mimetype);
    }

    return OK;
}

static int is_newer(char *f1, char *f2)
{
    struct stat stat1, stat2;
    ERRCHK_RET(stat(f1, &stat1) == -1, FUNCFAILED("stat"), ERRNOS);
    ERRCHK_RET(stat(f2, &stat2) == -1, FUNCFAILED("stat"), ERRNOS);

    int sec_d = stat1.st_mtim.tv_sec - stat2.st_mtim.tv_sec;
    if (sec_d < 0)
        return ERR;
    else if (sec_d == 0 && stat1.st_mtim.tv_nsec <= stat2.st_mtim.tv_nsec)
        return ERR;

    return OK;
}

static void md5_string(char *buf, size_t len, char *s)
{
    unsigned char out[MD5_DIGEST_LENGTH];
    char b[16];

    MD5((const unsigned char *)s, strlen(s), out);

    buf[0] = '\0';
    for(unsigned int i = 0; i < LEN(out); i++) {
        snprintf(b, LEN(b)-1, "%02x", out[i]);
        strncat(buf, b, len);
    }
}

static int check_cache(void)
{
    ERRCHK_RET_OK(check_file(ctpv.check_file));

    char cache_file[FILENAME_MAX];
    ERRCHK_RET_OK(
        get_cache_dir(cache_file, LEN(cache_file) - 1, "ctpv//"));

    {
        char cache_file_cpy[FILENAME_MAX];
        strncpy(cache_file_cpy, cache_file, LEN(cache_file_cpy) - 1);
        ERRCHK_RET(mkpath(cache_file_cpy, 0700) == -1, FUNCFAILED("mkpath"),
                   ERRNOS);
    }

    char name[64];
    md5_string(name, LEN(name)-1, ctpv.check_file);

    strncat(cache_file, name, LEN(cache_file)-1);
    puts(cache_file);

    if (access(cache_file, F_OK) != 0)
        return ERR;

    return is_newer(cache_file, ctpv.check_file);
}

int main(int argc, char *argv[])
{
    ctpv.ctpv_path = argc > 0 ? argv[0] : NULL;
    program = ctpv.ctpv_path ? ctpv.ctpv_path : "ctpv";

    int c;
    while ((c = getopt(argc, argv, "s:c:e:lmC:")) != -1) {
        switch (c) {
        case 's':
            ctpv.mode = MODE_SERVER;
            ctpv.server_id_s = optarg;
            break;
        case 'c':
            ctpv.mode = MODE_CLEAR;
            ctpv.server_id_s = optarg;
            break;
        case 'e':
            ctpv.mode = MODE_END;
            ctpv.server_id_s = optarg;
            break;
        case 'l':
            ctpv.mode = MODE_LIST;
            break;
        case 'm':
            ctpv.mode = MODE_MIME;
            break;
        case 'C':
            ctpv.mode = MODE_CHECK_CACHE;
            ctpv.check_file = optarg;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv = &argv[optind];

    int ret;
    switch (ctpv.mode) {
        case MODE_PREVIEW:
            ret = preview(argc, argv);
            break;
        case MODE_SERVER:
            ret = server();
            break;
        case MODE_CLEAR:
            ret = clear();
            break;
        case MODE_END:
            ret = end();
            break;
        case MODE_LIST:
            ret = list();
            break;
        case MODE_MIME:
            ret = mime(argc, argv);
            break;
        case MODE_CHECK_CACHE:
            ret = check_cache();
            break;
        default:
            PRINTINTERR("unknowm mode: %d", ctpv.mode);
            ret = ERR;
            break;
    }

    cleanup();

    return ret == OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
