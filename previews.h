#include <stdlib.h>
#include <limits.h>

#include "gen/prev/scripts.h"
#include "preview.h"

/*
 * This file is supposed to be included in ctpv.c
 */

#define PP(e, t, s, n, p) { #n, e, t, s, prev_scr_##n##_sh, p }
#define PR(e, t, s, n) PP(e, t, s, n, 0)

Preview previews[] = {
    PP(NULL,      NULL,             NULL,             wrapper, INT_MAX),
    PR(NULL,      "text",           NULL,             text),
    PR(NULL,      NULL,             NULL,             any),
    PR("md",      NULL,             NULL,             markdown),
    PR(NULL,      "application",    "json",           json),
};
