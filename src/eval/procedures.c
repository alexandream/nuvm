#include "procedures.h"
#include "values.h"
#include "type-registry.h"

static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

static
NType _procedure_type;


int
ni_init_procedures(void) {
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        NError error = n_error_ok();
        if (ni_init_type_registry() < 0) {
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
        INITIALIZED = 1;
    }
    return 0;
}


NValue
n_create_procedure(NModule* module, uint32_t entry, uint8_t num_locals,
                   uint8_t max_locals, uint16_t size, NError *error) {
    NProcedure* proc_ptr = NULL;

    if (size == 0) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Procedure size must be bigger "
                    "than zero.", NULL, NULL);
        return 0;
    }

    if (max_locals < num_locals) {
        n_set_error(error, ILLEGAL_ARGUMENT, "The maximum number of locals "
                    "must be at least equal to the internal number of locals.",
                    NULL, NULL);
        return 0;
    }

    proc_ptr = malloc(sizeof(NProcedure));
    if (proc_ptr == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate procedure.",
                    NULL, NULL);
        return 0;
    }

    proc_ptr->object_header.type = &_procedure_type;
    proc_ptr->entry = entry;
    proc_ptr->num_locals = num_locals;
    proc_ptr->max_locals = max_locals;
    proc_ptr->size = size;
    return n_wrap_object((NObject*)proc_ptr);
}


int
n_is_procedure(NValue value) {
    if (!n_is_immediate(value)) {
        return ((NObject*) n_unwrap_object(value))->type == &_procedure_type;
    }
    return 0;
}
