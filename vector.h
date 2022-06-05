#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

#define VECTOR_TYPE(name, type) \
    typedef struct {            \
        size_t len, cap, size;  \
        type *buf;              \
    } Vector##name

#define VECTOR_SIGN(name, type, func, ret, ...) \
    ret vector_##func##_##type(__VA_ARGS__)

#define VECTOR_SIGN_V(name, type, func, ret, ...) \
    ret vector_##func##_##type(Vector##name *v __VA_OPT__(,) __VA_ARGS__)

#define VECTOR_SIGN_NEW(name, type)        VECTOR_SIGN(name,   type, new, Vector##name *, size_t cap)
#define VECTOR_SIGN_FREE(name, type)       VECTOR_SIGN_V(name, type, free, void)
#define VECTOR_SIGN_APPEND_ARR(name, type) VECTOR_SIGN_V(name, type, append_arr, void, type *arr, size_t len)
#define VECTOR_SIGN_APPEND(name, type)     VECTOR_SIGN_V(name, type, append, void, type val)
#define VECTOR_SIGN_GET(name, type)        VECTOR_SIGN_V(name, type, get, type, size_t i)

#define VECTOR_GEN_SOURCE_(name, type, spec)                  \
    spec VECTOR_SIGN_NEW(name, type)                          \
    {                                                         \
        return (Vector##name *)vector_new(sizeof(type), cap); \
    }                                                         \
    spec VECTOR_SIGN_FREE(name, type)                         \
    {                                                         \
        vector_free((Vector *)v);                             \
    }                                                         \
    spec VECTOR_SIGN_APPEND_ARR(name, type)                   \
    {                                                         \
        vector_append_arr((Vector *)v, arr, len);             \
    }                                                         \
    spec VECTOR_SIGN_APPEND(name, type)                       \
    {                                                         \
        vector_append((Vector *)v, &val);                     \
    }                                                         \
    spec VECTOR_SIGN_GET(name, type)                          \
    {                                                         \
        return *(type *)vector_get((Vector *)v, i);           \
    }

#define VECTOR_GEN_SOURCE(name, type) VECTOR_GEN_SOURCE_(name, type, )
#define VECTOR_GEN_SOURCE_STATIC(name, type) \
    VECTOR_TYPE(name, type);                 \
    VECTOR_GEN_SOURCE_(name, type, static __attribute__((unused)))

#define VECTOR_GEN_HEADER(name, type)   \
    VECTOR_TYPE(name, type);            \
    VECTOR_SIGN_NEW(name, type);        \
    VECTOR_SIGN_APPEND_ARR(name, type); \
    VECTOR_SIGN_APPEND(name, type);     \
    VECTOR_SIGN_FREE(name, type);       \
    VECTOR_SIGN_GET(name, type);

VECTOR_TYPE(, void);

Vector *vector_new(size_t size, size_t cap);
void vector_free(Vector *vec);
void vector_append_arr(Vector *vec, void *arr, size_t len);
void vector_append(Vector *vec, void *arr);
void *vector_get(Vector *vec, size_t i);

VECTOR_GEN_HEADER(Char, char)

#endif
