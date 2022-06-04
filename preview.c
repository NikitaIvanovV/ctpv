#include <stdio.h>
#include <unistd.h>

#include "utils.h"
#include "error.h"
#include "shell.h"
#include "server.h"
#include "preview.h"

#define FAILED_PREVIEW_EC NOTEXIST_EC

#define PREVP_SIZE sizeof(Preview *)

static struct {
    size_t len;
    Preview **list;
} previews;

static int cmp_previews(const void *p1, const void *p2)
{
    Preview *pr1 = *(Preview **)p1;
    Preview *pr2 = *(Preview **)p2;

    int i;

    if ((i = pr2->priority - pr1->priority) != 0)
        return i;

    if ((i = strcmpnull(pr1->ext, pr2->ext)) != 0)
        return -i;

    if ((i = strcmpnull(pr1->type, pr2->type)) != 0)
        return -i;

    if ((i = strcmpnull(pr1->subtype, pr2->subtype)) != 0)
        return i;

    return i;
}

void init_previews(Preview *ps, size_t len)
{
    previews.len = len;

    previews.list = malloc(len * PREVP_SIZE);
    if (!previews.list) {
        PRINTINTERR(FUNCFAILED("malloc"), ERRNOS);
        abort();
    }

    for (size_t i = 0; i < len; i++)
        previews.list[i] = &ps[i];

    qsort(previews.list, previews.len, PREVP_SIZE, cmp_previews);
}

void cleanup_previews(void)
{
    if (!previews.list)
        return;

    free(previews.list);
    previews.list = NULL;
    previews.len = 0;
}

static void break_mimetype(char *mimetype, char **type, char **subtype)
{
    *type = mimetype[0] == '\0' ? NULL : mimetype;
    *subtype = NULL;

    char *s = strchr(mimetype, '/');
    if (!s) {
        PRINTINTERR("invalid mimetype: '%s'", mimetype);
        abort();
    }

    *s = '\0';
    *subtype = &s[1];
}

#define MIMETYPE_MAX 64

static Preview *find_preview(const char *mimetype, const char *ext, size_t *i)
{
    Preview *p;
    char mimetype_c[MIMETYPE_MAX], *t, *s;

    strncpy(mimetype_c, mimetype, MIMETYPE_MAX - 1);
    break_mimetype(mimetype_c, &t, &s);

    for (; *i < previews.len; (*i)++) {
        p = previews.list[*i];

        if (p->ext && strcmpnull(p->ext, ext) != 0)
            continue;

        if (p->type && strcmpnull(p->type, t) != 0)
            continue;

        if (p->subtype && strcmpnull(p->subtype, s) != 0)
            continue;

        return p;
    }

    return NULL;
}

static void check_init_previews(void)
{
    if (!previews.list) {
        PRINTINTERR("init_previews() not called");
        abort();
    }
}

static int run(Preview *p, int *exitcode)
{
    int pipe_fds[2];
    ERRCHK_RET(pipe(pipe_fds) == -1, FUNCFAILED("pipe"), ERRNOS);

    int sp_arg[] = { pipe_fds[0], pipe_fds[1], STDERR_FILENO };

    char *script = prepend_helpers(p->script, p->script_len);
    char *args[] = SHELL_ARGS(script);
    int ret = spawn(args, NULL, exitcode, spawn_redirect, sp_arg);

    free(script);
    close(pipe_fds[1]);

    if (*exitcode != FAILED_PREVIEW_EC) {
        char buf[256];
        int len;
        while ((len = read(pipe_fds[0], buf, LEN(buf))) > 0) {
            write(STDOUT_FILENO, buf, len);
        }

        if (len == -1) {
            PRINTINTERR(FUNCFAILED("read"), ERRNOS);
            ret = ERR;
        }
    }

    close(pipe_fds[0]);

    return ret;
}

#define SET_PENV(n, v)                                                 \
    do {                                                               \
        if (v)                                                         \
            ERRCHK_RET(setenv((n), (v), 1) != 0, FUNCFAILED("setenv"), \
                       ERRNOS);                                        \
    } while (0)

int run_preview(const char *ext, const char *mimetype, PreviewArgs *pa)
{
    if (pa->id || (pa->id = getenv("id")))
        ERRCHK_RET_OK(server_set_fifo_var(pa->id));

    SET_PENV("f", pa->f);
    SET_PENV("w", pa->w);
    SET_PENV("h", pa->h);
    SET_PENV("x", pa->x);
    SET_PENV("y", pa->y);
    SET_PENV("id", pa->id);
    SET_PENV("cache_f", pa->cache_file);

    {
        char *s = pa->cache_valid ? "1" : "";
        SET_PENV("cache_valid", s);
    }

    SET_PENV("m", mimetype);
    SET_PENV("e", ext);

    check_init_previews();

    Preview *p;
    size_t i = 0;
    int exitcode;

run:
    p = find_preview(mimetype, ext, &i);
    if (!p) {
        puts("ctpv: no previews found");
        return ERR;
    }

    ERRCHK_RET_OK(run(p, &exitcode));
    if (exitcode == FAILED_PREVIEW_EC) {
        i++;
        goto run;
    }

    return exitcode == 0 ? OK : ERR;
}

Preview **get_previews_list(size_t *len)
{
    check_init_previews();
    *len = previews.len;
    return previews.list;
}
