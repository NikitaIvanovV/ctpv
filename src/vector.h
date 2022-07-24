#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

#include "attrs.h"

#define VECTOR_TYPE(name, type) \
    typedef struct {            \
        size_t len, cap, size;  \
        type *buf;              \
    } Vector##name

#define VECTOR_SIGN(name, type, func, ret, ...) \
    ret vector##name##_##func(__VA_ARGS__)

#define VECTOR_SIGN_V(name, type, func, ret, ...) \
    VECTOR_SIGN(name, type, func, ret,            \
                Vector##name *vec __VA_OPT__(, ) __VA_ARGS__)

#define VECTOR_SIGN_NEW(name, type)        VECTOR_SIGN(name,   type, new, Vector##name *, size_t cap)
#define VECTOR_SIGN_FREE(name, type)       VECTOR_SIGN_V(name, type, free, void)
#define VECTOR_SIGN_APPEND_ARR(name, type) VECTOR_SIGN_V(name, type, append_arr, size_t, type *arr, size_t len)
#define VECTOR_SIGN_APPEND(name, type)     VECTOR_SIGN_V(name, type, append, size_t, type val)
#define VECTOR_SIGN_GET(name, type)        VECTOR_SIGN_V(name, type, get, type, size_t i)
#define VECTOR_SIGN_RESIZE(name, type)     VECTOR_SIGN_V(name, type, resize, void, size_t len)
#define VECTOR_SIGN_REMOVE(name, type)     VECTOR_SIGN_V(name, type, remove, void, size_t i)

#define VECTOR_GEN_SOURCE_(name, type, spec)                  \
    inline spec VECTOR_SIGN_NEW(name, type)                   \
    {                                                         \
        return (Vector##name *)vector_new(sizeof(type), cap); \
    }                                                         \
    inline spec VECTOR_SIGN_FREE(name, type)                  \
    {                                                         \
        vector_free((Vector *)vec);                           \
    }                                                         \
    inline spec VECTOR_SIGN_APPEND_ARR(name, type)            \
    {                                                         \
        return vector_append_arr((Vector *)vec, arr, len);    \
    }                                                         \
    inline spec VECTOR_SIGN_APPEND(name, type)                \
    {                                                         \
        return vector_append((Vector *)vec, &val);            \
    }                                                         \
    inline spec VECTOR_SIGN_GET(name, type)                   \
    {                                                         \
        return *(type *)vector_get((Vector *)vec, i);         \
    }                                                         \
    inline spec VECTOR_SIGN_RESIZE(name, type)                \
    {                                                         \
        vector_resize((Vector *)vec, len);                    \
    }                                                         \
    inline spec VECTOR_SIGN_REMOVE(name, type)                \
    {                                                         \
        vector_remove((Vector *)vec, i);                      \
    }

#define VECTOR_GEN_SOURCE(name, type) VECTOR_GEN_SOURCE_(name, type, )
#define VECTOR_GEN_SOURCE_STATIC(name, type) \
    VECTOR_TYPE(name, type);                 \
    VECTOR_GEN_SOURCE_(name, type, static UNUSED)

#define VECTOR_GEN_HEADER(name, type)   \
    VECTOR_TYPE(name, type);            \
    VECTOR_SIGN_NEW(name, type);        \
    VECTOR_SIGN_FREE(name, type);       \
    VECTOR_SIGN_APPEND_ARR(name, type); \
    VECTOR_SIGN_APPEND(name, type);     \
    VECTOR_SIGN_GET(name, type);        \
    VECTOR_SIGN_RESIZE(name, type);     \
    VECTOR_SIGN_REMOVE(name, type);

VECTOR_TYPE(, void);

Vector *vector_new(size_t size, size_t cap);
void vector_free(Vector *vec);
size_t vector_append_arr(Vector *vec, void *arr, size_t len);
size_t vector_append(Vector *vec, void *arr);
void *vector_get(Vector *vec, size_t i);
void vector_resize(Vector *vec, size_t len);
void vector_remove(Vector *vec, size_t i);

VECTOR_GEN_HEADER(Char, char)

#endif
