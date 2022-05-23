#ifndef PREVIEW_H
#define PREVIEW_H

#include <stdlib.h>

typedef struct {
    char *ext, *type, *subtype, *script;
} Preview;

typedef struct {
    char *f, *w, *h, *x, *y;
} PreviewArgs;

void init_previews(Preview *ps, size_t len);
void cleanup_previews(void);
int run_preview(const char *ext, const char *mimetype, PreviewArgs *pa);
Preview **get_previews_list(size_t *len);

#endif
