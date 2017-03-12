#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"

NErrorRegistry* ER = NULL;

CONSTRUCTOR(constructor) {
    if (ni_init_errors() < 0) {
        ERROR("Can't initialize error system.", NULL);
    }
}


SETUP(setup) {
    ER = nt_create_error_registry();
    if (ER == NULL) {
        ERROR("Can't setup error registry for tests.", NULL);
    }
}


TEARDOWN(teardown) {
    if (ER != NULL) {
        nt_destroy_error_registry(ER);
    }
}




TEST(ok_constructor_is_actual_ok_result) {
	NError ok = n_error_ok();
	ASSERT(IS_OK(ok));
}


TEST(can_register_error_type) {
    NError error = n_error_ok();
    NErrorType type;

    n_construct_error_type(&type, "foo.bar.NewError");
    nt_register_error_type(ER, &type, &error);

    ASSERT(IS_OK(error));
}


TEST(rejects_null_name) {
    NError error = n_error_ok();
    NErrorType type;

    n_construct_error_type(&type, NULL);
    nt_register_error_type(ER, &type, &error);

    ASSERT(IS_ERROR(error, "nuvm.error.InvalidName"));
}


TEST(rejects_null_type) {
	NError error = n_error_ok();

	nt_register_error_type(ER, NULL, &error);
	ASSERT(IS_ERROR(error, "nuvm.IllegalArgument"));
}


TEST(can_register_and_find_different_types) {
	NError error = n_error_ok();
    NErrorType type1;
    NErrorType type2;
	NErrorType* type_retrieved;

    n_construct_error_type(&type1, "foo.ErrorX");
    nt_register_error_type(ER, &type1, &error);

    ASSERT(IS_OK(error));

    n_construct_error_type(&type2, "foo.ErrorY");
    nt_register_error_type(ER, &type2, &error);

	ASSERT(IS_OK(error));

    type_retrieved = nt_error_type(ER, "foo.ErrorX", &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_PTR(type_retrieved, &type1));
	ASSERT(EQ_STR(type_retrieved->name, type1.name));

    type_retrieved = nt_error_type(ER, "foo.ErrorY", &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_PTR(type_retrieved, &type2));
	ASSERT(EQ_STR(type_retrieved->name, type2.name));
}


TEST(rejects_empty_name) {
    NError error = n_error_ok();
    NErrorType type;

    n_construct_error_type(&type, "");
    nt_register_error_type(ER, &type, &error);

    ASSERT(IS_ERROR(error, "nuvm.error.InvalidName"));
}


TEST(rejects_blank_name) {
    NError error = n_error_ok();
    NErrorType type;

    n_construct_error_type(&type, " \n \t \f");
    nt_register_error_type(ER, &type, &error);

    ASSERT(IS_ERROR(error, "nuvm.error.InvalidName"));
}


TEST(rejects_repeated_name) {
    NError error = n_error_ok();
    NErrorType type1;
    NErrorType type2;

    n_construct_error_type(&type1, "foo.Error");
    nt_register_error_type(ER, &type1, &error);

    ASSERT(IS_OK(error));

    n_construct_error_type(&type2, "foo.Error");
    nt_register_error_type(ER, &type2, &error);

    ASSERT(IS_ERROR(error, "nuvm.error.RepeatedName"));
}


TEST(can_find_by_name) {
    NError error = n_error_ok();
    NErrorType  type;
    NErrorType *type_retrieved;

    n_construct_error_type(&type, "foo.Error");
    nt_register_error_type(ER, &type, &error);

    ASSERT(IS_OK(error));

    type_retrieved = nt_error_type(ER, "foo.Error", &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_PTR(type_retrieved, &type));
	ASSERT(EQ_STR(type_retrieved->name, type.name));
}


TEST(cant_find_unknown_name) {
    NError error = n_error_ok();

    NErrorType* error_type = nt_error_type(ER, "foo.DidntRegister", &error);

    ASSERT(IS_ERROR(error, "nuvm.error.UnknownErrorType"));
    ASSERT(IS_NULL(error_type));
}



AtTest* tests[] = {
	&ok_constructor_is_actual_ok_result,
	&can_register_error_type,
	&rejects_null_name,
	&rejects_null_type,
	&rejects_empty_name,
	&rejects_blank_name,
	&rejects_repeated_name,
	&can_find_by_name,
	&cant_find_unknown_name,
	&can_register_and_find_different_types,
	NULL
};


TEST_RUNNER("Errors", tests, constructor, NULL, setup, teardown)
