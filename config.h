#ifndef CONFIG_H
#define CONFIG_H

#include "preview.h"

int config_load(VectorPreview *prevs, char *filename);
void config_cleanup(void);

#endif
