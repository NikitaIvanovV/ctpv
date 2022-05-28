#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>

#include "error.h"
#include "utils.h"
#include "shell.h"
#include "gen/server.h"

static pid_t ueberzug_pid;

static void kill_ueberzug(void)
{
    if (kill(ueberzug_pid, SIGTERM) == -1) {
        if (errno == ESRCH)
            print_error("ueberzug is not running");
        else
            PRINTINTERR(FUNCFAILED("kill"), ERRNOS);
    }

    spawn_wait(ueberzug_pid, NULL);
}

static void sig_int_handler(int s)
{
    /* Do nothing */
}

static int listen(int fifo_fd)
{
    int ret = OK;

    ERRCHK_GOTO(signal(SIGINT, sig_int_handler) == SIG_ERR, ret, exit,
                FUNCFAILED("signal"), ERRNOS);

    int pipe_fds[2];
    ERRCHK_GOTO(pipe(pipe_fds) == -1, ret, signal, FUNCFAILED("pipe"), ERRNOS);

    char *args[] = { "ueberzug", "layer", NULL };
    int sp_arg[] = { pipe_fds[1], pipe_fds[0], STDIN_FILENO };
    if (spawn(args, &ueberzug_pid, NULL, spawn_redirect, sp_arg) != OK)
        ret = ERR;

    close(pipe_fds[0]);

    /* If spawn() failed */
    if (ret != OK)
        goto close;

    struct pollfd pollfd = { .fd = fifo_fd, .events = POLLIN };

    int poll_ret, len;
    while ((poll_ret = poll(&pollfd, 1, -1) > 0)) {
        static char buf[1024];
        while ((len = read(fifo_fd, buf, LEN(buf))) > 0) {
            /* First zero byte means that ctpv -e $id was run */
            if (buf[0] == 0)
                goto close;
            write(pipe_fds[1], buf, len);
        }
    }

    ERRCHK_GOTO(poll_ret < 0, ret, close, FUNCFAILED("poll"), ERRNOS);

close:
    close(pipe_fds[1]);
    kill_ueberzug();

signal:
    signal(SIGINT, SIG_DFL);

exit:
    return ret;
}

static int check_ueberzug(int *exitcode)
{
    char *args[] = SHELL_ARGS("command -v ueberzug > /dev/null");
    return spawn(args, NULL, exitcode, NULL, NULL);
}

int server_listen(char const *id_s)
{
    int ret = OK;

    int exitcode;
    ERRCHK_GOTO_OK(check_ueberzug(&exitcode), ret, exit);

    if (exitcode == 127) {
        print_error("ueberzug is not installed");
        goto exit;
    }

    char fifo[256];
    snprintf(fifo, LEN(fifo)-1, "/tmp/ctpvfifo.%s", id_s);

    ERRCHK_GOTO(mkfifo(fifo, 0600) == -1 && errno != EEXIST, ret, exit,
                FUNCFAILED("mkfifo"), ERRNOS);

    int fifo_fd;
    ERRCHK_GOTO((fifo_fd = open(fifo, O_RDONLY | O_NONBLOCK)) == -1, ret, fifo,
                FUNCFAILED("open"), ERRNOS);

    ERRCHK_GOTO_OK(listen(fifo_fd), ret, fifo_fd);

fifo_fd:
    close(fifo_fd);

fifo:
    if (remove(fifo) == -1)
        PRINTINTERR(FUNCFAILED("remove"), ERRNOS);

exit:
    return ret;
}

static int run_script(char *script, size_t script_len, char *arg)
{
    int ret = OK;

    char *s = prepend_helpers(script, script_len);
    char *args[] = SHELL_ARGS(s, arg);
    int exitcode;
    ERRCHK_GOTO_OK(spawn(args, NULL, &exitcode, NULL, NULL), ret, cleanup);

cleanup:
    free(s);
    return ret;
}

int server_clear(void)
{
    return run_script(scr_clear_sh, LEN(scr_clear_sh)-1, "");
}

int server_end(const char *id_s)
{
    return run_script(scr_end_sh, LEN(scr_end_sh)-1, (char *)id_s);
}
