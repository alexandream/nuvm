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

static
NModule *MOD;


CONSTRUCTOR(constructor) {
    NError error = n_error_ok();

    if (ni_init_evaluator() < 0) {
        ERROR("Can't initialize evaluator module.", NULL);
    }

    MOD = n_new_module(&error);
    if (!n_is_ok(&error)) {
        ERROR("Can't create module.", NULL);
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
    nt_construct_evaluator(&EVAL);

    MOD->code = CODE;
    MOD->code_size = CODE_SIZE;
    MOD->registers = REGISTERS;
    MOD->num_registers = NUM_REGISTERS;

    EVAL.current_module = MOD;
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


TEST(sp_starts_on_end_of_dummy_frame) {
    /* There's three elements on the stack upon construction. */
    ASSERT(EQ_INT(EVAL.sp, 3));
}


TEST(dummy_frame_is_pushed) {
    ASSERT(EQ_INT(EVAL.stack[0], -1));
    ASSERT(EQ_INT(EVAL.stack[1], 0));
    ASSERT(EQ_INT(EVAL.stack[2], 0));
}


TEST(fp_starts_on_zero) {
    ASSERT(EQ_INT(EVAL.fp, 0));
}

AtTest* tests[] = {
    &index_error_is_registered,
    &opcode_error_is_registered,
    &step_nop_increases_pc,
    &pc_starts_on_zero,
    &run_stops_on_halt,
    &get_register_gives_correct_value,
    &get_register_detects_out_of_range,
    &sp_starts_on_end_of_dummy_frame,
    &fp_starts_on_zero,
    &dummy_frame_is_pushed,
    NULL
};


TEST_RUNNER("Evaluator", tests, constructor, NULL, setup, NULL)

