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

#define FIFO_FILENAME_SIZE 256

static pid_t ueberzug_pid;

static volatile int do_exit = 0;

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

static void sig_handler_exit(int s)
{
    do_exit = 1;
}

static int register_signal(int sig, __sighandler_t handler)
{
    ERRCHK_RET(signal(sig, handler), FUNCFAILED("signal"), ERRNOS);
    return OK;
}

static int listen(int fifo_fd)
{
    int ret = OK;

    /*
     * We don't register actual handlers because when one occures,
     * poll() returns 0, which will break the loop and a normal
     * exit will happen.
     */
    ERRCHK_GOTO_OK(register_signal(SIGINT, sig_handler_exit), ret, exit);
    ERRCHK_GOTO_OK(register_signal(SIGTERM, sig_handler_exit), ret, exit);

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

    /*
     * "Listen" to fifo and redirect all the input to ueberzug
     * instance.
     */
    int poll_ret, len;
    while ((poll_ret = poll(&pollfd, 1, 100) >= 0)) {
        if (do_exit)
            goto close;

        if (poll_ret == 0)
            continue;

        static char buf[1024];
        while ((len = read(fifo_fd, buf, LEN(buf))) > 0) {
            /* But first byte equal to 0 means "exit" */
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
    signal(SIGTERM, SIG_DFL);

exit:
    return ret;
}

static int check_ueberzug(int *exitcode)
{
    char *args[] = SHELL_ARGS("command -v ueberzug > /dev/null");
    return spawn(args, NULL, exitcode, NULL, NULL);
}

static void get_fifo_name(char *buf, size_t len, const char *id_s)
{
    snprintf(buf, len-1, "/tmp/ctpvfifo.%s", id_s);
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

    char fifo[FIFO_FILENAME_SIZE];
    get_fifo_name(fifo, LEN(fifo), id_s);

    ERRCHK_GOTO(mkfifo(fifo, 0600) == -1 && errno != EEXIST, ret, exit,
                FUNCFAILED("mkfifo"), ERRNOS);

    int fifo_fd;
    ERRCHK_GOTO((fifo_fd = open(fifo, O_RDONLY | O_NONBLOCK)) == -1, ret, fifo,
                FUNCFAILED("open"), ERRNOS);

    ERRCHK_GOTO_OK(listen(fifo_fd), ret, fifo_fd);

fifo_fd:
    close(fifo_fd);

fifo:
    if (remove(fifo) == -1 && errno != ENOENT)
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

int server_set_fifo_var(const char *id_s)
{
    char fifo[FIFO_FILENAME_SIZE];
    get_fifo_name(fifo, LEN(fifo), id_s);
    ERRCHK_RET(setenv("fifo", fifo, 1) != 0, FUNCFAILED("setenv"), ERRNOS);

    return OK;
}

int server_clear(const char *id_s)
{
    ERRCHK_RET_OK(server_set_fifo_var(id_s));

    return run_script(scr_clear_sh, LEN(scr_clear_sh)-1, (char *)id_s);
}

int server_end(const char *id_s)
{
    ERRCHK_RET_OK(server_set_fifo_var(id_s));

    return run_script(scr_end_sh, LEN(scr_end_sh)-1, (char *)id_s);
}
