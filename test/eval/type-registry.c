#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"
#include "eval/type-registry.h"

NTypeRegistry* TR = NULL;

CONSTRUCTOR(constructor) {
	int status = ni_init_type_registry();
	if (status < 0) {
		ERROR("Cant initialize types system.", NULL);
	}
}


SETUP(setup) {
    TR = nt_create_type_registry();
    if (TR == NULL) {
        ERROR("Can't setup type registry for tests.", NULL);
    }
}


TEARDOWN(teardown) {
    if (TR != NULL) {
        nt_destroy_type_registry(TR);
    }
}


TEST(can_register_type) {
	NError error = n_error_ok();
	NType type;

	n_construct_type(&type, "foo.Type");
	nt_register_type(TR, &type, &error);

	ASSERT(IS_OK(error));
}


TEST(rejects_empty_name) {
	NError error = n_error_ok();
	NType type;

	n_construct_type(&type, "");
	nt_register_type(TR, &type, &error);

	ASSERT(IS_ERROR(error, "nuvm.types.InvalidName"));
}


TEST(rejects_null_name) {
	NError error = n_error_ok();
	NType type;

	n_construct_type(&type, NULL);
	nt_register_type(TR, &type, &error);

	ASSERT(IS_ERROR(error, "nuvm.types.InvalidName"));

}


TEST(rejects_null_type) {
	NError error = n_error_ok();

	nt_register_type(TR, NULL, &error);

	ASSERT(IS_ERROR(error, "nuvm.IllegalArgument"));
}


TEST(rejects_name_with_blanks) {
	NError error = n_error_ok();
	NType type;

	n_construct_type(&type, "foo.TypeWith aBlank");
	nt_register_type(TR, &type, &error);

	ASSERT(IS_ERROR(error, "nuvm.types.InvalidName"));
}


TEST(rejects_repeated_name) {
	NError error = n_error_ok();
	NType type1;
	NType type2;

	n_construct_type(&type1, "foo.NamedType");
	nt_register_type(TR, &type1, &error);

	ASSERT(IS_OK(error));

	n_construct_type(&type2, "foo.NamedType");
	nt_register_type(TR, &type2, &error);

	ASSERT(IS_ERROR(error, "nuvm.types.RepeatedName"));
}


TEST(can_find_by_name) {
	NError error = n_error_ok();
	NType type;
	NType *retrieved_type;

	n_construct_type(&type, "foo.Type");
	nt_register_type(TR, &type, &error);

	ASSERT(IS_OK(error));

	retrieved_type = nt_type(TR, "foo.Type", &error);

	ASSERT(IS_OK(error));
	ASSERT(EQ_PTR(retrieved_type, &type));
}


TEST(can_register_and_find_different_types) {
	NError error = n_error_ok();
	NType type1;
	NType type2;
	NType *retrieved_type;

	n_construct_type(&type1, "foo.Type1");
	nt_register_type(TR, &type1, &error);

	ASSERT(IS_OK(error));

	n_construct_type(&type2, "foo.Type2");
	nt_register_type(TR, &type2, &error);

	ASSERT(IS_OK(error));

	retrieved_type = nt_type(TR, "foo.Type1", &error);

	ASSERT(IS_OK(error));
	ASSERT(EQ_PTR(retrieved_type, &type1));
	ASSERT(EQ_STR(retrieved_type->name, type1.name));

	retrieved_type = nt_type(TR, "foo.Type2", &error);

	ASSERT(IS_OK(error));
	ASSERT(EQ_PTR(retrieved_type, &type2));
	ASSERT(EQ_STR(retrieved_type->name, type2.name));
}


AtTest* tests[] = {
	&can_register_type,
	&rejects_empty_name,
	&rejects_null_name,
	&rejects_null_type,
	&rejects_name_with_blanks,
	&rejects_repeated_name,
	&can_find_by_name,
	&can_register_and_find_different_types,
	NULL
};


TEST_RUNNER("TypeRegistry", tests, constructor, NULL, setup, teardown)
