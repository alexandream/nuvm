#ifndef N_E_PROCEDURES_H
#define N_E_PROCEDURES_H

#include "values.h"

typedef struct NProcedure NProcedure;

struct NProcedure {
    NObject object_header;
    int entry;
};

int
ni_init_procedures(void);

NValue
n_create_procedure(int entry, NError *error);

int
n_is_procedure(NValue);

#endif /* N_E_PROCEDURES_H */

