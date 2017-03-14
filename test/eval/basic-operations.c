#include <stdlib.h>
#include <stdio.h>
#include "../test.h"

#include "common/errors.h"

#include "common/instruction-encoders.h"

#include "eval/evaluator.h"
#include "eval/primitives.h"
#include "eval/values.h"


#define CODE_SIZE 128
#define NUM_REGISTERS 16

typedef struct {
    uint8_t dest;
    NFixnum value;
} FixnumLoadData;


static
unsigned char CODE[CODE_SIZE];


static
NValue REGISTERS[NUM_REGISTERS];

static
NValue COPY_RESULT[NUM_REGISTERS];

static
NEvaluator EVAL;

static
NError ERR;

static
NValue TRUE_PRIMITIVE;

static
NValue FLAG_PRIMITIVE;

static
NValue COPY_PRIMITIVE;

static
int FLAG;

static NValue
true_function(int n_args, NValue *args, NError *error);

static NValue
flag_function(int n_args, NValue *args, NError *error);

static NValue
copy_function(int n_args, NValue *args, NError *error);



CONSTRUCTOR(constructor) {
    if (ni_init_evaluator() < 0) {
        ERROR("Can't initialize evaluator module.", NULL);
    }

    TRUE_PRIMITIVE = n_create_primitive(true_function, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create true primitive.", NULL);
    }

    FLAG_PRIMITIVE = n_create_primitive(flag_function, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create flag primitive.", NULL);
    }

    COPY_PRIMITIVE = n_create_primitive(copy_function, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create copy primitive.", NULL);
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
    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
}


TEST(op_load_i16_increments_pc_by_4) {
    n_encode_op_load_i16(CODE, 0, 0);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 4));
}


FixnumLoadData load_i16_array[] = {
    { 0, 32767 },
    { 15, -32768 },
    { 3, 0 }
};
AtArrayIterator load_i16_iter = at_static_array_iterator(load_i16_array);

DD_TEST(op_load_i16_loads_correct_value, load_i16_iter, FixnumLoadData, load) {
    uint8_t dest = load->dest;
    NFixnum value = load->value;
    NFixnum result;

    n_encode_op_load_i16(CODE, dest, value);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));

    result = n_unwrap_fixnum(n_evaluator_get_register(&EVAL, dest, &ERR));
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_INT(result, value));
}


int16_t jump_array[] = { -5, 0, 5 };
AtArrayIterator jump_iter = at_static_array_iterator(jump_array);

DD_TEST(op_jump_adds_offset_to_pc, jump_iter, int16_t, offset) {
    EVAL.pc = 32;
    n_encode_op_jump(CODE +32, *offset);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 32 + *offset));
}


TEST(op_jump_unless_adds_4_to_pc_on_false) {
    n_encode_op_jump_unless(CODE, 5, 12357);
    REGISTERS[5] = N_FALSE;

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 4));
}


int16_t jump_unless_array[] = { -5, 0, 5 };
AtArrayIterator jump_unless_iter = at_static_array_iterator(jump_unless_array);
DD_TEST(op_jump_unless_adds_offset_to_pc, jump_unless_iter, int16_t, offset) {
    EVAL.pc = 32;
    REGISTERS[3] = N_TRUE;
    n_encode_op_jump_unless(CODE +32, 3, *offset);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 32 + *offset));
}


TEST(op_call_adds_4_plus_nargs_to_pc) {
    n_encode_op_call(CODE, 0, 1, 5);
    REGISTERS[1] = TRUE_PRIMITIVE;

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 9));
}


TEST(op_call_calls_primitive_func) {
    n_encode_op_call(CODE, 0, 5, 0);
    FLAG = 0;
    REGISTERS[5] = FLAG_PRIMITIVE;

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(FLAG, 1));
}

TEST(op_call_passes_arguments) {
    int i;
    n_encode_op_call(CODE, 0, 5, 3);
    CODE[4] = 7;
    CODE[5] = 1;
    CODE[6] = 9;

    for (i = 0; i < 3; i++) {
        COPY_RESULT[i] = N_UNKNOWN;
    }

    REGISTERS[5] = COPY_PRIMITIVE;
    REGISTERS[7] = N_TRUE;
    REGISTERS[1] = N_FALSE;
    REGISTERS[9] = n_wrap_fixnum(123);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(n_eq_values(COPY_RESULT[0], N_TRUE)));
    ASSERT(IS_TRUE(n_eq_values(COPY_RESULT[1], N_FALSE)));
    ASSERT(IS_TRUE(n_eq_values(COPY_RESULT[2], n_wrap_fixnum(123))));
}


TEST(op_call_stores_returned_value) {
    n_encode_op_call(CODE, 14, 1, 0);
    REGISTERS[1] = TRUE_PRIMITIVE;
    REGISTERS[14] = N_UNKNOWN;

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(REGISTERS[14], N_TRUE)));
}


AtTest* tests[] = {
    &op_load_i16_increments_pc_by_4,
    &op_load_i16_loads_correct_value,

    &op_jump_adds_offset_to_pc,
    &op_jump_unless_adds_4_to_pc_on_false,
    &op_jump_unless_adds_offset_to_pc,

    &op_call_adds_4_plus_nargs_to_pc,
    &op_call_calls_primitive_func,
    &op_call_passes_arguments,
    &op_call_stores_returned_value,

    NULL
};


TEST_RUNNER("BasicOperations", tests, constructor, NULL, setup, teardown)

static NValue
true_function(int n_args, NValue *args, NError *error) {
    return N_TRUE;
}


static NValue
flag_function(int n_args, NValue *args, NError *error) {
    FLAG = 1;
    return N_UNKNOWN;
}


static NValue
copy_function(int n_args, NValue *args, NError *error) {
    int i;
    for (i = 0; i < n_args && i < NUM_REGISTERS; i++) {
        COPY_RESULT[i] = args[i];
    }
    return N_UNKNOWN;
}
