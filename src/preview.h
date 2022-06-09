#ifndef PREVIEW_H
#define PREVIEW_H

#include <stdlib.h>

#include "vector.h"

#define MIMETYPE_MAX 64

typedef struct {
    char *name, *ext, *type, *subtype, *script;
    int priority;
    size_t script_len;
} Preview;

VECTOR_GEN_HEADER(Preview, Preview)

typedef struct {
    char *f, *w, *h, *x, *y, *id;
    char *cache_file;
    int cache_valid;
} PreviewArgs;

void previews_init(Preview *ps, size_t len);
void previews_cleanup(void);
int preview_run(const char *ext, const char *mimetype, PreviewArgs *pa);
Preview **previews_get(size_t *len);

#endif
