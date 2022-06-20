#include <string.h>

#include "ctpv.h"
#include "shell.h"
#include "error.h"
#include "../gen/helpers.h"

/*
 * Returns string with helpers.sh prepended
 *
 * User must call free()
 */
static char *prepend_helpers(char *str, size_t len)
{
    char *buf, *b;
    size_t l, helpers_len = LEN(scr_helpers_sh) - 1;

    if (!(buf = malloc(sizeof(*buf) * (helpers_len + len)))) {
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    b = buf;
    l = helpers_len;
    memcpy(b, scr_helpers_sh, l * sizeof(*b));

    b += l;
    l = len;
    memcpy(b, str, l * sizeof(*b));

    return buf;
}

int run_script(char *script, size_t script_len, int *exitcode, int *signal,
               SpawnProg sp, void *sp_arg)
{
    ERRCHK_RET_ERN(setenv("forcekitty", ctpv.opts.forcekitty ? "1" : "", 1) == -1);
    ERRCHK_RET_ERN(setenv("noimages", ctpv.opts.noimages ? "1" : "", 1) == -1);

    char *scr = prepend_helpers(script, script_len);
    char *args[] = SHELL_ARGS(scr);
    int ret = spawn(args, NULL, exitcode, signal, sp, sp_arg);

    free(scr);

    return ret;
}
