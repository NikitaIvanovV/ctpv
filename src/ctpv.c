#include <stdio.h>
#include <magic.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <openssl/md5.h>

#include "ctpv.h"
#include "error.h"
#include "utils.h"
#include "config.h"
#include "server.h"
#include "preview.h"
#include "../version.h"
#include "../previews.h"
#include "../gen/help.h"

struct InputFile {
    char link[PATH_MAX], path[PATH_MAX];
};

struct CTPV ctpv;

const char any_type[] = ANY_TYPE;

static magic_t magic;

static Parser *parser;

static VectorPreview *previews;

static void cleanup(void)
{
    previews_cleanup();
    if (parser)
        config_cleanup(parser);
    if (magic != NULL)
        magic_close(magic);
    if (previews)
        vectorPreview_free(previews);
}

static RESULT init_magic(void)
{
    ERRCHK_RET_MSG(!(magic = magic_open(MAGIC_MIME_TYPE)), magic_error(magic));

    ERRCHK_RET_MSG(magic_load(magic, NULL) != 0, magic_error(magic));

    return OK;
}

static RESULT create_dir(char *buf, size_t len)
{
    char dir[len];
    strncpy(dir, buf, LEN(dir) - 1);
    ERRCHK_RET_ERN(mkpath(dir, 0700) == -1);

    return OK;
}

static RESULT get_config_file(char *buf, size_t len)
{
    ERRCHK_RET_OK(get_config_dir(buf, len, "ctpv/"));
    ERRCHK_RET_OK(create_dir(buf, len));

    strncat(buf, "config", len - 1);

    if (access(buf, F_OK) != 0)
        close(creat(buf, 0600));

    return OK;
}

static RESULT config(int prevs)
{
    char config_file[FILENAME_MAX];
    ERRCHK_RET_OK(get_config_file(config_file, LEN(config_file)));

    ERRCHK_RET_OK(config_load(&parser, prevs ? previews : NULL, config_file));

    return OK;
}

static RESULT init_previews(void)
{
    /* 20 is some arbitrary number, it's here in order to
     * to save one realloc() if user has less then 20 custom previews */
    previews = vectorPreview_new(LEN(b_previews) + 20);
    vectorPreview_append_arr(previews, b_previews, LEN(b_previews));

    ERRCHK_RET_OK(config(1));

    previews_init(previews->buf, previews->len);

    return OK;
}

static const char *get_mimetype(const char *path)
{
    const char *r = magic_file(magic, path);
    if (!r) {
        FUNCFAILED("magic_file", magic_error(magic));
        return NULL;
    }

    return r;
}

static inline void file_access_err(char *f, int errno_)
{
    print_errorf("failed to access '%s': %s", f, strerror(errno_));
}

static RESULT get_input_file(struct InputFile *input_f, char *f)
{
    if (!f) {
        print_error("file not given");
        return ERR;
    }

    input_f->link[0] = input_f->path[0] = '\0';

    if (realpath(f, input_f->path) == NULL) {
        file_access_err(input_f->path, errno);
        return ERR;
    }

    ssize_t link_len = readlink(f, input_f->link, LEN(input_f->link));
    if (link_len == -1 && errno != EINVAL) {
        FUNCFAILED("readlink", strerror(errno));
        return ERR;
    }

    input_f->link[link_len] = '\0';

    return OK;
}

static RESULT is_newer(int *resp, char *f1, char *f2)
{
    struct stat stat1, stat2;
    ERRCHK_RET_ERN(lstat(f1, &stat1) == -1);
    ERRCHK_RET_ERN(lstat(f2, &stat2) == -1);

#if __APPLE__
    struct timespec *t1 = &stat1.st_ctimespec;
    struct timespec *t2 = &stat2.st_ctimespec;
#else
    struct timespec *t1 = &stat1.st_ctim;
    struct timespec *t2 = &stat2.st_ctim;
#endif

    time_t sec_d = t1->tv_sec - t2->tv_sec;
    if (sec_d < 0)
        goto older;
    else if (sec_d == 0 && t1->tv_nsec <= t2->tv_nsec)
        goto older;

    *resp = 1;
    return OK;

older:
    *resp = 0;
    return OK;
}

static void md5_string(char *buf, size_t len, char *s)
{
    unsigned char out[MD5_DIGEST_LENGTH];
    char b[16];

    MD5((const unsigned char *)s, strlen(s), out);

    buf[0] = '\0';
    for (unsigned int i = 0; i < LEN(out); i++) {
        snprintf(b, LEN(b)-1, "%02x", out[i]);
        strncat(buf, b, len);
    }
}

static RESULT get_cache_file(char *dir, size_t dir_len, char *filename,
                             size_t filename_len, char *file)
{
    ERRCHK_RET_OK(get_cache_dir(dir, dir_len, "ctpv/"));
    ERRCHK_RET_OK(create_dir(dir, dir_len));

    size_t dir_str_len = strlen(dir);

    memcpy(filename, dir, filename_len);

    md5_string(filename + dir_str_len, filename_len - dir_str_len - 1, file);

    /* Remove dash at the end */
    dir[dir_str_len-1] = '\0';

    return OK;
}

static RESULT check_cache(int *resp, char *file, char *cache_file)
{
    if (access(cache_file, F_OK) != 0) {
        *resp = 0;
        return OK;
    }

    return is_newer(resp, cache_file, file);
}

#define GET_PARG(a, i) (a) = (argc > (i) ? argv[i] : NULL)

static RESULT preview(int argc, char *argv[])
{
    char *f, *w, *h, *x, *y, *id;
    char w_buf[24], h_buf[24], y_buf[24];
    long w_l, h_l, y_l;

    GET_PARG(f, 0);
    GET_PARG(w, 1);
    GET_PARG(h, 2);
    GET_PARG(x, 3);
    GET_PARG(y, 4);
    GET_PARG(id, 5);

    if (!w)
        w = "80";
    if (!h)
        h = "40";
    if (!x)
        x = "0";
    if (!y)
        y = "0";

    ERRCHK_RET_OK(strtol_w(&w_l, w, NULL, 10));
    ERRCHK_RET_OK(strtol_w(&h_l, h, NULL, 10));
    ERRCHK_RET_OK(strtol_w(&y_l, y, NULL, 10));

    ERRCHK_RET_OK(init_previews());

    struct InputFile input_f;
    ERRCHK_RET_OK(get_input_file(&input_f, f));

    if (!ctpv.opts.nosymlinkinfo && *input_f.link) {
        printf("\033[1;36mSymlink points to:\033[m\n\t%s\n\n", input_f.link);
        fflush(stdout);

        y_l += 3;
        h_l -= 3;
    }

    /* To some reason lf chops off last 2 characters of a line */
    w_l = MAX(0, w_l - 2);

    snprintf(w_buf, LEN(w_buf), "%ld", w_l);
    snprintf(h_buf, LEN(h_buf), "%ld", h_l);
    snprintf(y_buf, LEN(y_buf), "%ld", y_l);

    w = w_buf;
    h = h_buf;
    y = y_buf;

    ERRCHK_RET_OK(init_magic());

    const char *mimetype;
    ERRCHK_RET(!(mimetype = get_mimetype(input_f.path)));

    char cache_dir[FILENAME_MAX], cache_file[FILENAME_MAX];
    ERRCHK_RET_OK(get_cache_file(cache_dir, LEN(cache_file), cache_file,
                                 LEN(cache_file), input_f.path));

    int cache_valid;
    ERRCHK_RET_OK(check_cache(&cache_valid, input_f.path, cache_file));

    PreviewArgs args = {
        .f = input_f.path,
        .w = w,
        .h = h,
        .x = x,
        .y = y,
        .id = id,
        .cache_dir = cache_dir,
        .cache_file = cache_file,
        .cache_valid = cache_valid,
    };

    return preview_run(get_ext(input_f.path), mimetype, &args);
}

static RESULT server(void)
{
    return server_listen(ctpv.server_id_s);
}

static RESULT clear(void)
{
    ERRCHK_RET_OK(config(0));
    return server_clear(ctpv.server_id_s);
}

static RESULT end(void)
{
    ERRCHK_RET_OK(config(0));
    return server_end(ctpv.server_id_s);
}

static RESULT list(void)
{
    ERRCHK_RET_OK(init_previews());

    size_t len;
    Preview p, **list = previews_get(&len);
    const char *n, *e, *t, *s;

    const char header_name[] = "Name", header_ext[] = "Extension",
               header_mime[] = "MIME type";

    int width_name = 0, width_ext = 0;

    for (size_t i = 0; i < len + 1; i++) {
        if (i < len) {
            p = *list[i];
            n = p.name;
            e = p.ext;
        } else {
            n = header_name;
            e = header_ext;
        }

        int name_len = strlennull(n);
        int ext_len = strlennull(e);
        width_name = MAX(width_name, name_len);
        width_ext = MAX(width_ext, ext_len);
    }

    width_name += 2, width_ext += 2;

    puts("List of available previews:\n");
    printf("\t%-*s %-*s %s\n\n", width_name, header_name, width_ext, header_ext,
           header_mime);

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

        printf("\t%-*s .%-*s %s/%s\n", width_name, p.name, width_ext - 1, e, t,
               s);
    }

    puts("\nNote: '" ANY_TYPE "' means that it matches any.\n");

    return OK;
}

static RESULT mime(int argc, char *argv[])
{
    const char *mimetype;
    struct InputFile input_f;

    if (argc <= 0) {
        print_error("files are not specified");
        return ERR;
    }

    ERRCHK_RET_OK(init_magic());

    for (int i = 0; i < argc; i++) {
        ERRCHK_RET_OK(get_input_file(&input_f, argv[i]));

        mimetype = get_mimetype(input_f.path);
        ERRCHK_RET(!mimetype);

        if (argc > 1)
            printf("%s:\t", argv[i]);

        printf(".%s ", get_ext(input_f.path));
        puts(mimetype);
    }

    return OK;
}

static RESULT help(void)
{
    printf("Usage: %s [OPTION]... FILE\n\n", program);
    printf("Options:\n%s", help_txt);
    return OK;
}

static RESULT version(void)
{
    printf("%s version %s\n", program, VERSION);
    return OK;
}

int main(int argc, char *argv[])
{
    program = argc > 0 ? argv[0] : "ctpv";

    ctpv.opts.shell = "/bin/sh";
    ctpv.debug = 0;

    int c;
    while ((c = getopt(argc, argv, "hs:c:e:lmdv")) != -1) {
        switch (c) {
        case 'h':
            ctpv.mode = MODE_HELP;
            break;
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
        case 'd':
            ctpv.debug = 1;
            break;
        case 'v':
            ctpv.mode = MODE_VERSION;
            break;
        default:
            return EXIT_FAILURE;
        }
    }

    argc -= optind;
    argv = &argv[optind];

    enum Result ret;
    switch (ctpv.mode) {
    case MODE_PREVIEW:
        ret = preview(argc, argv);
        break;
    case MODE_HELP:
        ret = help();
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
    case MODE_VERSION:
        ret = version();
        break;
    default:
        PRINTINTERR("unknown mode: %d", ctpv.mode);
        ret = ERR;
        break;
    }

    cleanup();

    return ret == OK ? EXIT_SUCCESS : EXIT_FAILURE;
}
