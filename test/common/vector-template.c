#include "../test.h"

#define VECTOR_T_CLEANUP
#include "common/templates/vector.h"

#define VECTOR_T_STRUCT NVector
#define VECTOR_T_ELEMENT_T int

#include "common/templates/vector.h"

typedef struct NVector NVector;


VECTOR_T_D_STRUCT;

static VECTOR_T_I_CONSTRUCT(construct_vector)

static VECTOR_T_I_DESTRUCT(destruct_vector)

static VECTOR_T_I_GET_REF(vector_get_ref)

static VECTOR_T_I_SET(vector_set)

static VECTOR_T_I_SET_SIZE(vector_set_size)

static VECTOR_T_I_PUSH(vector_push)


static NVector vector_storage, *vector = &vector_storage;
static int error;

SETUP(setup) {
    error = 0;
    construct_vector(vector, 3, &error);
    if (error) {
        ERROR("Could not construct vector for test.", NULL);
    }

    error = 0;
}


TEARDOWN(teardown) {
    destruct_vector(vector);
}



TEST(construct_sets_all_properties) {
    ASSERT(IS_TRUE(vector->pool != NULL));
    ASSERT(EQ_UINT(vector->size, 0));
    ASSERT(EQ_UINT(vector->capacity, 3));
}


TEST(set_size_rejects_above_capacity) {
    vector_set_size(vector, 4, &error);
    ASSERT(IS_TRUE(error));
}

TEST(set_size_changes_size) {
    vector_set_size(vector, 3, &error);
    ASSERT(IS_TRUE(!error));

    ASSERT(EQ_UINT(vector->size, 3));
}


TEST(get_ref_returns_what_was_set) {
    int input = 12345;
    int *ref;

    vector_set_size(vector, 3, &error);
    ASSERT(IS_TRUE(!error));

    vector_set(vector, 1, &input, &error);
    ASSERT(IS_TRUE(!error));

    ref = vector_get_ref(vector, 1, &error);
    ASSERT(IS_TRUE(!error));

    ASSERT(EQ_INT(*ref, 12345));
}


TEST(get_ref_returns_what_was_pushed) {
    int input = 54321;
    int *ref;

    vector_push(vector, &input, &error);
    ASSERT(IS_TRUE(!error));

    ref = vector_get_ref(vector, 0, &error);
    ASSERT(IS_TRUE(!error));

    ASSERT(EQ_INT(*ref, input));
}


TEST(push_to_capacity_adds_only_size) {
    size_t orig_size = vector->size;
    size_t orig_capacity = vector->capacity;
    size_t i = 0;

    int input = 42;

    for (i = orig_size; i < vector->capacity; i++) {
        vector_push(vector, &input, &error);
        ASSERT(IS_TRUE(!error));
    }

    ASSERT(EQ_UINT(vector->capacity, orig_capacity));
    ASSERT(IS_TRUE(vector->size > orig_size));
}


TEST(push_past_capacity_grows_vector) {
    size_t orig_size = vector->size;
    size_t orig_capacity = vector->capacity;
    size_t i = 0;

    int input = 42;

    for (i = orig_size; i <= orig_capacity; i++) {
        vector_push(vector, &input, &error);
        ASSERT(IS_TRUE(!error));
    }

    ASSERT(IS_TRUE(vector->capacity > orig_capacity));
    ASSERT(IS_TRUE(vector->size > orig_size));

}

AtTest* tests[] = {
    &construct_sets_all_properties,
    &set_size_rejects_above_capacity,
    &set_size_changes_size,
    &get_ref_returns_what_was_set,
    &get_ref_returns_what_was_pushed,
    &push_to_capacity_adds_only_size,
    &push_past_capacity_grows_vector,
    NULL
};

TEST_RUNNER("VectorTemplate", tests, NULL, NULL, setup, teardown)
