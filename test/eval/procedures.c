#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"
#include "eval/type-registry.h"
#include "eval/procedures.h"

static
NValue OTHER_VALUES[5];

static
NError ERR;

CONSTRUCTOR(constructor) {
    if (ni_init_values() < 0) {
        ERROR("Can't initialize user procedures module.", NULL);
    }
    OTHER_VALUES[0] = N_TRUE;
    OTHER_VALUES[1] = N_FALSE;
    OTHER_VALUES[2] = N_UNKNOWN;
    OTHER_VALUES[3] = n_wrap_fixnum(N_FIXNUM_MIN);
    OTHER_VALUES[4] = n_wrap_fixnum(N_FIXNUM_MAX);
}


SETUP(setup) {
    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
}



TEST(procedure_type_is_registered) {
    NType* procedures_type = n_find_type("nuvm.UserProcedure", &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(procedures_type != NULL));
}


TEST(create_with_negative_entry_fails) {
    n_create_procedure(-1, 0, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}

TEST(is_procedure_detects_procedure) {
    NValue proc = n_create_procedure(0, 0, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_is_procedure(proc)));
}


AtArrayIterator other_iter = at_static_array_iterator(OTHER_VALUES);
DD_TEST(is_procedure_rejects_other, other_iter, NValue, value) {
    ASSERT(IS_TRUE(!n_is_procedure(*value)));
}


TEST(type_of_procedure_is_correct) {
    NType *proc_type;
    NValue proc = n_create_procedure(0, 0, &ERR);

    ASSERT(IS_OK(ERR));

    proc_type = n_type_of(proc);

    ASSERT(IS_TRUE(proc_type != NULL));
    ASSERT(EQ_STR(proc_type->name, "nuvm.UserProcedure"));
}


TEST(create_procedure_sets_entry) {
    NValue proc = n_create_procedure(12345, 0, &ERR);
    NProcedure *proc_ptr;
    ASSERT(IS_OK(ERR));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);
    ASSERT(EQ_INT(proc_ptr->entry, 12345));
}


TEST(create_procedure_sets_num_locals) {
    NValue proc = n_create_procedure(12345, 123, &ERR);
    NProcedure *proc_ptr;
    ASSERT(IS_OK(ERR));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);
    ASSERT(EQ_UINT(proc_ptr->num_locals, 123));
}

AtTest* tests[] = {
    &procedure_type_is_registered,
    &create_with_negative_entry_fails,
    &is_procedure_detects_procedure,
    &is_procedure_rejects_other,
    &type_of_procedure_is_correct,
    &create_procedure_sets_entry,
    &create_procedure_sets_num_locals,
    NULL
};

TEST_RUNNER("UserProcedures", tests, constructor, NULL, setup, teardown)
