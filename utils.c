#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error.h"
#include "utils.h"

char *program;

/*
 * Call command
 *
 * If cpid is NULL, wait for the command to finish executing;
 * otherwise store pid in cpid
 *
 * fd is a NULL-terminated array of pairs of file descriptors
 * to pass to dup2()
 */
int spawn(char *args[], pid_t *cpid, int *exitcode, int *fds[2])
{
    if (exitcode)
        *exitcode = -1;

    pid_t pid = fork();
    ERRCHK_RET(pid == -1, "fork() failed");

    /* Child process */
    if (pid == 0) {
        while (*fds) {
            if (dup2((*fds)[0], (*fds)[1]) == -1) {
                print_errorf("dup2() failed: %s", strerror(errno));
                exit(EXIT_FAILURE);
            }
            fds = &fds[1];
        }

        execvp(args[0], args);
        print_errorf("exec() failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (cpid) {
        *cpid = pid;
    } else {
        int stat;
        ERRCHK_RET(waitpid(pid, &stat, 0) == -1, "waitpid() failed: %s",
                   strerror(errno));

        if (exitcode && WIFEXITED(stat))
            *exitcode = WEXITSTATUS(stat);
    }

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

void char_v_append(CharVec *v, char c)
{
    if (!v->buf) {
        v->buf = malloc(v->cap * sizeof(v->buf[0]));
        if (!v->buf) {
            print_error("calloc() failed");
            abort();
        }
        v->buf[0] = '\0';
        v->len++;
    }

    if (v->len + 1 >= v->cap) {
        v->cap *= 2;
        v->buf = realloc(v->buf, v->cap * sizeof(v->buf[0]));
        if (!v->buf) {
            print_error("realloc() failed");
            abort();
        }
    }

    v->buf[v->len - 1] = c;
    v->buf[v->len] = '\0';
    v->len++;
}
