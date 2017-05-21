#include <stdio.h>

#include "values.h"
#include "primitives.h"
#include "procedures.h"
#include "evaluator.h"

#include "../common/opcodes.h"
#include "../common/instruction-decoders.h"


static
NErrorType INDEX_OO_BOUNDS =  { "nuvm.IndexOutOfBounds", NULL };

static
NErrorType UNKNOWN_OPCODE  =  { "nuvm.UnknownOpcode", NULL };

static
NErrorType *ILLEGAL_ARGUMENT = NULL;

static int
op_jump_unless(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_load_i16(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_call(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_return(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_arg_ref(NEvaluator *self, unsigned char *stream, NError *error);

int
ni_init_evaluator(void) {
    NError error = n_error_ok();

    if (ni_init_errors() < 0) {
        return -1;
    }
    if (ni_init_values() < 0) {
        return -2;
    }
    n_register_error_type(&INDEX_OO_BOUNDS, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -3;
    }

    n_register_error_type(&UNKNOWN_OPCODE, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -4;
    }

    ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -5;
    }
    return 0;
}


void n_evaluator_step(NEvaluator *self, NError *error) {
    unsigned char *stream = self->code + self->pc;
    uint8_t opcode = stream[0];
    switch (opcode) {
        case N_OP_NOP:
            self->pc += 1;
            break;
        case N_OP_HALT:
            self->halted = 1;
            break;
        case N_OP_LOAD_I16:
            self->pc += op_load_i16(self, stream, error);
            break;
        case N_OP_JUMP: {
            int16_t offset;
            n_decode_op_jump(stream, &offset);
            self->pc += offset;
            break;
        }
        case N_OP_JUMP_UNLESS:
            self->pc += op_jump_unless(self, stream, error);
            break;
        case N_OP_CALL:
            /* Note that this ASSIGNS to the pc instead of ADDING to it.
             * That is because procedure calls may completely change the
             * current pc, jumping to the entry point of a user procedure. */
            self->pc = op_call(self, stream, error);
            break;
        case N_OP_RETURN:
            self->pc = op_return(self, stream, error);
            break;
        case N_OP_ARG_REF:
            self->pc += op_arg_ref(self, stream, error);
            break;
        default: {
            self->halted = 1;
            n_set_error(error, &UNKNOWN_OPCODE, "Found an unknown opcode.",
                        NULL, NULL);
            break;
        }
    }
}


void n_evaluator_run(NEvaluator *self, NError *error) {
    while (! self->halted ) {
        n_evaluator_step(self, error);
        if (!n_is_ok(error)) {
            self->halted = 1;
        }
    }
}


NValue
n_evaluator_get_register(NEvaluator *self, int index, NError *error) {
    if (index < self->num_registers) {
        return self->registers[index];
    }
    else {
        n_set_error(error, &INDEX_OO_BOUNDS, "The given index is larger "
                    "than the number of addressable registers "
                    "in this evaluator.", NULL, NULL);
        return N_UNKNOWN;

    }
}


#ifdef N_TEST
void
nt_construct_evaluator(NEvaluator* self, unsigned char* code, int code_size,
                       NValue* registers, int num_registers) {

    self->code = code;
    self->code_size = code_size;
    self->registers = registers;
    self->num_registers = num_registers;
    self->pc = 0;
    self->stack_size = N_STACK_SIZE;
    self->halted = 0;

    /* Initialize the dummy frame.
     * A dummy frame is composed of a frame pointer of -1, followed by two
     * zeroes: one for the return value register index and another for the
     * return address. */
    self->sp = 3;
    self->stack[0] = -1;
    self->stack[1] = 0;
    self->stack[2] = 0;
}
#endif /* N_TEST */

static int
op_call(NEvaluator *self, unsigned char *stream, NError *error) {
    int i;
    uint8_t dest, target, n_args;
    int size = n_decode_op_call(stream, &dest, &target, &n_args);
    int next_pc = self->pc + size + n_args;
    NValue callable = self->registers[target];
    NValue result;

    for (i = 0; i < n_args; i++) {
        uint8_t arg_index = stream[size + i];
        self->arguments[i] = self->registers[arg_index];
    }

    if (n_is_primitive(callable)) {
        result = n_call_primitive(callable, n_args, self->arguments, error);
        if (!n_is_ok(error)) {
            return 0;
        }

        self->registers[dest] = result;
        return next_pc;
    }
    else {
        /* Set up the new frame and jump to the procedure's entry point. */
        NProcedure* proc = (NProcedure*) n_unwrap_pointer(callable);
        int previous_fp = self->fp;
        self->fp = self->sp;
        self->sp += 3;  /* Make space for the new frame. */
        self->stack[self->sp -3] = previous_fp;
        self->stack[self->sp -2] = dest;
        self->stack[self->sp -1] = next_pc;
        self->sp += proc->num_locals; /* Make space for the locals. */
        return proc->entry;
    }
}


static int
op_jump_unless(NEvaluator *self, unsigned char *stream, NError *error) {
    uint8_t r_condition;
    int16_t offset;
    int size = n_decode_op_jump_unless(stream, &r_condition, &offset);
    NValue condition = self->registers[r_condition];

    if (n_eq_values(condition, N_TRUE)) {
        return offset;
    }
    else if (n_eq_values(condition, N_FALSE)) {
        return size;
    }
    else {
        n_set_error(error, ILLEGAL_ARGUMENT, "Condition to "
                    "jump-unless must be a valid Boolean value.",
                    NULL, NULL);
        return 0;
    }
}


static int
op_return(NEvaluator *self, unsigned char *stream, NError *error) {
    if (self->stack[self->sp -3] == -1) {
        /* We're on a dummy frame. Halt the machine. */
        self->halted = 1;
        return self->pc;
    }
    else {
        int stored_pc = self->stack[self->sp -1];
        int dest      = self->stack[self->sp -2];
        int stored_fp = self->stack[self->sp -3];
        uint8_t src;

        n_decode_op_return(stream, &src);
        self->registers[dest] = self->registers[src];

        self->sp = self->fp;
        self->fp = stored_fp;
        return stored_pc;
    }

}


static int
op_arg_ref(NEvaluator *self, unsigned char *stream, NError *error) {
    uint8_t dest;
    uint8_t source;
    int size = n_decode_op_arg_ref(stream, &dest, &source);

    self->registers[dest] = self->arguments[source];
    return size;
}


static int
op_load_i16(NEvaluator *self, unsigned char *stream, NError *error) {
    uint8_t dest;
    int16_t value;
    int size = n_decode_op_load_i16(stream, &dest, &value);

    self->registers[dest] = n_wrap_fixnum(value);
    return size;
}
