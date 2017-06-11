#ifndef N_E_MODULE_H
#define N_E_MODULE_H

#include <stdlib.h>

#include "../common/errors.h"

#include "values.h"

typedef struct NModule NModule;


struct NModule {
    unsigned char *code;
    int code_size;
    NValue *registers;
    int num_registers;
    int entry_point;
};


int
ni_init_modules(void);


NModule*
n_new_module(NError *error);


#endif /* N_E_MODULE_H */
