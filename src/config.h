#ifndef CONFIG_H
#define CONFIG_H

#include "preview.h"

typedef struct Parser Parser;

int config_load(Parser **ctx, VectorPreview *prevs, char *filename);
void config_cleanup(Parser *ctx);

#endif
