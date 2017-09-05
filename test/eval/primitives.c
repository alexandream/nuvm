#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"
#include "eval/type-registry.h"
#include "eval/primitives.h"


static NError ERR;
static int FLAG;
static NValue OTHER_VALUES[5];
static NValue ALL_FALSE[3];
static NValue FLAG_PRIMITIVE;
static NValue SECOND_FLAG_PRIMITIVE;
static NValue COPY_PRIMITIVE;


static NValue
flag_func(int n_args, NValue *args, NError *error);

static NValue
copy_func(int n_args, NValue *args, NError *error);



CONSTRUCTOR(constructor) {
    if (ni_init_values() < 0) {
        ERROR("Can't initialize primitive procedures module.", NULL);
    }

    OTHER_VALUES[0] = N_TRUE;
    OTHER_VALUES[1] = N_FALSE;
    OTHER_VALUES[2] = N_UNKNOWN;
    OTHER_VALUES[3] = n_wrap_fixnum(N_FIXNUM_MIN);
    OTHER_VALUES[4] = n_wrap_fixnum(N_FIXNUM_MAX);

    FLAG_PRIMITIVE = n_create_primitive(flag_func, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create flag primitive.", NULL);
    }

    SECOND_FLAG_PRIMITIVE = n_create_primitive(flag_func, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create second flag primitive.", NULL);
    }

    COPY_PRIMITIVE = n_create_primitive(copy_func, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create copy primitive.", NULL);
    }
}

SETUP(setup) {
    ERR = n_error_ok();
    FLAG = 0;

    ALL_FALSE[0] = N_FALSE;
    ALL_FALSE[1] = N_FALSE;
    ALL_FALSE[2] = N_FALSE;
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
}



TEST(primitive_type_is_registered) {
    NType* primitives_type = n_find_type("nuvm.PrimitiveProcedure", &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(primitives_type != NULL));
}


TEST(create_primitive_from_null_fails) {
    n_create_primitive(NULL, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(is_primitive_detects_primitive) {
    ASSERT(IS_TRUE(n_is_primitive(FLAG_PRIMITIVE)));
    ASSERT(IS_TRUE(n_is_primitive(COPY_PRIMITIVE)));
}


AtArrayIterator other_iter = at_static_array_iterator(OTHER_VALUES);
DD_TEST(is_primitive_rejects_other, other_iter, NValue, value) {
    ASSERT(IS_TRUE(!n_is_primitive(*value)));
}


TEST(type_of_primitive_is_correct) {
    NType *flag_type = n_type_of(FLAG_PRIMITIVE);
    NType *copy_type = n_type_of(COPY_PRIMITIVE);

    ASSERT(IS_TRUE(flag_type != NULL));
    ASSERT(EQ_STR(flag_type->name, "nuvm.PrimitiveProcedure"));
    ASSERT(IS_TRUE(copy_type != NULL));
    ASSERT(EQ_STR(copy_type->name, "nuvm.PrimitiveProcedure"));
}


TEST(same_func_primitives_are_not_eq) {
    ASSERT(IS_TRUE(!n_eq_values(FLAG_PRIMITIVE, SECOND_FLAG_PRIMITIVE)));
}


TEST(diff_func_primitives_are_not_eq) {
    ASSERT(IS_TRUE(!n_eq_values(FLAG_PRIMITIVE, COPY_PRIMITIVE)));
}


TEST(call_primitive_calls_flag_func) {
    NValue result = n_call_primitive(FLAG_PRIMITIVE, 0, NULL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(result, N_TRUE)));
    ASSERT(EQ_INT(FLAG, 1));
}


TEST(call_primitive_calls_copy_func) {
    NValue result;
    NValue args[3];

    args[0] = N_FALSE;
    args[1] = N_TRUE;
    args[2] = N_UNKNOWN;

    result = n_call_primitive(COPY_PRIMITIVE, 3, args, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(result, N_TRUE)));
    ASSERT(IS_TRUE(n_eq_values(ALL_FALSE[0], N_FALSE)));
    ASSERT(IS_TRUE(n_eq_values(ALL_FALSE[1], N_TRUE)));
    ASSERT(IS_TRUE(n_eq_values(ALL_FALSE[2], N_UNKNOWN)));
}




AtTest* tests[] = {
    &primitive_type_is_registered,
    &create_primitive_from_null_fails,
    &is_primitive_detects_primitive,
    &is_primitive_rejects_other,
    &type_of_primitive_is_correct,
    &same_func_primitives_are_not_eq,
    &diff_func_primitives_are_not_eq,
    &call_primitive_calls_flag_func,
    &call_primitive_calls_copy_func,
    NULL
};


TEST_RUNNER("PrimitiveProcedures", tests, constructor, NULL, setup, teardown)


static NValue
flag_func(int n_args, NValue *args, NError *error) {
    FLAG = 1;
    return N_TRUE;
}


static NValue
copy_func(int n_args, NValue *args, NError *error) {
    ALL_FALSE[0] = args[0];
    ALL_FALSE[1] = args[1];
    ALL_FALSE[2] = args[2];

    return N_TRUE;
}
