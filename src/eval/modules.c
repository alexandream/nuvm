#include "../common/common.h"

#include "modules.h"

static
NErrorType* BAD_ALLOCATION = NULL;

int
ni_init_modules(void) {
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        NError error = n_error_ok();

        n_init_common(&error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
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
n_create_module(uint16_t num_globals, uint32_t code_size, NError *error) {
    NModule *self = malloc(sizeof(NModule));
    if (self == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate module");
        goto clean_up;
    }
    self->code = NULL;
    self->globals = NULL;

    self->code = malloc(sizeof(unsigned char) * code_size);
    if (self->code == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate module");
        goto clean_up;
    }

    self->globals = malloc(sizeof(NValue) * num_globals);
    if (self->globals == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate module");
        goto clean_up;
    }

    self->code_size = code_size;
    self->num_globals = num_globals;
    self->entry_point = 0;
    return self;

clean_up:
    n_destroy_module(self);
    return NULL;
}


void
n_destroy_module(NModule* self) {
    if (self != NULL) {
        if (self->code != NULL) {
            free(self->code);
        }
        if (self->globals != NULL) {
            free(self->globals);
        }

        free(self);
    }
}
