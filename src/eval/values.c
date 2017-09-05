#include <stdlib.h>

#include "values.h"
#include "type-registry.h"
#include "primitives.h"
#include "procedures.h"

static NType _boolean_type;
static NType _unknown_type;
static NType  _fixnum_type;

NValue N_TRUE;
NValue N_FALSE;
NValue N_UNKNOWN;

int
ni_init_values() {
    NError error = n_error_ok();

    if (ni_init_type_registry() < 0) {
        return -1;
    }

    if (ni_init_primitives() < 0) {
        return -2;
    }

    if (ni_init_procedures() < 0) {
        return -3;
    }

    n_construct_type(&_boolean_type, "nuvm.Boolean");
    n_register_type(&_boolean_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -4;
    }

    n_construct_type(&_unknown_type, "nuvm.Unknown");
    n_register_type(&_unknown_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -4;
    }

    n_construct_type(&_fixnum_type, "nuvm.Fixnum");
    n_register_type(&_fixnum_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -5;
    }

    {
        NObject *true_ptr    = malloc(sizeof(NObject));
        NObject *false_ptr   = malloc(sizeof(NObject));
        NObject *unknown_ptr = malloc(sizeof(NObject));

        if (!true_ptr || !false_ptr || !unknown_ptr) {
            return -5;
        }

        true_ptr->type = &_boolean_type;
        false_ptr->type = &_boolean_type;
        unknown_ptr->type = &_unknown_type;

        N_TRUE = n_wrap_object(true_ptr);
        N_FALSE = n_wrap_object(false_ptr);
        N_UNKNOWN = n_wrap_object(unknown_ptr);
    }

    return 0;
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

