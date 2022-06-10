#include <stdlib.h>
#include <limits.h>

#include "src/utils.h"
#include "src/preview.h"
#include "gen/prev/scripts.h"

/*
 * This file is supposed to be included in src/ctpv.c
 */

#define PNAME(n)          prev_scr_##n##_sh
#define PP(e, t, s, n, o) { #n, e, t, s, PNAME(n), o, 0, LEN(PNAME(n)) }
#define PR(e, t, s, n)    PP(e, t, s, n, 0)

Preview b_previews[] = {
    PP(NULL,      NULL,             NULL,             wrapper, INT_MAX),
    PR(NULL,      NULL,             NULL,             any),

    PR("md",      NULL,             NULL,             markdown),

    PR(NULL,      "text",           NULL,             bat),
    PR(NULL,      "text",           NULL,             highlight),
    PR(NULL,      "text",           NULL,             source_highlight),
    PR(NULL,      "text",           NULL,             cat),

    PR(NULL,      "image",          NULL,             image),
    PR(NULL,      "video",          NULL,             video),

    PR(NULL,      "application",    "pdf",            pdf),
    PR(NULL,      "application",    "json",           json),

};

/* vim: set nowrap: */
