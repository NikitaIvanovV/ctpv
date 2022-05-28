#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "error.h"
#include "utils.h"

char *program = NULL;

int spawn_redirect(const void *arg)
{
    int *fds = (int *)arg;

    ERRCHK_RET(close(fds[0]) == -1, FUNCFAILED("close"), ERRNOS);
    ERRCHK_RET(dup2(fds[1], fds[2]) == -1, FUNCFAILED("dup2"), ERRNOS);

    return OK;
}

int spawn_wait(pid_t pid, int *exitcode)
{
    int stat;
    ERRCHK_RET(waitpid(pid, &stat, 0) == -1, FUNCFAILED("waitpid"), ERRNOS);

    if (exitcode && WIFEXITED(stat))
        *exitcode = WEXITSTATUS(stat);

    return OK;
}

/*
 * Call command
 *
 * If cpid is NULL, wait for the command to finish executing;
 * otherwise store pid in cpid
 *
 * cfunc is a function to call when child process is created
 */
int spawn(char *args[], pid_t *cpid, int *exitcode, int (*cfunc)(const void *),
          const void *carg)
{
    if (exitcode)
        *exitcode = -1;

    pid_t pid = fork();
    ERRCHK_RET(pid == -1, FUNCFAILED("fork"), ERRNOS);

    /* Child process */
    if (pid == 0) {
        if (cfunc && (cfunc(carg) != OK))
            exit(EXIT_FAILURE);

        execvp(args[0], args);
        if (errno == ENOENT)
            exit(NOTEXIST_EC);

        PRINTINTERR(FUNCFAILED("exec"), ERRNOS);
        exit(EXIT_FAILURE);
    }

    if (cpid)
        *cpid = pid;

    if (exitcode)
        ERRCHK_RET_OK(spawn_wait(pid, exitcode));

    return OK;
}

int strcmpnull(char const *s1, char const *s2)
{
    if (!s1 && !s2)
        return 0;
    else if (s1 && !s2)
        return 1;
    else if (!s1 && s2)
        return -1;

    return strcmp(s1, s2);
}

int get_cache_dir(char *buf, size_t len, char *name)
{
    char *home, *cache_d, cache_d_buf[FILENAME_MAX];

    if (!(cache_d = getenv("XDG_CACHE_HOME"))) {
        home = getenv("HOME");
        ERRCHK_RET(!home, "HOME env var does not exist");

        snprintf(cache_d_buf, LEN(cache_d_buf)-1, "%s/.cache", home);
        cache_d = cache_d_buf;
    }

    snprintf(buf, len, "%s/%s", cache_d, name);
    return OK;
}

int mkpath(char* file_path, mode_t mode)
{
    for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
        *p = '\0';

        if (mkdir(file_path, mode) == -1) {
            if (errno != EEXIST) {
                *p = '/';
                return -1;
            }
        }

        *p = '/';
    }

    return 0;
}

CharVec char_v_new(size_t cap)
{
    CharVec v;
    v.buf = NULL;
    v.len = 0;
    v.cap = cap;

    return v;
}

void char_v_free(CharVec *v)
{
    if (v->buf) {
        free(v->buf);
        v->buf = NULL;
        v->len = 0;
    }
}

static void char_v_create(CharVec *v)
{
    if (v->buf)
        return;

    if (!(v->buf = malloc(v->cap * sizeof(*v->buf)))) {
        PRINTINTERR(FUNCFAILED("malloc"), ERRNOS);
        abort();
    }

    v->buf[0] = '\0';
    v->len++;
}

static void char_v_check_cap(CharVec *v, size_t len_inc)
{
    void *new_buf;
    size_t new_len = v->len + len_inc;

    if (new_len < v->cap)
        return;

    while (new_len >= v->cap)
        v->cap *= 2;

    if (!(new_buf = realloc(v->buf, v->cap * sizeof(*v->buf)))) {
        free(v->buf);
        PRINTINTERR(FUNCFAILED("realloc"), ERRNOS);
        abort();
    }

    v->buf = new_buf;
}

void char_v_append(CharVec *v, char c)
{
    char_v_create(v);
    char_v_check_cap(v, 1);

    v->buf[v->len - 1] = c;
    v->buf[v->len] = '\0';
    v->len++;
}

void char_v_append_str(CharVec *v, char *s)
{
    size_t len = strlen(s);

    char_v_create(v);
    char_v_check_cap(v, len);

    memcpy(v->buf + v->len * sizeof(*v->buf), s, len + 1);
    v->len += len;
}
