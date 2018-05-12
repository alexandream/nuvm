#include <stdlib.h>

#include "values.h"
#include "type-registry.h"
#include "primitives.h"
#include "procedures.h"

static NType  _fixnum_type;


void
ni_init_values(NError* error) {
#define EC ON_ERROR(error, return)
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        n_construct_type(&_fixnum_type, "nuvm.Fixnum");
        n_register_type(&_fixnum_type, error);                       EC;
        INITIALIZED = 1;
    }
#undef EC
}


NValue
n_wrap_fixnum(NFixnum fixnum) {
    return ((NValue) fixnum) << 1;
}


NValue
n_wrap_object(NObject* pointer) {
    return ((NValue) pointer) | 1;
}


NFixnum
n_unwrap_fixnum(NValue value) {
    return (NFixnum) (value >> 1);
}


NObject*
n_unwrap_object(NValue value) {
    return (NObject*) (value & ~1);
}


int
n_is_fixnum(NValue value) {
    return (value & 1) == 0;
}


int
n_is_immediate(NValue value) {
    return n_is_fixnum(value);
}


NType*
n_type_of(NValue value) {
    if (n_is_fixnum(value)) {
        return &_fixnum_type;
    }
    return n_unwrap_object(value)->type;
}

