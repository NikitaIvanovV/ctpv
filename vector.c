#include <stdio.h>

#include "error.h"
#include "vector.h"

#define DEFAULT_CAP 16

VECTOR_GEN_SOURCE(Char, char)

Vector *vector_new(size_t size, size_t cap)
{
    Vector *v;

    if (cap == 0)
        cap = DEFAULT_CAP;

    if (!(v = malloc(sizeof(*v)))) {
        PRINTINTERR(FUNCFAILED("malloc"), ERRNOS);
        abort();
    }

    if (!(v->buf = malloc(size * cap))) {
        PRINTINTERR(FUNCFAILED("malloc"), ERRNOS);
        abort();
    }

    v->size = size;
    v->cap = cap;
    v->len = 0;

    return v;
}

void vector_free(Vector *vec)
{
    free(vec->buf);
    free(vec);
}

static void resize_if_needed(Vector *vec, size_t new_len)
{
    void *p;
    size_t cap = vec->cap;

    while (new_len > cap)
        cap *= 2;

    if (cap == vec->cap)
        return;

    if (!(p = realloc(vec->buf, vec->size * cap))) {
        vector_free(vec);
        PRINTINTERR(FUNCFAILED("realloc"), ERRNOS);
        abort();
    }

    vec->buf = p;
    vec->cap = cap;
}

void vector_append_arr(Vector *vec, void *arr, size_t len)
{
    resize_if_needed(vec, vec->len + len);

    memcpy(vec->buf + vec->len * vec->size, arr, len * vec->size);
    vec->len += len;
}

void vector_append(Vector *vec, void *val)
{
    vector_append_arr(vec, val, 1);
}

void *vector_get(Vector *vec, size_t i)
{
    return vec->buf + i * vec->size;
}

void vector_resize(Vector *vec, size_t len)
{
    resize_if_needed(vec, len);

    vec->len = len;
}
