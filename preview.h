#ifndef PREVIEW_H
#define PREVIEW_H

#include <stdlib.h>

#include "vector.h"

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

void init_previews(Preview *ps, size_t len);
void cleanup_previews(void);
int run_preview(const char *ext, const char *mimetype, PreviewArgs *pa);
Preview **get_previews_list(size_t *len);

#endif
