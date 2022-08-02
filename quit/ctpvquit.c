#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        fprintf(stderr, "id not given\n");
        return EXIT_FAILURE;
    }

    char *endptr, *pid_s = argv[1];

    errno = 0;
    long pid = strtol(pid_s, &endptr, 10);

    if (errno != 0) {
        perror("strtol");
        return EXIT_FAILURE;
    }

    if (endptr == pid_s) {
        fprintf(stderr, "%s: invalid number\n", pid_s);
        return EXIT_FAILURE;
    }

    while (1) {
        sleep(1);

        if (kill(pid, 0) == -1) {
            if (errno != ESRCH) {
                perror("kill");
                return EXIT_FAILURE;
            }

            execlp("ctpv", "ctpv", "-e", pid_s, NULL);
            perror("execlp");
            break;
        }
    }

    return EXIT_FAILURE;
}
