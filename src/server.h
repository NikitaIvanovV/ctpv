#ifndef SERVER_H
#define SERVER_H

#include "result.h"

RESULT server_listen(const char *id_s);
RESULT server_set_fifo_var(const char *id_s);
RESULT server_clear(const char *id_s);
RESULT server_end(const char *id_s);

#endif
