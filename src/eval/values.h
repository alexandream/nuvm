#ifndef N_E_VALUES_H
#define N_E_VALUES_H

#include "../common/compatibility/stdint.h"
#include "types.h"

typedef struct NObject NObject;

typedef intptr_t NValue;
typedef short NFixnum;

extern NValue N_TRUE;
extern NValue N_FALSE;
extern NValue N_UNKNOWN;

#define N_FIXNUM_MIN ((NFixnum) -32768)
#define N_FIXNUM_MAX ((NFixnum)  32767)

struct NObject {
    NType *type;
};

int
ni_init_values(void);

#define n_wrap_boolean(STT) (STT? N_TRUE : N_FALSE)

#define n_unwrap_boolean(VAL) (VAL == N_TRUE ? 1 : 0)

NValue
n_wrap_fixnum(NFixnum fixnum);

NValue
n_wrap_pointer(NObject* pointer);

NFixnum
n_unwrap_fixnum(NValue value);

NObject*
n_unwrap_pointer(NValue value);

int
n_is_unknown(NValue value);

int
n_is_boolean(NValue value);

int
n_is_fixnum(NValue value);

int
n_is_pointer(NValue value);

NType*
n_type_of(NValue value);

#define n_eq_values(LEFT, RIGHT) ((LEFT) == (RIGHT))
#endif /* N_E_VALUES_H */

