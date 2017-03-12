#ifndef N_E_VALUE_H
#define N_E_VALUE_H

#include "types.h"

typedef struct NObject NObject;

typedef void* NValue;

extern NValue N_TRUE;
extern NValue N_FALSE;
extern NValue N_UNKNOWN;


struct NObject {
	NType *type;
};

int
ni_init_values(void);

#define n_wrap_boolean(STT) (STT? N_TRUE : N_FALSE)

#define n_unwrap_boolean(VAL) (VAL == N_TRUE ? 1 : 0)

int
n_is_unknown(NValue value);

int
n_is_boolean(NValue value);

NType*
n_type_of(NValue value);

#define n_eq_values(LEFT, RIGHT) ((LEFT) == (RIGHT))
#endif /* N_E_VALUE_H */

