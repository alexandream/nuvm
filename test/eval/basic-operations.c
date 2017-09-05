#include <stdlib.h>
#include <stdio.h>
#include "../test.h"

#include "common/errors.h"

#include "common/instruction-encoders.h"

#include "eval/evaluator.h"
#include "eval/primitives.h"
#include "eval/procedures.h"
#include "eval/singletons.h"
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
NModule* MOD;

static
NError ERR;

static
NValue TRUE_PRIMITIVE;

static
NValue FLAG_PRIMITIVE;

static
NValue COPY_PRIMITIVE;

static
NValue ENTRY_PROC;

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

	if (ni_init_all_values() < 0) {
		ERROR("Can't initialize singletons module.", NULL);
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

    ENTRY_PROC = n_create_procedure(0, 0, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't create module's entry procedure.", NULL);
    }

    MOD = n_new_module(&ERR);
    if (!n_is_ok(&ERR)) {
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
    MOD->entry_point = 15;
    MOD->registers[15] = ENTRY_PROC;

    n_prepare_evaluator(&EVAL, MOD, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Can't prepare evaluator to run the given module.", NULL);
    }

    for (i = 0; i < N_ARGUMENTS_SIZE; i++) {
        EVAL.arguments[i] = N_UNKNOWN;
    }
    /* Make room for some locals. */
    EVAL.sp += 16;
    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
}


TEST(load_i16_increments_pc_by_4) {
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

DD_TEST(load_i16_loads_correct_value, load_i16_iter, FixnumLoadData, load) {
    uint8_t dest = load->dest;
    NFixnum value = load->value;
    NFixnum result;

    n_encode_op_load_i16(CODE, dest, value);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));

    result = n_unwrap_fixnum(n_evaluator_get_local(&EVAL, dest, &ERR));
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_INT(result, value));
}


int16_t jump_array[] = { -5, 0, 5 };
AtArrayIterator jump_iter = at_static_array_iterator(jump_array);

DD_TEST(jump_adds_offset_to_pc, jump_iter, int16_t, offset) {
    EVAL.pc = 32;
    n_encode_op_jump(CODE +32, *offset);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 32 + *offset));
}


TEST(jump_unless_adds_4_to_pc_on_false) {
    n_encode_op_jump_unless(CODE, 5, 12357);
    n_evaluator_set_local(&EVAL, 5, N_FALSE, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 4));
}


int16_t jump_unless_array[] = { -5, 0, 5 };
AtArrayIterator jump_unless_iter = at_static_array_iterator(jump_unless_array);
DD_TEST(jump_unless_adds_offset_to_pc, jump_unless_iter, int16_t, offset) {
    EVAL.pc = 32;
    n_evaluator_set_local(&EVAL, 3, N_TRUE, &ERR);
    n_encode_op_jump_unless(CODE +32, 3, *offset);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 32 + *offset));
}


TEST(call_adds_4_plus_nargs_to_pc) {
    n_encode_op_call(CODE, 0, 1, 5);
    n_evaluator_set_local(&EVAL, 1, TRUE_PRIMITIVE, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 9));
}


TEST(call_proc_sets_pc) {
    NValue proc = n_create_procedure(17, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_encode_op_call(CODE, 0, 1, 0);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 17));
}


TEST(call_proc_pushes_frame_pointer) {
    NValue proc = n_create_procedure(0, 0, &ERR);
    ASSERT(IS_OK(ERR));

    EVAL.fp = 12345;

    n_encode_op_call(CODE, 9, 1, 0);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.stack[EVAL.sp -3], 12345));
}


TEST(call_proc_pushes_ret_index) {
    NValue proc = n_create_procedure(0, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_encode_op_call(CODE, 9, 1, 0);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.stack[EVAL.fp +1], 9));
}


TEST(call_proc_pushes_ret_addr) {
    NValue proc = n_create_procedure(0, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_encode_op_call(CODE, 9, 1, 3);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.stack[EVAL.fp+2], 7));
}


TEST(call_proc_pushes_arguments) {
    NValue proc = n_create_procedure(0, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_encode_op_call(CODE, 9, 1, 3);
    CODE[4] = 7;
    CODE[5] = 3;
    CODE[6] = 9;

    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    n_evaluator_set_local(&EVAL, 7, N_TRUE, &ERR);
    n_evaluator_set_local(&EVAL, 3, N_FALSE, &ERR);
    n_evaluator_set_local(&EVAL, 9, n_wrap_fixnum(123), &ERR);

    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(n_evaluator_get_local(&EVAL, 0, &ERR),
                               N_TRUE)));
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(n_evaluator_get_local(&EVAL, 1, &ERR),
                               N_FALSE)));
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(n_evaluator_get_local(&EVAL, 2, &ERR),
                               n_wrap_fixnum(123))));
    ASSERT(IS_OK(ERR));

}


TEST(call_proc_w_no_locals_adds_3_to_sp) {
    NValue proc = n_create_procedure(0, 0, &ERR);
    int sp_before_step;
    ASSERT(IS_OK(ERR));

    n_encode_op_call(CODE, 9, 1, 0);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    sp_before_step = EVAL.sp;
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.sp, sp_before_step + 3));
}


TEST(call_proc_adds_3_plus_nlocals_to_sp) {
    NValue proc = n_create_procedure(0, 8, &ERR);
    int sp_before_step;
    ASSERT(IS_OK(ERR));

    n_encode_op_call(CODE, 9, 1, 0);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    sp_before_step = EVAL.sp;
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.sp, sp_before_step + 3 + 8));

}


TEST(call_calls_primitive_func) {
    n_encode_op_call(CODE, 0, 5, 0);
    FLAG = 0;
    n_evaluator_set_local(&EVAL, 5, FLAG_PRIMITIVE, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(FLAG, 1));
}

TEST(call_passes_arguments) {
    int i;
    n_encode_op_call(CODE, 0, 5, 3);
    CODE[4] = 7;
    CODE[5] = 1;
    CODE[6] = 9;

    for (i = 0; i < 3; i++) {
        COPY_RESULT[i] = N_UNKNOWN;
    }

    n_evaluator_set_local(&EVAL, 5, COPY_PRIMITIVE, &ERR);
    n_evaluator_set_local(&EVAL, 7, N_TRUE, &ERR);
    n_evaluator_set_local(&EVAL, 1, N_FALSE, &ERR);
    n_evaluator_set_local(&EVAL, 9, n_wrap_fixnum(123), &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(n_eq_values(COPY_RESULT[0], N_TRUE)));
    ASSERT(IS_TRUE(n_eq_values(COPY_RESULT[1], N_FALSE)));
    ASSERT(IS_TRUE(n_eq_values(COPY_RESULT[2], n_wrap_fixnum(123))));
}


TEST(call_stores_returned_value) {
    n_encode_op_call(CODE, 14, 1, 0);
    n_evaluator_set_local(&EVAL, 1, TRUE_PRIMITIVE, &ERR);
    n_evaluator_set_local(&EVAL, 14, N_UNKNOWN, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(n_evaluator_get_local(&EVAL, 14, &ERR), N_TRUE)));
}


TEST(call_moves_fp_up_to_previous_sp) {
    int previous_sp = EVAL.sp;
    NValue proc = n_create_procedure(0, 0, &ERR);
    n_encode_op_call(CODE, 0, 1, 0);
    n_evaluator_set_local(&EVAL, 1, proc, &ERR);

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.fp, previous_sp));

}


TEST(return_halts_on_dummy_frame) {
    n_encode_op_nop(CODE);
    n_encode_op_nop(CODE+1);
    n_encode_op_nop(CODE+2);
    n_encode_op_return(CODE+3, 0);
    n_encode_op_nop(CODE+5);

    n_evaluator_run(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 3));
}


TEST(return_rolls_sp_to_saved_fp) {
    EVAL.sp = 8;
    EVAL.fp = 5;
    n_encode_op_return(CODE, 0);

    /* Create a useless frame */
    EVAL.stack[EVAL.sp -3] = 0;
    EVAL.stack[EVAL.sp -2] = 0;
    EVAL.stack[EVAL.sp -1] = 0;

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.sp, 5));
}


TEST(return_restores_saved_fp) {
    EVAL.sp = 18;
    EVAL.fp = 15;
    n_encode_op_return(CODE, 0);

    /* Create a frame with fp = 3 */
    EVAL.stack[EVAL.sp -3] = 3;
    EVAL.stack[EVAL.sp -2] = 0;
    EVAL.stack[EVAL.sp -1] = 0;

    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.fp, 3));


}


TEST(return_sets_pc_to_saved_addr) {
    EVAL.sp = 6;
    EVAL.fp = 3;
    /* Create a frame where return addr is 21. */
    EVAL.stack[EVAL.fp] = 0;
    EVAL.stack[EVAL.fp +1] = 0;
    EVAL.stack[EVAL.fp +2] = 21;

    n_encode_op_return(CODE, 0);
    n_evaluator_step(&EVAL, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 21));
}


TEST(return_sets_dest_register_value) {
    EVAL.sp = 10;
    EVAL.fp = 7;
    /* Create a frame where return index is 3. */
    EVAL.stack[EVAL.fp] = 0;
    EVAL.stack[EVAL.fp +1] = 3;
    EVAL.stack[EVAL.fp +2] = 0;

    n_evaluator_set_local(&EVAL, 7, N_TRUE, &ERR);

    n_encode_op_return(CODE, 7);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(n_evaluator_get_local(&EVAL, 3, &ERR), N_TRUE)));
}


TEST(global_ref_adds_4_to_pc) {
    n_encode_op_global_ref(CODE, 0, 0);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 4));
}


TEST(global_ref_copies_values) {
    REGISTERS[9] = N_TRUE;

    /* Create a frame where the third local is N_UNKNOWN */
    EVAL.fp = 17;
    EVAL.stack[EVAL.fp +3+2] = N_UNKNOWN;

    n_encode_op_global_ref(CODE, 2, 9);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_eq_values(EVAL.stack[EVAL.fp +3+2], N_TRUE)));
}


TEST(global_set_adds_4_to_pc) {
    n_encode_op_global_set(CODE, 0, 0);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(EVAL.pc, 4));
}


TEST(global_set_copies_values) {
    REGISTERS[9] =  N_UNKNOWN;

    /* Create a frame where the third local is N_UNKNOWN */
    EVAL.fp = 17;
    EVAL.stack[EVAL.fp +3+2] = N_TRUE;

    n_encode_op_global_set(CODE, 9, 2);
    n_evaluator_step(&EVAL, &ERR);

    ASSERT(IS_OK(ERR));
    {
        NValue result = n_evaluator_get_register(&EVAL, 9, &ERR);
        ASSERT(IS_OK(ERR));
        ASSERT(IS_TRUE(n_eq_values(result, N_TRUE)));
    }
}



AtTest* tests[] = {
    &load_i16_increments_pc_by_4,
    &load_i16_loads_correct_value,

    &jump_adds_offset_to_pc,
    &jump_unless_adds_4_to_pc_on_false,
    &jump_unless_adds_offset_to_pc,

    &call_adds_4_plus_nargs_to_pc,
    &call_calls_primitive_func,
    &call_passes_arguments,
    &call_stores_returned_value,

    &call_proc_sets_pc,
    &call_proc_pushes_frame_pointer,
    &call_proc_pushes_ret_index,
    &call_proc_pushes_ret_addr,
    &call_proc_pushes_arguments,
    &call_proc_w_no_locals_adds_3_to_sp,
    &call_proc_adds_3_plus_nlocals_to_sp,
    &call_moves_fp_up_to_previous_sp,

    &return_halts_on_dummy_frame,
    &return_rolls_sp_to_saved_fp,
    &return_sets_pc_to_saved_addr,
    &return_sets_dest_register_value,
    &return_restores_saved_fp,

    &global_ref_copies_values,
    &global_ref_adds_4_to_pc,

    &global_set_copies_values,
    &global_set_adds_4_to_pc,

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
