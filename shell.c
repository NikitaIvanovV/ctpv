#include <string.h>

#include "shell.h"
#include "error.h"
#include "gen/helpers.h"

/*
 * Returns string with helpers.sh prepended
 *
 * User must call free()
 */
char *prepend_helpers(char *str, size_t len)
{
    char *buf, *b;
    size_t mlen;

    if (!(buf = malloc(sizeof(*buf) * (LEN(scr_helpers_sh) + len)))) {
        PRINTINTERR(FUNCFAILED("malloc"), ERRNOS);
        abort();
    }

    b = buf;
    mlen = LEN(scr_helpers_sh);
    memcpy(b, scr_helpers_sh, mlen);

    b += (mlen - 1) * sizeof(*str);
    mlen = len;
    memcpy(b, str, mlen);

    b += mlen * sizeof(*str);
    b[0] = '\0';

    return buf;
}
