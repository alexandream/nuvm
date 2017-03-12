#include <stdlib.h>

#include "values.h"

static NType _boolean_type;
static NType _unknown_type;
static NType  _fixnum_type;

NValue N_TRUE;
NValue N_FALSE;
NValue N_UNKNOWN;

int
ni_init_values() {
    NError error = n_error_ok();

    if (ni_init_types() < 0) {
        return -1;
    }

    n_construct_type(&_boolean_type, "nuvm.Boolean");
    n_register_type(&_boolean_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    n_construct_type(&_unknown_type, "nuvm.Unknown");
    n_register_type(&_unknown_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    n_construct_type(&_fixnum_type, "nuvm.Fixnum");
    n_register_type(&_fixnum_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    {
        NObject *true_ptr    = malloc(sizeof(NObject));
        NObject *false_ptr   = malloc(sizeof(NObject));
        NObject *unknown_ptr = malloc(sizeof(NObject));

        if (!true_ptr || !false_ptr || !unknown_ptr) {
            return -3;
        }

        true_ptr->type = &_boolean_type;
        false_ptr->type = &_boolean_type;
        unknown_ptr->type = &_unknown_type;

        N_TRUE = n_wrap_pointer(true_ptr);
        N_FALSE = n_wrap_pointer(false_ptr);
        N_UNKNOWN = n_wrap_pointer(unknown_ptr);
    }

    return 0;
}


NValue
n_wrap_fixnum(NFixnum fixnum) {
    return ((NValue) fixnum) << 1;
}


NValue
n_wrap_pointer(NObject* pointer) {
    return ((NValue) pointer) | 1;
}


NFixnum
n_unwrap_fixnum(NValue value) {
    return (NFixnum) (value >> 1);
}


NObject*
n_unwrap_pointer(NValue value) {
    return (NObject*) (value & ~1);
}

int
n_is_boolean(NValue value) {
    return (n_eq_values(N_TRUE, value) || n_eq_values(N_FALSE, value));
}


int
n_is_unknown(NValue value) {
    return n_eq_values(N_UNKNOWN, value);
}


int
n_is_fixnum(NValue value) {
    return (value & 1) == 0;
}


NType*
n_type_of(NValue value) {
    return n_unwrap_pointer(value)->type;
}

