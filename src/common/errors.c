#include "errors.h"
#include "name-registry.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef N_ERROR_REGISTRY_INITIAL_SIZE
#define N_ERROR_REGISTRY_INITIAL_SIZE 32
#endif

struct NErrorRegistry {
    NNameRegistry reg;
};

static
NErrorType ERROR_TYPES[] = {
    { "nuvm.BadAllocation" },
    { "nuvm.IllegalArgument" },
    { "nuvm.UnexpectedEoF" },
    { "nuvm.error.InvalidName" },
    { "nuvm.error.RepeatedName" },
    { "nuvm.error.UnknownErrorType" },
    { "nuvm.InvalidModuleFormat" },
    { "nuvm.Overflow" },
    { "nuvm.IoError" },
    { "nuvm.UnexpectedToken" },
    { NULL }
};

static
NErrorType* BAD_ALLOCATION = ERROR_TYPES+0;

static
NErrorType* ILLEGAL_ARGUMENT = ERROR_TYPES+1;

static
NErrorType* INVALID_NAME = ERROR_TYPES+3;

static
NErrorType* REPEATED_NAME = ERROR_TYPES+4;

static
NErrorType* UNKNOWN_ERROR_TYPE = ERROR_TYPES+5;

static
NErrorRegistry DEFAULT_REGISTRY;

static
int INITIALIZED = 0;





static void
construct_registry(NErrorRegistry* self, NError* error);

static NErrorType*
find_error_type(NErrorRegistry* self, const char* name, NError* error);

static void
register_error_type(NErrorRegistry* self, NErrorType* type, NError* error);


void
ni_init_errors(NError* error) {
#define EC ON_ERROR(error, return)
    if (!INITIALIZED) {
        construct_registry(&DEFAULT_REGISTRY, error);                 EC;
        {
            NErrorType* next_type = ERROR_TYPES;
            int i = 0;
            while(next_type->name != NULL) {
                n_register_error_type(next_type, error);              EC;
                next_type++;
                i++;
            }
        }
        INITIALIZED = 1;
    }
#undef EC
}


NError
n_error_ok() {
    NError result;
    result.type = NULL;
    result.message = NULL;
    return result;
}


const char*
n_error_name(NError* error) {
    return error->type != NULL ? error->type->name : "nuvm.Ok";
}

void
n_destroy_error(NError* self) {
    if (self->type) {
        if (self->clean_up) {
            self->clean_up(self->type, self);
        }
    }
}


void
n_construct_error_type(NErrorType* type, const char* name) {
    type->name = name;
}


void
n_register_error_type(NErrorType* type, NError* error) {
    register_error_type(&DEFAULT_REGISTRY, type, error);
}

NErrorType*
n_error_type(const char* name, NError* error) {
    return find_error_type(&DEFAULT_REGISTRY, name, error);
}

#ifdef N_TEST

NErrorRegistry*
nt_create_error_registry() {
    NError error = n_error_ok();
    NErrorRegistry* self = malloc(sizeof(NErrorRegistry));

    if (self == NULL) {
        return NULL;
    }

    construct_registry(self, &error);
    if (!n_is_ok(&error)) {
        free(self);
        return NULL;
    }

    return self;
}


void
nt_destroy_error_registry(NErrorRegistry* self) {
    ni_destruct_name_registry(&self->reg);
    free(self);
}


void
nt_register_error_type(NErrorRegistry* registry, NErrorType* type,
                       NError* error) {
    register_error_type(registry, type, error);
}


NErrorType*
nt_error_type(NErrorRegistry* registry, const char* name, NError* error) {
    return find_error_type(registry, name, error);
}
#endif


static void
construct_registry(NErrorRegistry* self, NError* error) {
    int initial_size = N_ERROR_REGISTRY_INITIAL_SIZE;
    int status = ni_construct_name_registry(&self->reg, initial_size);
    if (status < 0) {
        n_set_error(error, BAD_ALLOCATION,
                    "Can't allocate registry pool.");
        return;
    }
}


static NErrorType*
find_error_type(NErrorRegistry* self, const char* name, NError* error) {
    NErrorType* found = (NErrorType*) ni_find_named_object(&self->reg, name);
    if (found == NULL) {
        n_set_error(error, UNKNOWN_ERROR_TYPE, "Could not find the given "
                    "name in the type registry.");
        return NULL;
    }
    return found;
}


static void
register_error_type(NErrorRegistry* self, NErrorType* type, NError* error) {
    int status;
    if (type == NULL) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Can't register a NULL type.");
        return;
    }

    status = ni_register_named_object(&self->reg, type->name, type);
    if (status == N_NAMED_REG_INVALID_NAME) {
        n_set_error(error, INVALID_NAME, "The name of an error type must "
                    "be non-NULL, non-empty and contain only alphanumeric "
                    "characters and dots.");
    }
    else if (status == N_NAMED_REG_REPEATED_NAME) {
        n_set_error(error, REPEATED_NAME, "The name of an error "
                    "type must be unique.");
    }
    else if (status == N_NAMED_REG_BAD_ALLOCATION) {
        n_set_error(error, BAD_ALLOCATION, "Could not grow the error type "
                    "registry storage area for insertion.");
    }
}

