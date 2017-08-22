#ifndef N_E_VALUES_H
#define N_E_VALUES_H

#include "../common/compatibility/stdint.h"

typedef struct NObject NObject;
typedef struct NType NType;

typedef intptr_t NValue;
typedef short NFixnum;

extern NValue N_TRUE;
extern NValue N_FALSE;
extern NValue N_UNKNOWN;

#define N_FIXNUM_MIN ((NFixnum) -32768)
#define N_FIXNUM_MAX ((NFixnum)  32767)

#define n_eq_values(LEFT, RIGHT) ((LEFT) == (RIGHT))


struct NType {
    const char* name;
};

struct NObject {
    NType *type;
};

int
ni_init_values(void);

NValue
n_wrap_object(NObject* object);

NObject*
n_unwrap_object(NValue value);

int
n_is_immediate(NValue value);


NType*
n_type_of(NValue value);





int
n_is_unknown(NValue value);

int
n_is_boolean(NValue value);


int
n_is_fixnum(NValue value);

NValue
n_wrap_fixnum(NFixnum fixnum);

NFixnum
n_unwrap_fixnum(NValue value);


#define n_wrap_boolean(STT) (STT? N_TRUE : N_FALSE)

#define n_unwrap_boolean(VAL) (VAL == N_TRUE ? 1 : 0)

#endif /* N_E_VALUES_H */

