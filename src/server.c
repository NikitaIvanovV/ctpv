#include <poll.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "error.h"
#include "utils.h"
#include "shell.h"
#include "../gen/server.h"

#define FIFO_FILENAME_SIZE 256

static int ueberzug_pid;

static volatile int do_exit = 0;

static void kill_ueberzug(void)
{
    if (kill(ueberzug_pid, SIGTERM) == -1) {
        if (errno == ESRCH)
            print_error("ueberzug is not running");
        else
            FUNCFAILED("kill", strerror(errno));
    }

    waitpid(ueberzug_pid, NULL, 0);
}

static void sig_handler_exit(int s)
{
    do_exit = 1;
}

static RESULT open_fifo(int *fd, char *f)
{
    ERRCHK_RET_ERN((*fd = open(f, O_RDONLY | O_NONBLOCK)) == -1);

    return OK;
}

static RESULT listen(char *fifo)
{
    enum Result ret = OK;

    struct pollfd pollfd = { .fd = -1, .events = POLLIN };
    ERRCHK_GOTO_OK(open_fifo(&pollfd.fd, fifo), ret, exit);

    /*
     * We don't register actual handlers because when one occures,
     * poll() returns 0, which will break the loop and a normal
     * exit will happen.
     */
    ERRCHK_GOTO_OK(register_signal(SIGINT, sig_handler_exit), ret, fifo);
    ERRCHK_GOTO_OK(register_signal(SIGTERM, sig_handler_exit), ret, fifo);

    int pipe_fds[2];
    ERRCHK_GOTO_ERN(pipe(pipe_fds) == -1, ret, signal);

    char *args[] = { "ueberzug", "layer", NULL };
    int sp_arg[] = { pipe_fds[1], pipe_fds[0], STDIN_FILENO };
    if (spawn(args, &ueberzug_pid, NULL, NULL, spawn_redirect, sp_arg) != OK)
        ret = ERR;

    close(pipe_fds[0]);

    /* If spawn() failed */
    if (ret != OK)
        goto close;

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

        if (pollfd.revents & POLLIN) {
            static char buf[1024];
            while ((len = read(pollfd.fd, buf, sizeof(buf))) > 0) {
                /* But first byte equal to 0 means "exit" */
                if (buf[0] == 0)
                    goto close;
                write(pipe_fds[1], buf, len);
            }
        }

        if (pollfd.revents & POLLHUP) {
            close(pollfd.fd);
            ERRCHK_GOTO_OK(open_fifo(&pollfd.fd, fifo), ret, close);
        }
    }


    ERRCHK_GOTO_ERN(poll_ret < 0, ret, close);

close:
    close(pipe_fds[1]);
    kill_ueberzug();

signal:
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

fifo:
    if (pollfd.fd >= 0)
        close(pollfd.fd);

exit:
    return ret;
}

static RESULT check_ueberzug(int *exitcode)
{
    char *args[] = SHELL_ARGS("command -v ueberzug > /dev/null");
    return spawn(args, NULL, exitcode, NULL, NULL, NULL);
}

static void get_fifo_name(char *buf, size_t len, const char *id_s)
{
    snprintf(buf, len-1, "/tmp/ctpvfifo.%s", id_s);
}

RESULT server_listen(const char *id_s)
{
    enum Result ret = OK;

    int exitcode;
    ERRCHK_GOTO_OK(check_ueberzug(&exitcode), ret, exit);

    if (exitcode == 127) {
        ret = ERR;
        print_error("ueberzug is not installed");
        goto exit;
    }

    char fifo[FIFO_FILENAME_SIZE];
    get_fifo_name(fifo, LEN(fifo), id_s);

    if (mkfifo(fifo, 0600) == -1) {
        if (errno == EEXIST)
            print_errorf("server with id %s is already running or fifo %s still exists", id_s, fifo);
        else
            FUNCFAILED("mkfifo", strerror(errno));
        ret = ERR;
        goto exit;
    }

    ERRCHK_GOTO_OK(listen(fifo), ret, fifo);

fifo:
    if (remove(fifo) == -1 && errno != ENOENT)
        FUNCFAILED("remove", strerror(errno));

exit:
    return ret;
}

static inline RESULT run_server_script(char *script, size_t script_len, char *arg)
{
    return run_script(script, script_len, NULL, NULL, NULL, NULL);
}

RESULT server_set_fifo_var(const char *id_s)
{
    char fifo[FIFO_FILENAME_SIZE];
    get_fifo_name(fifo, LEN(fifo), id_s);
    ERRCHK_RET_ERN(setenv("fifo", fifo, 1) != 0);

    return OK;
}

RESULT server_clear(const char *id_s)
{
    ERRCHK_RET_OK(server_set_fifo_var(id_s));

    return run_server_script(scr_clear_sh, LEN(scr_clear_sh), (char *)id_s);
}

RESULT server_end(const char *id_s)
{
    ERRCHK_RET_OK(server_set_fifo_var(id_s));

    return run_server_script(scr_end_sh, LEN(scr_end_sh), (char *)id_s);
}
