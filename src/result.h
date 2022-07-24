#ifndef RESULT_H
#define RESULT_H

#include "attrs.h"

#define RESULT enum Result WARN_UNUSED_RESULT

enum Result {
    OK,
    ERR,
};

#endif
