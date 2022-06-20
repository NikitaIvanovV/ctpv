#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <stdarg.h>

#define NOTEXIST_EC 127

#define LEN(a)    (sizeof(a) / sizeof((a)[0]))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define STRINGIZE(x)  STRINGIZE2(x)
#define STRINGIZE2(x) #x

#define FORMATTED_STRING(arr, format)                   \
    do {                                                \
        va_list args;                                   \
        va_start(args, (format));                       \
        vsnprintf((arr), LEN(arr) - 1, (format), args); \
        va_end(args);                                   \
    } while (0)

extern char *program;

int spawn_redirect(const void *arg);
int spawn_wait(int pid, int *exitcode, int *signal);
int spawn(char *args[], int *cpid, int *exitcode, int *signal,
          int (*cfunc)(const void *), const void *carg);

int strcmpnull(const char *s1, const char *s2);
int strlennull(const char *s);

int get_cache_dir(char *buf, size_t len, char *name);
int get_config_dir(char *buf, size_t len, char *name);

int mkpath(char* file_path, int mode);
const char *get_ext(const char *path);

#endif
