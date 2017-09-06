
#include "modules.h"

static
NErrorType* BAD_ALLOCATION = NULL;

int
ni_init_modules(void) {
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        NError error = n_error_ok();

        if (ni_init_errors() < 0) {
            return -1;
        }

        BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", &error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -2;
        }
        INITIALIZED = 1;
    }

    return 0;
}


NModule*
n_new_module(NError *error) {
    NModule *self = malloc(sizeof(NModule));
    if (self == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate module",
                    NULL, NULL);
        return NULL;
    }
    self->code = NULL;
    self->code_size = 0;
    self->registers = NULL;
    self->num_registers = 0;
    return self;
}
