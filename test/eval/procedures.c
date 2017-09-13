#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"
#include "eval/type-registry.h"
#include "eval/singletons.h"
#include "eval/procedures.h"

static
NValue OTHER_VALUES[5];

static
NError ERR;

CONSTRUCTOR(constructor) {
    if (ni_init_all_values() < 0) {
        ERROR("Can't initialize values modules.", NULL);
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


TEST(is_procedure_detects_procedure) {
    NValue proc = n_create_procedure(NULL, 0, 0, 0, 1, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_is_procedure(proc)));
}


AtArrayIterator other_iter = at_static_array_iterator(OTHER_VALUES);
DD_TEST(is_procedure_rejects_other, other_iter, NValue, value) {
    ASSERT(IS_TRUE(!n_is_procedure(*value)));
}


TEST(type_of_procedure_is_correct) {
    NType *proc_type;
    NValue proc = n_create_procedure(NULL, 0, 0, 0, 1, &ERR);

    ASSERT(IS_OK(ERR));

    proc_type = n_type_of(proc);

    ASSERT(IS_TRUE(proc_type != NULL));
    ASSERT(EQ_STR(proc_type->name, "nuvm.UserProcedure"));
}


TEST(create_procedure_sets_entry) {
    NValue proc = n_create_procedure(NULL, 12345, 0, 0, 1, &ERR);
    NProcedure *proc_ptr;
    ASSERT(IS_OK(ERR));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);
    ASSERT(EQ_UINT(proc_ptr->entry, 12345));
}


TEST(create_procedure_sets_num_locals) {
    NValue proc = n_create_procedure(NULL, 12345, 123, 123, 1, &ERR);
    NProcedure *proc_ptr;
    ASSERT(IS_OK(ERR));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);
    ASSERT(EQ_UINT(proc_ptr->num_locals, 123));
}


TEST(create_procedure_sets_max_locals) {
    NValue proc = n_create_procedure(NULL, 12345, 123, 149, 1, &ERR);
    NProcedure *proc_ptr;
    ASSERT(IS_OK(ERR));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);
    ASSERT(EQ_UINT(proc_ptr->max_locals, 149));
}


TEST(create_procedure_sets_size) {
    NValue proc = n_create_procedure(NULL, 12345, 123, 149, 1234, &ERR);
    NProcedure *proc_ptr;
    ASSERT(IS_OK(ERR));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);
    ASSERT(EQ_UINT(proc_ptr->size, 1234));

}


TEST(procedure_needs_positive_size) {
    n_create_procedure(NULL, 12345, 123, 149, 0, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));

}


TEST(max_locals_gte_num_locals) {
    n_create_procedure(NULL, 12345, 123, 122, 1, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


AtTest* tests[] = {
    &procedure_type_is_registered,
    &is_procedure_detects_procedure,
    &is_procedure_rejects_other,
    &type_of_procedure_is_correct,
    &create_procedure_sets_entry,
    &create_procedure_sets_num_locals,
    &create_procedure_sets_max_locals,
    &create_procedure_sets_size,
    &procedure_needs_positive_size,
    &max_locals_gte_num_locals,
    NULL
};

TEST_RUNNER("UserProcedures", tests, constructor, NULL, setup, teardown)
