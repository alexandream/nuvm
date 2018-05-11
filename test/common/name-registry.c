#include <stdlib.h>

#include "../test.h"

#include "common/common.h"
#include "common/name-registry.h"

NNameRegistry NR;

CONSTRUCTOR(constructor) {
    NT_INITIALIZE_MODULE(n_init_common);
}


SETUP(setup) {
    int status = ni_construct_name_registry(&NR, 2);
    if (status != N_NAMED_REG_SUCCESS) {
        ERROR("Can't setup name registry for tests.", NULL);
    }
}


TEARDOWN(teardown) {
    ni_destruct_name_registry(&NR);
}



TEST(can_register_name_object) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name = "foo.Name";

    status = ni_register_named_object(&NR, name, name);
    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));
}


TEST(rejects_null_name) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name = NULL;

    status = ni_register_named_object(&NR, name, name);

    ASSERT(EQ_INT(status, N_NAMED_REG_INVALID_NAME));
}


TEST(can_register_and_find_different_names) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name1 = "foo.Name";
    const char* name2 = "bar.Name";
    const char* name_retrieved;

    status = ni_register_named_object(&NR, name1, name1);

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));

    status = ni_register_named_object(&NR, name2, name2);

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));

    name_retrieved = ni_find_named_object(&NR, "foo.Name");

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));
    ASSERT(EQ_PTR(name_retrieved, name1));

    name_retrieved = ni_find_named_object(&NR, "bar.Name");

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));
    ASSERT(EQ_PTR(name_retrieved, name2));
}


TEST(can_register_and_find_more_than_initial_size) {
    int status;
    const char* name_retrieved;
    const char*  names[] = { "name1", "name2", "name3", "name4" };
    int i;


    /* Insert all 4. */
    for (i = 0; i < 4; i++) {
        status = ni_register_named_object(&NR, names[i], names[i]);
        ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));
    }

    for (i = 0; i < 4; i++) {
        name_retrieved = ni_find_named_object(&NR, names[i]);

        ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));
        ASSERT(EQ_PTR(name_retrieved, names[i]));
    }
}
TEST(rejects_empty_name) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name = "";

    status = ni_register_named_object(&NR, name, name);
    ASSERT(EQ_INT(status, N_NAMED_REG_INVALID_NAME));
}


TEST(rejects_blank_name) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name = "    ";

    status = ni_register_named_object(&NR, name, name);
    ASSERT(EQ_INT(status, N_NAMED_REG_INVALID_NAME));
}


TEST(rejects_invalid_name) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name = "invalid-name";

    status = ni_register_named_object(&NR, name, name);
    ASSERT(EQ_INT(status, N_NAMED_REG_INVALID_NAME));
}


TEST(rejects_repeated_name) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name1 = "foo.Name";
    const char* name2 = "foo.Name";

    status = ni_register_named_object(&NR, name1, name1);

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));

    status = ni_register_named_object(&NR, name2, name2);

    ASSERT(EQ_INT(status, N_NAMED_REG_REPEATED_NAME));
}


TEST(can_find_by_name) {
    int status = N_NAMED_REG_SUCCESS;
    const char* name = "foo.Name";
    const char* name_retrieved;

    status = ni_register_named_object(&NR, name, name);

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));

    name_retrieved = ni_find_named_object(&NR, "foo.Name");

    ASSERT(EQ_INT(status, N_NAMED_REG_SUCCESS));
    ASSERT(EQ_PTR(name_retrieved, name));
}


TEST(cant_find_unknown_name) {
    const char* name_obj = ni_find_named_object(&NR, "foo.Name");
    ASSERT(IS_NULL(name_obj));
}



AtTest* tests[] = {
    &can_register_name_object,
    &rejects_null_name,
    &rejects_empty_name,
    &rejects_blank_name,
    &rejects_invalid_name,
    &rejects_repeated_name,
    &can_find_by_name,
    &cant_find_unknown_name,
    &can_register_and_find_different_names,
    &can_register_and_find_more_than_initial_size,
    NULL
};


TEST_RUNNER("NameRegistry", tests, constructor, NULL, setup, teardown)
