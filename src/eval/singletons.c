#include "values.h"
#include "type-registry.h"
#include "singletons.h"

static NType _boolean_type;
static NType _unknown_type;

static
NErrorType* BAD_ALLOCATION = NULL;

NValue N_TRUE;
NValue N_FALSE;
NValue N_UNKNOWN;


void
ni_init_singletons(NError* error) {
#define EC ON_ERROR(error, return)
    n_construct_type(&_boolean_type, "nuvm.Boolean");
    n_register_type(&_boolean_type, error);                      EC;

    n_construct_type(&_unknown_type, "nuvm.Unknown");
    n_register_type(&_unknown_type, error);                      EC;

    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);  EC;

    {
        NObject *true_ptr    = malloc(sizeof(NObject));
        NObject *false_ptr   = malloc(sizeof(NObject));
        NObject *unknown_ptr = malloc(sizeof(NObject));

        if (!true_ptr || !false_ptr || !unknown_ptr) {
            free(true_ptr);
            free(false_ptr);
            free(unknown_ptr);

            n_set_error(error, BAD_ALLOCATION,
                    "Unable to allocate space for singletons");
            return;
        }

        true_ptr->type = &_boolean_type;
        false_ptr->type = &_boolean_type;
        unknown_ptr->type = &_unknown_type;

        N_TRUE = n_wrap_object(true_ptr);
        N_FALSE = n_wrap_object(false_ptr);
        N_UNKNOWN = n_wrap_object(unknown_ptr);
    }
#undef EC
}


int
n_is_boolean(NValue value) {
    return (n_eq_values(N_TRUE, value) || n_eq_values(N_FALSE, value));
}


int
n_is_unknown(NValue value) {
    return n_eq_values(N_UNKNOWN, value);
}
