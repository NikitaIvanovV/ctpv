#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "utils.h"
#include "error.h"
#include "shell.h"
#include "server.h"
#include "preview.h"

#define FAILED_PREVIEW_EC NOTEXIST_EC
#define ENOUGH_READ_EC    141

#define PREVP_SIZE sizeof(Preview *)

#define MIMETYPE_MAX 64

VECTOR_GEN_SOURCE(Preview, Preview)

static struct {
    size_t len;
    Preview **list;
} previews;

static int cmp_previews(const void *p1, const void *p2)
{
    Preview *pr1 = *(Preview **)p1;
    Preview *pr2 = *(Preview **)p2;

    int i;

    if ((i = pr2->order - pr1->order) != 0)
        return i;

    if ((i = strcmpnull(pr1->ext, pr2->ext)) != 0)
        return -i;

    if ((i = strcmpnull(pr1->type, pr2->type)) != 0)
        return -i;

    if ((i = strcmpnull(pr1->subtype, pr2->subtype)) != 0)
        return -i;

    if ((i = pr2->priority - pr1->priority) != 0)
        return i;

    return i;
}

void previews_init(Preview *ps, size_t len)
{
    previews.len = len;

    previews.list = malloc(len * PREVP_SIZE);
    if (!previews.list) {
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    for (size_t i = 0; i < len; i++)
        previews.list[i] = &ps[i];

    qsort(previews.list, previews.len, PREVP_SIZE, cmp_previews);
}

void previews_cleanup(void)
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

static Preview *find_preview(const char *type, const char *subtype,
                             const char *ext, size_t *i)
{
    Preview *p;
    const char *rext;

    for (; *i < previews.len; (*i)++) {
        p = previews.list[*i];

        if (ext && (p->attrs & PREV_ATTR_EXT_SHORT)) {
            if ((rext = strrchr(ext, '.')))
                rext += 1;
        } else {
            rext = ext;
        }

        if (p->ext && strcmpnull(p->ext, rext) != 0)
            continue;

        if (p->type && strcmpnull(p->type, type) != 0)
            continue;

        if (p->subtype && strcmpnull(p->subtype, subtype) != 0)
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

static RESULT run(Preview *p, int *exitcode, int *signal)
{
    int pipe_fds[2];
    ERRCHK_RET_ERN(pipe(pipe_fds) == -1);

    int sp_arg[] = { pipe_fds[0], pipe_fds[1], STDERR_FILENO };

    enum Result ret = run_script(p->script, p->script_len, exitcode, signal, spawn_redirect, sp_arg);

    close(pipe_fds[1]);

    if (*exitcode != FAILED_PREVIEW_EC) {
        char buf[256];
        int len;
        while ((len = read(pipe_fds[0], buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, len);
        }

        if (len == -1) {
            FUNCFAILED("read", strerror(errno));
            ret = ERR;
        }
    }

    close(pipe_fds[0]);

    return ret;
}

#define SET_PENV(n, v)                                \
    do {                                              \
        if (v)                                        \
            ERRCHK_RET_ERN(setenv((n), (v), 1) != 0); \
    } while (0)

RESULT preview_run(const char *ext, const char *mimetype, PreviewArgs *pa)
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
    SET_PENV("cache_d", pa->cache_dir);

    {
        char *s = pa->cache_valid ? "1" : "";
        SET_PENV("cache_valid", s);
    }

    SET_PENV("m", mimetype);
    SET_PENV("e", ext);

    check_init_previews();

    Preview *p;
    size_t i = 0;
    int exitcode, signal;
    char mimetype_c[MIMETYPE_MAX], *t, *s;

    strncpy(mimetype_c, mimetype, LEN(mimetype_c) - 1);
    break_mimetype(mimetype_c, &t, &s);

run:
    p = find_preview(t, s, ext, &i);
    if (!p) {
        puts("ctpv: no previews found");
        return ERR;
    }

    if (ctpv.debug)
      fprintf(stderr, "Running preview: %s\n", p->name);

    ERRCHK_RET_OK(run(p, &exitcode, &signal));

    switch (exitcode) {
    case FAILED_PREVIEW_EC:
        if (ctpv.debug)
          fprintf(stderr, "Preview %s failed\n", p->name);
        i++;
        goto run;
    case ENOUGH_READ_EC:
        exitcode = 0;
        break;
    }

    if (signal == SIGPIPE)
        exitcode = 0;

    return exitcode == 0 ? OK : ERR;
}

Preview **previews_get(size_t *len)
{
    check_init_previews();
    *len = previews.len;
    return previews.list;
}
