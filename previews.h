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

    PR("tar.gz",   NULL,            NULL,             atool),
    PR("tgz",      NULL,            NULL,             atool),
    PR("tgz",      NULL,            NULL,             atool),
    PR("tar.gz",   NULL,            NULL,             atool),
    PR("tgz",      NULL,            NULL,             atool),
    PR("tar.bz",   NULL,            NULL,             atool),
    PR("tbz",      NULL,            NULL,             atool),
    PR("tar.bz2",  NULL,            NULL,             atool),
    PR("tbz2",     NULL,            NULL,             atool),
    PR("tar.Z",    NULL,            NULL,             atool),
    PR("tZ",       NULL,            NULL,             atool),
    PR("tar.lzo",  NULL,            NULL,             atool),
    PR("tzo",      NULL,            NULL,             atool),
    PR("tar.lz",   NULL,            NULL,             atool),
    PR("tlz",      NULL,            NULL,             atool),
    PR("tar.xz",   NULL,            NULL,             atool),
    PR("txz",      NULL,            NULL,             atool),
    PR("tar.7z",   NULL,            NULL,             atool),
    PR("t7z",      NULL,            NULL,             atool),
    PR("tar",      NULL,            NULL,             atool),
    PR("zip",      NULL,            NULL,             atool),
    PR("jar",      NULL,            NULL,             atool),
    PR("war",      NULL,            NULL,             atool),
    PR("rar",      NULL,            NULL,             atool),
    PR("lha",      NULL,            NULL,             atool),
    PR("lzh",      NULL,            NULL,             atool),
    PR("7z",       NULL,            NULL,             atool),
    PR("alz",      NULL,            NULL,             atool),
    PR("ace",      NULL,            NULL,             atool),
    PR("a",        NULL,            NULL,             atool),
    PR("arj",      NULL,            NULL,             atool),
    PR("arc",      NULL,            NULL,             atool),
    PR("rpm",      NULL,            NULL,             atool),
    PR("deb",      NULL,            NULL,             atool),
    PR("cab",      NULL,            NULL,             atool),
    PR("gz",       NULL,            NULL,             atool),
    PR("bz",       NULL,            NULL,             atool),
    PR("bz2",      NULL,            NULL,             atool),
    PR("Z",        NULL,            NULL,             atool),
    PR("lzma",     NULL,            NULL,             atool),
    PR("lzo",      NULL,            NULL,             atool),
    PR("lz",       NULL,            NULL,             atool),
    PR("xz",       NULL,            NULL,             atool),
    PR("rz",       NULL,            NULL,             atool),
    PR("lrz",      NULL,            NULL,             atool),
    PR("7z",       NULL,            NULL,             atool),
    PR("cpio",     NULL,            NULL,             atool),

};

/* vim: set nowrap: */
