#include "procedures.h"
#include "values.h"

static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

static
NType _procedure_type;


int
ni_init_procedures(void) {
    NError error = n_error_ok();
    if (ni_init_types() < 0) {
        return -1;
    }

    n_construct_type(&_procedure_type, "nuvm.UserProcedure");
    n_register_type(&_procedure_type, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -3;
    }

    ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -4;
    }
    return 0;
}


NValue
n_create_procedure(int entry, NError *error) {
    NProcedure* proc_ptr = NULL;
    if (entry < 0) {
        n_set_error(error, ILLEGAL_ARGUMENT,
                    "Entry points must be non-negative", NULL, NULL);
    }

    proc_ptr = malloc(sizeof(NProcedure));
    if (proc_ptr == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate procedure.",
                    NULL, NULL);
    }

    proc_ptr->object_header.type = &_procedure_type;
    proc_ptr->entry = entry;

    return n_wrap_pointer((NObject*)proc_ptr);
}


int
n_is_procedure(NValue value) {
    if (n_is_pointer(value)) {
        return ((NObject*) n_unwrap_pointer(value))->type == &_procedure_type;
    }
    return 0;
}
