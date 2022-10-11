#ifndef PREVIEW_H
#define PREVIEW_H

#include <stdlib.h>

#include "vector.h"
#include "result.h"

enum PreviewAttr {
    PREV_ATTR_NONE = 0,
    PREV_ATTR_EXT_SHORT = 1 << 0,
};

typedef struct {
    char *name, *ext, *type, *subtype, *script;
    int order, priority;
    enum PreviewAttr attrs;
    size_t script_len;
} Preview;

VECTOR_GEN_HEADER(Preview, Preview)

typedef struct {
    char *f, *w, *h, *x, *y, *id;
    char *cache_file, *cache_dir;
    int cache_valid;
} PreviewArgs;

void previews_init(Preview *ps, size_t len);
void previews_cleanup(void);
RESULT preview_run(const char *ext, const char *mimetype, PreviewArgs *pa);
Preview **previews_get(size_t *len);

#endif
