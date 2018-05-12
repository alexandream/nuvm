#ifndef N_E_MODULE_H
#define N_E_MODULE_H

#include <stdlib.h>

#include "../common/errors.h"

#include "values.h"

typedef struct NModule NModule;


struct NModule {
    unsigned char *code;
    uint32_t code_size;
    NValue *globals;
    uint16_t num_globals;
    uint32_t entry_point;
};


void
ni_init_modules(NError* error);


NModule*
n_create_module(uint16_t num_globals, uint32_t code_size, NError *error);

void
n_destroy_module(NModule* self);


#endif /* N_E_MODULE_H */
