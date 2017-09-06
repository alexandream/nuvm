#include "values.h"
#include "type-registry.h"
#include "singletons.h"

static NType _boolean_type;
static NType _unknown_type;


NValue N_TRUE;
NValue N_FALSE;
NValue N_UNKNOWN;


int
ni_init_singletons() {
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        NError error = n_error_ok();
        if (ni_init_type_registry() < 0) {
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
            return -3;
        }

        {
            NObject *true_ptr    = malloc(sizeof(NObject));
            NObject *false_ptr   = malloc(sizeof(NObject));
            NObject *unknown_ptr = malloc(sizeof(NObject));

            if (!true_ptr || !false_ptr || !unknown_ptr) {
                return -4;
            }

            true_ptr->type = &_boolean_type;
            false_ptr->type = &_boolean_type;
            unknown_ptr->type = &_unknown_type;

            N_TRUE = n_wrap_object(true_ptr);
            N_FALSE = n_wrap_object(false_ptr);
            N_UNKNOWN = n_wrap_object(unknown_ptr);
        }
        INITIALIZED = 1;
    }
    return 0;
}   


int
n_is_boolean(NValue value) {
    return (n_eq_values(N_TRUE, value) || n_eq_values(N_FALSE, value));
}


int
n_is_unknown(NValue value) {
    return n_eq_values(N_UNKNOWN, value);
}
