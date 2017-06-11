#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"
#include "eval/values.h"

CONSTRUCTOR(constructor) {
    if (ni_init_values() < 0) {
        ERROR("Can't initialize values module.", NULL);
    }
}

TEST(boolean_type_is_registered) {
    NError error = n_error_ok();
    NType* boolean_type = n_find_type("nuvm.Boolean", &error);
    ASSERT(IS_TRUE(boolean_type != NULL));
    ASSERT(IS_OK(error));
}


TEST(unknown_type_is_registered) {
    NError error = n_error_ok();
    NType* boolean_type = n_find_type("nuvm.Unknown", &error);
    ASSERT(IS_TRUE(boolean_type != NULL));
    ASSERT(IS_OK(error));
}


TEST(false_not_equal_true) {
    ASSERT(IS_TRUE(!n_eq_values(N_TRUE, N_FALSE)));
}


TEST(true_equal_true) {
    ASSERT(IS_TRUE(n_eq_values(N_TRUE, N_TRUE)));
}


TEST(false_equal_false) {
    ASSERT(IS_TRUE(n_eq_values(N_TRUE, N_TRUE)));
}


TEST(wrap_boolean_false_is_false) {
    NValue false_val = n_wrap_boolean(0);
    ASSERT(IS_TRUE(n_eq_values(false_val, N_FALSE)));
}


TEST(wrap_boolean_true_is_true) {
    NValue true_val = n_wrap_boolean(1);
    ASSERT(IS_TRUE(n_eq_values(true_val, N_TRUE)));
}


TEST(boolean_true_is_boolean) {
    ASSERT(IS_TRUE(n_is_boolean(N_TRUE)));
}


TEST(boolean_false_is_boolean) {
    ASSERT(IS_TRUE(n_is_boolean(N_FALSE)));
}


TEST(boolean_true_is_not_unknown) {
    ASSERT(IS_TRUE(!n_is_unknown(N_TRUE)));
}


TEST(boolean_false_is_not_unknown) {
    ASSERT(IS_TRUE(!n_is_unknown(N_FALSE)));
}


TEST(unknown_is_unknown) {
    ASSERT(IS_TRUE(n_is_unknown(N_UNKNOWN)));
}


TEST(unknown_is_not_boolean) {
    ASSERT(IS_TRUE(!n_is_boolean(N_UNKNOWN)));
}


TEST(boolean_true_has_boolean_type) {
    ASSERT(EQ_STR(n_type_of(N_TRUE)->name, "nuvm.Boolean"));
}


TEST(boolean_false_has_boolean_type) {
    ASSERT(EQ_STR(n_type_of(N_FALSE)->name, "nuvm.Boolean"));
}


TEST(unknown_has_unknown_type) {
    ASSERT(EQ_STR(n_type_of(N_UNKNOWN)->name, "nuvm.Unknown"));
}


AtTest* tests[] = {
    &boolean_type_is_registered,
    &unknown_type_is_registered,
    &false_not_equal_true,
    &true_equal_true,
    &false_equal_false,
    &wrap_boolean_false_is_false,
    &wrap_boolean_true_is_true,
    &boolean_true_is_boolean,
    &boolean_false_is_boolean,
    &boolean_true_is_not_unknown,
    &boolean_false_is_not_unknown,
    &unknown_is_unknown,
    &unknown_is_not_boolean,
    &boolean_true_has_boolean_type,
    &boolean_false_has_boolean_type,
    &unknown_has_unknown_type,
    NULL
};


TEST_RUNNER("Singletons", tests, constructor, NULL, NULL, NULL)

