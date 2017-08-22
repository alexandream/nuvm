#ifndef N_E_PRIMITIVES_H
#define N_E_PRIMITIVES_H

#include "../common/errors.h"

#include "values.h"

typedef struct NPrimitive NPrimitive;
typedef NValue (*NPrimitiveFunc)(int, NValue*, NError*);

struct NPrimitive {
    NObject object_header;
    NPrimitiveFunc func;
};

int
ni_init_primitives(void);

NValue
n_create_primitive(NPrimitiveFunc function, NError *error);

int
n_is_primitive(NValue);

NValue
n_call_primitive(NValue primitive, int n_args, NValue *args, NError *error);

#endif /* N_E_PRIMITIVES_H */

