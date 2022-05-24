#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdarg.h>

#define LEN(a)    (sizeof(a) / sizeof((a)[0]))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x

#define FORMATTED_STRING(arr, format)                   \
    do {                                                \
        va_list args;                                   \
        va_start(args, (format));                       \
        vsnprintf((arr), LEN(arr) - 1, (format), args); \
        va_end(args);                                   \
    } while (0)

typedef struct {
    char *buf;
    size_t len, cap;
} CharVec;

extern char *program;

int spawn(char *args[], pid_t *cpid, int *exitcode, int *fds[2]);

int strcmpnull(char const *s1, char const *s2);

CharVec char_v_new(size_t cap);
void char_v_append(CharVec *v, char c);
void char_v_free(CharVec *v);

#endif
