#ifndef    N_TEST_H
#define    N_TEST_H

#include "atest.h"
#include "atcheck.h"

#include "common/errors.h"

#define DESTRUCTOR(NAME)          at_destructor(NAME)
#define CONSTRUCTOR(NAME)         at_constructor(NAME)
#define SETUP(NAME)               at_setup(NAME)
#define TEARDOWN(NAME)            at_teardown(NAME)

#define TEST(NAME)                at_test(NAME)
#define DD_TEST(NM, ITER, IT, IN) at_data_driven_test(NM, ITER, IT, IN)


#define ASSERT(CHECK)        at_assert(CHECK)
#define ERROR(MSG, DISPOSE)  at_error(MSG, DISPOSE)
/* Helper functions. */

AtCheckResult
is_error(const char* expr, NError* err, const char* type_name);

AtCheckResult
is_error_type(const char* expr, NError* err, NErrorType* type_name);

/* Checks used during the tests. */
#define EQ_PTR(VALUE, EXPECTED) AT_EQ_PTR(VALUE, EXPECTED)
#define IS_NULL(VALUE) AT_IS_NULL(VALUE)
#define IS_TRUE(VALUE) AT_IS_TRUE(VALUE)

#define IS_ERROR(VALUE, ERROR)  is_error(#VALUE, &VALUE, ERROR)
#define IS_ERROR_TYPE(VAL, ERR) is_error_type(#VAL, &VAL, ERR)
#define IS_OK(VALUE)            IS_ERROR_TYPE(VALUE, NULL)
#define EQ_STR(VALUE, EXPECTED) AT_EQ_STR(VALUE, EXPECTED)

#define EQ_INT(VALUE, EXPECTED)  AT_EQ_INT(VALUE, EXPECTED)
#define EQ_UINT(VALUE, EXPECTED) AT_EQ_UINT(VALUE, EXPECTED)
#define EQ_LONG(VALUE, EXPECTED) AT_EQ_LONG(VALUE, EXPECTED)



#define TEST_RUNNER(NAME, TESTS, CR, DR, SU, TD)        \
int main(int argc, char** argv) {                       \
    AtTest** _tests = tests;                            \
    AtStreamReporter* reporter = NULL;                  \
    AtSuite* suite =                                    \
        at_new_suite(NAME, CR, DR, SU, TD);             \
                                                        \
    while (*_tests) {                                   \
        at_add_test(suite, *_tests);                    \
        _tests++;                                       \
    }                                                   \
                                                        \
    if (argc > 1) {                                     \
        const char* file_name = argv[1];                \
        reporter = at_new_stream_reporter(file_name);   \
    }                                                   \
                                                        \
    at_run_suite(suite, (AtReporter*) reporter);        \
    return 0;                                           \
}



typedef struct {
    const char* contents;
    void (*clean_up)(const char*);
} NtTestMessage;

NtTestMessage
NT_MAKE_MODULE_INIT_ERROR_MSG(NError* error);

#define NT_INITIALIZE_MODULE(MOD_INIT_FUNC) do {                   \
    NError ERR = n_error_ok();                                     \
    MOD_INIT_FUNC(&ERR);                                           \
    if (!n_is_ok(&ERR)) {                                          \
        NtTestMessage msg = NT_MAKE_MODULE_INIT_ERROR_MSG(&ERR);   \
        n_destroy_error(&ERR);                                     \
        ERROR(msg.contents, msg.clean_up);                         \
    }                                                              \
} while(0)


#endif /* N_TEST_H */
