#include <string.h>

#include "ctpv.h"
#include "shell.h"
#include "error.h"
#include "../gen/helpers.h"

/*
 * Returns string with sh/helpers.sh prepended
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

#define OPT_SETENV(name) \
    ERRCHK_RET_ERN(setenv((#name), ctpv.opts.name ? "1" : "", 1) == -1)

RESULT run_script(char *script, size_t script_len, int *exitcode, int *signal,
                  SpawnProg sp, void *sp_arg)
{
    OPT_SETENV(forcekitty);
    OPT_SETENV(forcekittyanim);
    OPT_SETENV(forcechafa);
    OPT_SETENV(noimages);
    OPT_SETENV(nosymlinkinfo);
    OPT_SETENV(autochafa);
    OPT_SETENV(showgpg);

    char *scr = prepend_helpers(script, script_len);
    char *args[] = SHELL_ARGS(scr);
    enum Result ret = spawn(args, NULL, exitcode, signal, sp, sp_arg);

    free(scr);

    return ret;
}
