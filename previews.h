#include <stdlib.h>
#include <limits.h>

#include "utils.h"
#include "preview.h"
#include "gen/prev/scripts.h"

/*
 * This file is supposed to be included in ctpv.c
 */

#define PNAME(n) prev_scr_##n##_sh
#define PP(e, t, s, n, p) { #n, e, t, s, PNAME(n), p, LEN(PNAME(n)) }
#define PR(e, t, s, n) PP(e, t, s, n, 0)

Preview previews[] = {
    PP(NULL,      NULL,             NULL,             wrapper, INT_MAX),
    PR(NULL,      "text",           NULL,             text),
    PR(NULL,      NULL,             NULL,             any),
    PR("md",      NULL,             NULL,             markdown),
    PR(NULL,      "application",    "json",           json),
    PR(NULL,      "image",          NULL,             image),
    PR(NULL,      "video",          NULL,             video),
    PR(NULL,      "application",    "pdf",            pdf),
};

/* vim: set nowrap: */
