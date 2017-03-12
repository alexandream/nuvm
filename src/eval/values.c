#include <stdlib.h>

#include "values.h"

static NType _boolean_type;
static NType _unknown_type;

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
	
    N_TRUE = malloc(sizeof(NObject));
    N_FALSE = malloc(sizeof(NObject));
	N_UNKNOWN = malloc(sizeof(NObject));

	if (!N_TRUE || !N_FALSE || !N_UNKNOWN) {
		return -3;
	}

	((NObject*)    N_TRUE)->type = &_boolean_type;
	((NObject*)   N_FALSE)->type = &_boolean_type;
	((NObject*) N_UNKNOWN)->type = &_unknown_type;

    return 0;
}


int n_is_boolean(NValue value) {
    return (n_eq_values(N_TRUE, value) || n_eq_values(N_FALSE, value));
}


int n_is_unknown(NValue value) {
    return n_eq_values(N_UNKNOWN, value);
}

NType*
n_type_of(NValue value) {
	return ((NObject*) value)->type;
}

