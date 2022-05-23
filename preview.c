#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "error.h"
#include "preview.h"

#define PREVP_SIZE sizeof(Preview *)

static char shell[] = "sh";

static Preview **prevs;
static size_t prevs_len;

static int cmp_previews(const void *p1, const void *p2)
{
    Preview *pr1 = *(Preview **)p1;
    Preview *pr2 = *(Preview **)p2;

    if (pr1->ext && pr2->ext)
        return strcmp(pr1->ext, pr2->ext);
    else if (!pr1->ext && pr2->ext)
        return 1;
    else if (pr1->ext && !pr2->ext)
        return -1;

    if (!pr1->type && pr2->type)
        return 1;
    else if (pr1->type && !pr2->type)
        return -1;
    else if (!pr1->type && !pr2->type)
        return 0;

    if (!pr1->subtype && pr2->subtype)
        return 1;
    else if (pr1->subtype && !pr2->subtype)
        return -1;

    int ret = strcmp(pr1->type, pr2->type);

    if (ret == 0 && pr1->subtype && pr2->subtype)
        return strcmp(pr1->subtype, pr2->subtype);

    return ret;
}

void init_previews(Preview *ps, size_t len)
{
    prevs_len = len;

    prevs = malloc(len * PREVP_SIZE);
    if (!prevs) {
        print_error("malloc() failed");
        abort();
    }

    for (size_t i = 0; i < len; i++)
        prevs[i] = &ps[i];

    qsort(prevs, len, PREVP_SIZE, cmp_previews);
}

void cleanup_previews(void)
{
    if (prevs) {
        free(prevs);
        prevs = NULL;
    }

    prevs_len = 0;
}

static void break_mimetype(char *mimetype, char **type, char **subtype)
{
    *type = mimetype[0] == '\0' ? NULL : mimetype;
    *subtype = NULL;

    char *s = strchr(mimetype, '/');
    if (!s) {
        print_errorf("invalid mimetype: '%s'", mimetype);
        abort();
    }

    *s = '\0';
    *subtype = &s[1];
}

#define MIMETYPE_MAX 64

static Preview *find_preview(char const *mimetype, char const *ext, size_t *i)
{
    Preview *p;
    char mimetype_c[MIMETYPE_MAX], *t, *s;

    strncpy(mimetype_c, mimetype, MIMETYPE_MAX - 1);
    break_mimetype(mimetype_c, &t, &s);

    for (; *i < prevs_len; (*i)++) {
        p = prevs[*i];

        if (!p->ext && !p->type)
            return p;

        if (p->ext && !ext)
            continue;

        if (p->ext && strcmp(ext, p->ext) == 0)
            return p;

        if (p->type && strcmp(t, p->type) != 0)
            continue;

        if (p->type && !p->subtype)
            return p;

        if (p->subtype && strcmp(s, p->subtype) == 0)
            return p;
    }

    return NULL;
}

static void check_init_previews(void)
{
    if (!prevs) {
        print_error("init_previews() not called");
        abort();
    }
}

static int run(Preview *p, int *exitcode)
{
    char *args[] = { shell, "-c", p->script, shell, NULL };

    int *fds[] = { (int[]){ STDOUT_FILENO, STDERR_FILENO }, NULL };

    return spawn(args, NULL, exitcode, fds);
}

#define SET_PENV(n, v)                            \
    do {                                          \
        if (v)                                    \
            ERRCHK_RET(setenv((n), (v), 1) != 0); \
    } while (0)

int run_preview(const char *ext, const char *mimetype, PreviewArgs *pa)
{
    SET_PENV("f", pa->f);
    SET_PENV("w", pa->w);
    SET_PENV("h", pa->h);
    SET_PENV("x", pa->x);
    SET_PENV("y", pa->y);

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
        return OK;
    }

    ERRCHK_RET_OK(run(p, &exitcode));
    if (exitcode == 127) {
        i++;
        goto run;
    }

    return OK;
}

Preview **get_previews_list(size_t *len)
{
    check_init_previews();
    *len = prevs_len;
    return prevs;
}
