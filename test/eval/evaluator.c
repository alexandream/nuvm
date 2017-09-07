#include <stdlib.h>
#include <stdio.h>
#include "../test.h"

#include "common/errors.h"

#include "common/instruction-encoders.h"

#include "eval/evaluator.h"
#include "eval/procedures.h"
#include "eval/values.h"


#define CODE_SIZE 128
#define NUM_REGISTERS 16

static
unsigned char *CODE;


static
NValue *REGISTERS;


static
NEvaluator EVAL;

static
NModule *MOD;

static
NValue PROC;

static const
int PROC_ENTRY = 42;

CONSTRUCTOR(constructor) {
    NError error = n_error_ok();

    if (ni_init_evaluator() < 0) {
        ERROR("Can't initialize evaluator module.", NULL);
    }

    MOD = n_create_module(NUM_REGISTERS, CODE_SIZE, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't create module.", NULL);
    }
    REGISTERS = MOD->globals;
    CODE = MOD->code;

    PROC = n_create_procedure(PROC_ENTRY, 2, 2, 1, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't create module's entry procedure.", NULL);
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

    MOD->entry_point = 15;

    REGISTERS[15] = PROC;
}


TEST(pc_starts_negative) {
    ASSERT(IS_TRUE(EVAL.pc < 0));
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


TEST(evaluator_starts_halted) {
    ASSERT(IS_TRUE(EVAL.halted));
}


TEST(step_nop_increases_pc) {
    NError error = n_error_ok();

    n_prepare_evaluator(&EVAL, MOD, &error);
    ASSERT(IS_OK(error));

    n_encode_op_nop(CODE+PROC_ENTRY);
    n_evaluator_step(&EVAL, &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_INT(EVAL.pc, PROC_ENTRY+1));
}


TEST(run_stops_on_halt) {
    NError error = n_error_ok();

    n_prepare_evaluator(&EVAL, MOD, &error);
    ASSERT(IS_OK(error));

    n_encode_op_nop(CODE+PROC_ENTRY);
    n_encode_op_nop(CODE+PROC_ENTRY+1);
    n_encode_op_nop(CODE+PROC_ENTRY+2);
    n_encode_op_halt(CODE+PROC_ENTRY+3);
    n_evaluator_run(&EVAL, &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_INT(EVAL.pc, PROC_ENTRY+3));
}


TEST(sp_starts_on_zero) {
    ASSERT(EQ_INT(EVAL.sp, 0));
}


TEST(fp_starts_on_zero) {
    ASSERT(EQ_INT(EVAL.fp, 0));
}


TEST(get_global_gives_correct_value) {
    NValue value;
    NError error = n_error_ok();

    REGISTERS[3] = n_wrap_fixnum(99);

    value = n_evaluator_get_global(&EVAL, 3, &error);

    ASSERT(IS_OK(error));
    ASSERT(EQ_INT(n_unwrap_fixnum(value), 99));
}



TEST(get_global_detects_out_of_range) {
    NError error = n_error_ok();
    n_evaluator_get_global(&EVAL, NUM_REGISTERS, &error);

    ASSERT(IS_ERROR(error, "nuvm.IndexOutOfBounds"));
}


TEST(prepare_pushes_dummy_frame) {
    NError error = n_error_ok();

    n_prepare_evaluator(&EVAL, MOD, &error);
    ASSERT(IS_OK(error));

    ASSERT(IS_TRUE(EVAL.sp >= 3));
    ASSERT(EQ_INT(EVAL.stack[0], -1));
    ASSERT(EQ_INT(EVAL.stack[1], 0));
    ASSERT(EQ_INT(EVAL.stack[2], 0));
}


TEST(prepare_adds_num_locals_to_sp) {
    NError error = n_error_ok();

    n_prepare_evaluator(&EVAL, MOD, &error);
    ASSERT(IS_OK(error));

    /* 3 for the dummy frame, 2 for the num of locals in PROC */
    ASSERT(EQ_INT(EVAL.sp, 3+2));
}


TEST(prepare_sets_pc_to_proc_entry) {
    NError error = n_error_ok();

    n_prepare_evaluator(&EVAL, MOD, &error);
    ASSERT(IS_OK(error));

    ASSERT(EQ_INT(EVAL.pc, PROC_ENTRY));
}


TEST(prepare_clears_halted_flag) {
    NError error = n_error_ok();

    n_prepare_evaluator(&EVAL, MOD, &error);
    ASSERT(IS_OK(error));

    ASSERT(IS_TRUE(!EVAL.halted));
}


AtTest* tests[] = {
    &index_error_is_registered,
    &opcode_error_is_registered,
    &step_nop_increases_pc,
    &run_stops_on_halt,
    &pc_starts_negative,
    &evaluator_starts_halted,
    &sp_starts_on_zero,
    &fp_starts_on_zero,
    &get_global_gives_correct_value,
    &get_global_detects_out_of_range,
    &prepare_pushes_dummy_frame,
    &prepare_adds_num_locals_to_sp,
    &prepare_sets_pc_to_proc_entry,
    &prepare_clears_halted_flag,
    NULL
};


TEST_RUNNER("Evaluator", tests, constructor, NULL, setup, NULL)

