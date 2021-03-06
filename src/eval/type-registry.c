#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include "../common/common.h"
#include "../common/errors.h"
#include "../common/name-registry.h"

#include "type-registry.h"

#ifndef N_TYPE_REGISTRY_INITIAL_SIZE
#define N_TYPE_REGISTRY_INITIAL_SIZE 32
#endif

struct NTypeRegistry {
    NNameRegistry reg;
};


static
NTypeRegistry DEFAULT_REGISTRY;


static NErrorType ERROR_TYPES[] = {
    { "nuvm.types.InvalidName" },
    { "nuvm.types.RepeatedName" },
    { "nuvm.types.UnknownType" },
    { NULL }
};

static
NErrorType* INVALID_NAME = ERROR_TYPES+0;

static
NErrorType* REPEATED_NAME = ERROR_TYPES+1;

static
NErrorType* UNKNOWN_TYPE = ERROR_TYPES+2;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

static
NErrorType* BAD_ALLOCATION = NULL;

static void
construct_registry(NTypeRegistry* self, NError* error);

static NType*
find_type(NTypeRegistry* self, const char* name, NError* error);

static void
register_type(NTypeRegistry* self, NType* type, NError* error);


void
ni_init_type_registry(NError* error) {
#define EC ON_ERROR(error, return)
	construct_registry(&DEFAULT_REGISTRY, error);                     EC;

	{
		NErrorType* next_type = ERROR_TYPES;
		while(next_type->name != NULL) {
			n_register_error_type(next_type, error);                  EC;
			next_type++;
		}
	}

	ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", error);   EC;
	BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);       EC;
#undef EC
}


void
n_construct_type(NType* type, const char* name) {
    type->name = name;
}


NType*
n_find_type(const char* name, NError* error) {
    return find_type(&DEFAULT_REGISTRY, name, error);
}


void
n_register_type(NType* type, NError* error) {
    register_type(&DEFAULT_REGISTRY, type, error);
}


#ifdef N_TEST
static void
destruct_registry(NTypeRegistry* self) {
    ni_destruct_name_registry(&self->reg);
}


NTypeRegistry* nt_create_type_registry() {
    NError error = n_error_ok();
    NTypeRegistry* self = malloc(sizeof(NTypeRegistry));
    if (self == NULL) {
        return NULL;
    }

    construct_registry(self, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        free(self);
        return NULL;
    }

    return self;
}


void nt_destroy_type_registry(NTypeRegistry* registry) {
    destruct_registry(registry);
    free(registry);
}


void nt_register_type(NTypeRegistry* registry, NType* type, NError* error) {
    register_type(registry, type, error);
}


NType* nt_type(NTypeRegistry* registry, const char* name, NError* error) {
    return find_type(registry, name, error);
}


#endif /* N_TEST */

static void
construct_registry(NTypeRegistry* self, NError* error) {
    int initial_size = N_TYPE_REGISTRY_INITIAL_SIZE;
    int status = ni_construct_name_registry(&self->reg, initial_size);
    if (status < 0) {
        n_set_error(error, BAD_ALLOCATION, "Can't allocate registry pool.");
        return;
    }
}


static NType*
find_type(NTypeRegistry* self, const char* name, NError* error) {
    NType* found = (NType*) ni_find_named_object(&self->reg, name);
    if (found == NULL) {
        n_set_error(error, UNKNOWN_TYPE, "Could not find the given "
                    "name in the type registry.");
    }
    return found;
}


static void
register_type(NTypeRegistry* self, NType* type, NError* error) {
    int status;
    if (type == NULL) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Can't register a NULL type.");
        return;
    }

    status = ni_register_named_object(&self->reg, type->name, type);
    if (status == N_NAMED_REG_INVALID_NAME) {
        n_set_error(error, INVALID_NAME, "The name of a type must "
                    "be non-NULL, non-empty and contain only alphanumeric "
                    "characters and dots.");
    }
    else if (status == N_NAMED_REG_REPEATED_NAME) {
        n_set_error(error, REPEATED_NAME,"The name of a type must be unique.");
    }
    else if (status == N_NAMED_REG_BAD_ALLOCATION) {
        n_set_error(error, BAD_ALLOCATION, "Could not grow the type "
                    "registry storage area for insertion.");
    }
}
