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
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    if (!(v->buf = malloc(size * cap))) {
        FUNCFAILED("malloc", strerror(errno));
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
        FUNCFAILED("realloc", strerror(errno));
        abort();
    }

    vec->buf = p;
    vec->cap = cap;
}

size_t vector_append_arr(Vector *vec, void *arr, size_t len)
{
    size_t old_len = vec->len;

    resize_if_needed(vec, vec->len + len);

    memcpy(vec->buf + vec->len * vec->size, arr, len * vec->size);
    vec->len += len;

    return old_len;
}

size_t vector_append(Vector *vec, void *val)
{
    return vector_append_arr(vec, val, 1);
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

void vector_remove(Vector *vec, size_t i)
{
    memcpy(vec->buf + i * vec->size, vec->buf + (--vec->len) * vec->size,
           vec->size);
}
