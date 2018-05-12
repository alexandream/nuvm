#ifndef N_E_SINGLETONS_H
#define N_E_SINGLETONS_H

#include "values.h"

extern NValue N_TRUE;
extern NValue N_FALSE;
extern NValue N_UNKNOWN;

void
ni_init_singletons(NError* error);

int
n_is_unknown(NValue value);

int
n_is_boolean(NValue value);

#define n_wrap_boolean(STT) (STT? N_TRUE : N_FALSE)

#define n_unwrap_boolean(VAL) (VAL == N_TRUE ? 1 : 0)

#endif /*N_E_SINGLETONS_H*/
