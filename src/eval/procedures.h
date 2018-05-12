#ifndef N_E_PROCEDURES_H
#define N_E_PROCEDURES_H

#include "../common/errors.h"
#include "values.h"
#include "modules.h"

typedef struct NProcedure NProcedure;

struct NProcedure {
    NObject object_header;
    NModule* module;
    uint32_t entry;
    uint8_t num_locals;
    uint8_t max_locals;
    uint16_t size;
};

void
ni_init_procedures(NError* error);

NValue
n_create_procedure(NModule* module, uint32_t entry, uint8_t num_locals,
                   uint8_t max_locals, uint16_t size, NError *error);

int
n_is_procedure(NValue);

#endif /* N_E_PROCEDURES_H */
