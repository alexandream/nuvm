#include <stdlib.h>

#include "../test.h"

#include "common/errors.h"
#include "eval/values.h"

CONSTRUCTOR(constructor) {
    if (ni_init_values() < 0) {
        ERROR("Can't initialize fixnum module.", NULL);
    }
}


SETUP(setup) {

}


TEARDOWN(teardown) {

}


static NFixnum fixnums_array[] = { N_FIXNUM_MIN, -1, 0, 1, N_FIXNUM_MAX };
AtArrayIterator fixnums_iter = at_static_array_iterator(fixnums_array);


TEST(fixnum_type_is_registered) {
	NError error = n_error_ok();
	NType* fixnum_type = n_find_type("nuvm.Fixnum", &error);
	ASSERT(IS_TRUE(fixnum_type != NULL));
	ASSERT(IS_OK(error));
}


TEST(min_fixnum_lt_max_fixnum) {
    ASSERT(IS_TRUE(N_FIXNUM_MIN < N_FIXNUM_MAX));
}


TEST(min_fixnum_is_negative) {
    ASSERT(IS_TRUE(N_FIXNUM_MIN < 0));
}


TEST(max_fixnum_is_positive) {
    ASSERT(IS_TRUE(N_FIXNUM_MAX > 0));
}


DD_TEST(correctly_identifies_values, fixnums_iter, NFixnum, fixnum) {
    NValue value = n_wrap_fixnum(*fixnum);
    ASSERT(IS_TRUE(n_is_fixnum(value)));
}


DD_TEST(wrap_unwrap_is_fixnum, fixnums_iter, NFixnum, fixnum) {
    NValue wrap = n_wrap_fixnum(*fixnum);
    NFixnum unwrap = n_unwrap_fixnum(wrap);
    ASSERT(EQ_LONG(unwrap, *fixnum));
}


AtTest* tests[] = {
	&fixnum_type_is_registered,
    &min_fixnum_lt_max_fixnum,
    &min_fixnum_is_negative,
    &max_fixnum_is_positive,
    &correctly_identifies_values,
    &wrap_unwrap_is_fixnum,
    NULL
};


TEST_RUNNER("Fixnum", tests, constructor, NULL, setup, teardown)
