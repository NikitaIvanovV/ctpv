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

    PR("md",      NULL,             NULL,             mdcat),
    PR("torrent", NULL,             NULL,             torrent),
    PR("odt",     NULL,             NULL,             odt),

    PR(NULL,      "text",           NULL,             bat),
    PR(NULL,      "text",           NULL,             highlight),
    PR(NULL,      "text",           NULL,             source_highlight),
    PR(NULL,      "text",           NULL,             cat),

    PR(NULL,      "image",          NULL,             ueberzug),
    PR(NULL,      "video",          NULL,             video),

    PR(NULL,      "application",    "pdf",            pdf),
    PR(NULL,      "application",    "json",           jq),
    PR(NULL,      "inode",          "directory",      ls),

    PR(NULL,      "text",           "html",           w3m),
    PR(NULL,      "text",           "html",           lynx),
    PR(NULL,      "text",           "html",           elinks),

    PR(NULL,      "text",           "x-diff",         delta),
    PR(NULL,      "text",           "x-patch",        delta),
    PR(NULL,      "text",           "x-diff",         diff_so_fancy),
    PR(NULL,      "text",           "x-patch",        diff_so_fancy),
    PR(NULL,      "text",           "x-diff",         colordiff),
    PR(NULL,      "text",           "x-patch",        colordiff),

};

/* vim: set nowrap: */
