#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "error.h"
#include "utils.h"

char *program = NULL;

RESULT spawn_redirect(const void *arg)
{
    int *fds = (int *)arg;

    ERRCHK_RET_ERN(close(fds[0]) == -1);
    ERRCHK_RET_ERN(dup2(fds[1], fds[2]) == -1);

    return OK;
}

RESULT spawn_wait(int pid, int *exitcode, int *signal)
{
    int stat;
    ERRCHK_RET_ERN(waitpid(pid, &stat, 0) == -1);

    if (exitcode)
        *exitcode = -1;

    if (signal)
        *signal = -1;

    if (WIFEXITED(stat)) {
        if (exitcode)
            *exitcode = WEXITSTATUS(stat);
    } else if (WIFSIGNALED(stat)) {
        if (signal)
            *signal = WTERMSIG(stat);
    }

    return OK;
}

/*
 * Run command
 *
 * If cpid is NULL, wait for the command to finish executing;
 * otherwise store pid in cpid
 *
 * cfunc is a function to call when child process is created
 */
RESULT spawn(char *args[], int *cpid, int *exitcode, int *signal,
             SpawnProg cfunc, const void *carg)
{
    if (exitcode)
        *exitcode = -1;

    int pid = fork();
    ERRCHK_RET_ERN(pid == -1);

    /* Child process */
    if (pid == 0) {
        if (cfunc && (cfunc(carg) != OK))
            exit(EXIT_FAILURE);

        execvp(args[0], args);
        if (errno == ENOENT)
            exit(NOTEXIST_EC);

        FUNCFAILED("exec", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (cpid)
        *cpid = pid;

    if (exitcode || signal)
        ERRCHK_RET_OK(spawn_wait(pid, exitcode, signal));

    return OK;
}

int strcmpnull(const char *s1, const char *s2)
{
    if (!s1 && !s2)
        return 0;
    else if (s1 && !s2)
        return 1;
    else if (!s1 && s2)
        return -1;

    return strcmp(s1, s2);
}

int strlennull(const char *s)
{
    return s ? strlen(s) : 0;
}

static RESULT get_xdg_dir(char *buf, size_t len, char *var, char *var_sub, char *name)
{
    char *home, *dir, dir_buf[FILENAME_MAX];

    if (!(dir = getenv(var))) {
        home = getenv("HOME");
        ERRCHK_RET(!home, "HOME env var does not exist");

        snprintf(dir_buf, LEN(dir_buf)-1, "%s/%s", home, var_sub);
        dir = dir_buf;
    }

    snprintf(buf, len - 1, "%s/%s", dir, name);
    return OK;
}

RESULT get_cache_dir(char *buf, size_t len, char *name)
{
    return get_xdg_dir(buf, len, "XDG_CACHE_HOME", ".cache", name);
}

RESULT get_config_dir(char *buf, size_t len, char *name)
{
    return get_xdg_dir(buf, len, "XDG_CONFIG_HOME", ".config", name);
}

int mkpath(char* file_path, int mode)
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

const char *get_ext(const char *path)
{
    const char *dot = NULL, *s = path + strlen(path) - 1;

    for (; ; s--) {
        if (*s == '.') {
            dot = s + 1;
            continue;
        }

        if (!isalnum(*s) || s == path)
            break;
    }

    return dot;
}

RESULT register_signal(int sig, SigHandler handler)
{
    ERRCHK_RET_ERN(signal(sig, handler) == SIG_ERR);
    return OK;
}

RESULT strtol_w(long *res, char *s, char **endptr, int base)
{
    char *endptr_int;

    errno = 0;
    *res = strtol(s, &endptr_int, base);

    if (errno != 0) {
        FUNCFAILED("strtol", strerror(errno));
        return ERR;
    }

    if (endptr_int[0] != '\0') {
        PRINTINTERR("strtol: invalid number %s", s);
        return ERR;
    }

    if (endptr)
        *endptr = endptr_int;

    return OK;
}
