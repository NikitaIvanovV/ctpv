#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "error.h"
#include "preview.h"

#define PREVP_SIZE sizeof(Preview *)

static char shell[] = "sh";

static Preview **sorted_by_ext,
               **sorted_by_mimetype;
static size_t prevs_length;

static int cmp_prev_ext(const void *p1, const void *p2)
{
    Preview *pr1 = *(Preview **)p1;
    Preview *pr2 = *(Preview **)p2;

    if (!pr1->ext)
        return 1;

    if (!pr2->ext)
        return -1;

    return strcmp(pr1->ext, pr2->ext);
}

static int cmp_prev_mimetype(const void *p1, const void *p2)
{
    Preview *pr1 = *(Preview **)p1;
    Preview *pr2 = *(Preview **)p2;

    if (pr1->type[0] == '\0')
        return 1;
    else if (pr2->type[0] == '\0')
        return -1;

    if (!pr1->subtype && pr2->subtype) {
        return 1;
    } else if (pr1->subtype && !pr2->subtype) {
        return -1;
    } else {
        int ret = strcmp(pr1->type, pr2->type);
        if (ret == 0 && pr1->subtype && pr2->subtype)
            return strcmp(pr1->subtype, pr2->subtype);
        return ret;
    }
}

void init_previews(Preview *ps, size_t len)
{
    prevs_length = len;
    size_t size = len * PREVP_SIZE;

    sorted_by_ext = malloc(size);
    sorted_by_mimetype = malloc(size);
    if (!sorted_by_ext || !sorted_by_mimetype) {
        print_error("malloc() failed");
        abort();
    }

    for (size_t i = 0; i < len; i++)
        sorted_by_ext[i] = sorted_by_mimetype[i] = &ps[i];

    qsort(sorted_by_ext,      len, PREVP_SIZE, cmp_prev_ext);
    qsort(sorted_by_mimetype, len, PREVP_SIZE, cmp_prev_mimetype);
}

void cleanup_previews(void)
{
    if (sorted_by_ext) {
        free(sorted_by_ext);
        sorted_by_ext = NULL;
    }

    if (sorted_by_mimetype) {
        free(sorted_by_mimetype);
        sorted_by_mimetype = NULL;
    }

    prevs_length = 0;
}

static Preview *find_by_ext(char const *ext)
{
    Preview *key = &(Preview){ .ext = (char *)ext };

    void *p =
        bsearch(&key, sorted_by_ext, prevs_length, PREVP_SIZE, cmp_prev_ext);

    return p ? *(Preview **)p : NULL;
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

static Preview *find_by_mimetype(char const *mimetype)
{
    Preview *p;
    char mimetype_c[MIMETYPE_MAX], *t, *s;

    strncpy(mimetype_c, mimetype, MIMETYPE_MAX - 1);
    break_mimetype(mimetype_c, &t, &s);

    for (size_t i = 0; i < prevs_length; i++) {
        p = sorted_by_mimetype[i];

        if (strcmp(t, p->type) == 0) {
            if (p->subtype) {
                if (strcmp(s, p->subtype) == 0)
                    return p;
            } else {
                return p;
            }
        }
    }

    return NULL;
}

Preview *find_preview(char const *ext, char const *mimetype)
{
    if (!sorted_by_ext || !sorted_by_mimetype) {
        print_error("init_previews() not called");
        abort();
    }

    Preview *ret = NULL;
    if (mimetype)
        ret = find_by_mimetype(mimetype);
    if (!ret)
        ret = find_by_ext(ext);

    return ret;
}

#define SET_PENV(n, v)                            \
    do {                                          \
        if (v)                                    \
            ERRCHK_RET(setenv((n), (v), 1) != 0); \
    } while (0)

int run_preview(Preview *p, PreviewArgs *pa)
{
    SET_PENV("f", pa->f);
    SET_PENV("w", pa->w);
    SET_PENV("h", pa->h);
    SET_PENV("x", pa->x);
    SET_PENV("y", pa->y);

    char *args[] = { shell, "-c", p->script, shell, NULL };
    int ret, exitcode;

    ret = spawn(args, NULL, &exitcode);

    if (exitcode > 0)
        printf("error: preview exited with code: %d\n", exitcode);

    return ret;
}
