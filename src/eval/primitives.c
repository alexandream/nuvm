#include "primitives.h"
#include "type-registry.h"
#include "values.h"

static
NType _primitive_type;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

static
NErrorType* BAD_ALLOCATION = NULL;

void
ni_init_primitives(NError* error) {
#define EC ON_ERROR(error, return)
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        n_construct_type(&_primitive_type, "nuvm.PrimitiveProcedure");
        n_register_type(&_primitive_type, error);                        EC;

        ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", error);  EC;
        BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);      EC;

        INITIALIZED = 1;
    }
#undef EC
}

NValue
n_create_primitive(NPrimitiveFunc function, NError *error) {
    NPrimitive *primitive;
    if (function == NULL) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Can't creat a NULL primitive.");
    }

    primitive = malloc(sizeof(NPrimitive));
    if (primitive == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Unable to allocate primitive.");
    }

    primitive->object_header.type = &_primitive_type;
    primitive->func = function;
    return n_wrap_object((NObject*) primitive);
}

int
n_is_primitive(NValue value) {
    if (!n_is_immediate(value)) {
        return ((NObject*) n_unwrap_object(value))->type == &_primitive_type;
    }
    return 0;
}

NValue
n_call_primitive(NValue primitive, int n_args, NValue *args, NError *error) {
    NPrimitiveFunc func = ((NPrimitive*) n_unwrap_object(primitive))->func;

    return func(n_args, args, error);
}
