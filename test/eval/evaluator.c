#include <stdlib.h>
#include <stdio.h>
#include "../test.h"

#include "common/errors.h"

#include "common/instruction-encoders.h"

#include "eval/evaluator.h"
#include "eval/values.h"


#define CODE_SIZE 128
#define NUM_REGISTERS 16

static
unsigned char CODE[CODE_SIZE];


static
NValue REGISTERS[NUM_REGISTERS];


static
NEvaluator EVAL;


CONSTRUCTOR(constructor) {
    if (ni_init_evaluator() < 0) {
        ERROR("Can't initialize evaluator module.", NULL);
    }
}


SETUP(setup) {
    int i;
    for (i = 0; i < CODE_SIZE; i++) {
        n_encode_op_nop(CODE+i);
    }

    for (i = 0; i < NUM_REGISTERS; i++) {
        REGISTERS[i] = n_wrap_fixnum(0);
    }
    nt_construct_evaluator(&EVAL, CODE, CODE_SIZE, REGISTERS, NUM_REGISTERS);
}


TEST(pc_starts_on_zero) {
    ASSERT(EQ_INT(EVAL.pc, 0));
}


TEST(index_error_is_registered) {
    NError error = n_error_ok();
    NErrorType* error_type =
        n_error_type("nuvm.IndexOutOfBounds", &error);

    ASSERT(IS_TRUE(error_type != NULL));
    ASSERT(IS_OK(error));
}


TEST(opcode_error_is_registered) {
    NError error = n_error_ok();
    NErrorType* error_type =
        n_error_type("nuvm.UnknownOpcode", &error);

    ASSERT(IS_TRUE(error_type != NULL));
    ASSERT(IS_OK(error));
}


TEST(step_nop_increases_pc) {
    NError error = n_error_ok();
    n_encode_op_nop(CODE);
    n_evaluator_step(&EVAL, &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_INT(EVAL.pc, 1));
}


TEST(run_stops_on_halt) {
    NError error = n_error_ok();

    n_encode_op_nop(CODE);
    n_encode_op_nop(CODE+1);
    n_encode_op_nop(CODE+2);
    n_encode_op_halt(CODE+3);
    n_evaluator_run(&EVAL, &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_INT(EVAL.pc, 3));
}



TEST(get_register_gives_correct_value) {
    NValue value;
    NError error = n_error_ok();

    REGISTERS[3] = n_wrap_fixnum(99);

    value = n_evaluator_get_register(&EVAL, 3, &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_INT(n_unwrap_fixnum(value), 99));
}



TEST(get_register_detects_out_of_range) {
    NError error = n_error_ok();
    n_evaluator_get_register(&EVAL, NUM_REGISTERS, &error);

    ASSERT(IS_ERROR(error, "nuvm.IndexOutOfBounds"));
}


AtTest* tests[] = {
    &index_error_is_registered,
    &opcode_error_is_registered,
    &step_nop_increases_pc,
    &pc_starts_on_zero,
    &run_stops_on_halt,
    &get_register_gives_correct_value,
    &get_register_detects_out_of_range,
    NULL
};


TEST_RUNNER("Evaluator", tests, constructor, NULL, setup, NULL)

