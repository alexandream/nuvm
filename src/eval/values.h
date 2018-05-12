#ifndef N_E_VALUES_H
#define N_E_VALUES_H

#include "../common/compatibility/stdint.h"
#include "../common/errors.h"

typedef struct NObject NObject;
typedef struct NType NType;

typedef intptr_t NValue;
typedef int32_t NFixnum;

#define N_FIXNUM_MIN ((NFixnum) -2147483648)
#define N_FIXNUM_MAX ((NFixnum)  2147483647)

#define n_eq_values(LEFT, RIGHT) ((LEFT) == (RIGHT))


struct NType {
    const char* name;
};

struct NObject {
    NType *type;
};

void
ni_init_values(NError* error);


NValue
n_wrap_object(NObject* object);

NObject*
n_unwrap_object(NValue value);

int
n_is_immediate(NValue value);


NType*
n_type_of(NValue value);





int
n_is_fixnum(NValue value);

NValue
n_wrap_fixnum(NFixnum fixnum);

NFixnum
n_unwrap_fixnum(NValue value);


#endif /* N_E_VALUES_H */

