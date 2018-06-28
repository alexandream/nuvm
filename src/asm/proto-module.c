#include "proto-module.h"
#include "proto-values.h"


/* Instantiate the vector "template" for the NProtoProcedure's instructions. */
#define VECTOR_T_CLEANUP
#include "../common/templates/vector.h"

#define VECTOR_T_STRUCT ValueVector
#define VECTOR_T_ELEMENT_T NProtoValue*
#include "../common/templates/vector.h"

typedef struct ValueVector ValueVector;
VECTOR_T_D_STRUCT;

static VECTOR_T_I_CONSTRUCT(vvec_construct)
static VECTOR_T_I_DESTRUCT(vvec_destruct)
static VECTOR_T_I_GET_REF(vvec_get_ref)
static VECTOR_T_I_PUSH(vvec_push)


struct NProtoModule {
    ValueVector values;
};


static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

void
ni_init_proto_module(NError* error) {
#define EC ON_ERROR(error, return)
    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);         EC;
    ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", error);     EC;
#undef EC
}


NProtoModule*
ni_create_proto_module(NError* error) {
    NProtoModule* result = NULL;
    int vvec_error = 0;

    result = (NProtoModule*) malloc(sizeof(NProtoModule));
    if (result == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                    "ProtoModule");
        goto clean_up;
    }
    
    vvec_construct(&result->values, 16, &vvec_error);
    if (vvec_error != 0) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                    "ProtoModule");
        goto clean_up;
    }
    return result;
 
clean_up:
        free(result);
        return NULL;
}


void
ni_destroy_proto_module(NProtoModule* self) {
    vvec_destruct(&self->values);
    free(self);
}


void
ni_add_proto_value(NProtoModule* self, NProtoValue* value, NError* error) {
    int vvec_error = 0;

    vvec_push(&self->values, &value, &vvec_error);

    if (vvec_error != 0) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space to grow "
                    "the values vector.");
        return;
    }
}


size_t
ni_proto_value_count(NProtoModule* self) {
    return self->values.size;
}


NProtoValue*
ni_get_proto_value(NProtoModule* self, size_t i, NError* error) {
    NProtoValue** result;
    int vvec_error = 0;

    result = vvec_get_ref(&self->values, i, &vvec_error);
    if (vvec_error != 0) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Index out of bounds.");
        return NULL;
    }

    return *result;
}

