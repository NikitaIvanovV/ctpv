#include <stdlib.h>

#include "gen/prev/scripts.h"
#include "preview.h"

/*
 * This file is supposed to be included in ctpv.c
 */

#define P(e, t, s, f) { e, t, s, f }

Preview previews[] = {
    P(NULL,   "text",           "plain",          prev_scr_file_sh),
};

#undef P
