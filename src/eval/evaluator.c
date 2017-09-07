#include "values.h"
#include "primitives.h"
#include "procedures.h"
#include "singletons.h"
#include "evaluator.h"


#include "../common/opcodes.h"
#include "../common/instruction-decoders.h"


static
NErrorType INDEX_OO_BOUNDS =  { "nuvm.IndexOutOfBounds", NULL };

static
NErrorType UNKNOWN_OPCODE  =  { "nuvm.UnknownOpcode", NULL };

static
NErrorType *ILLEGAL_ARGUMENT = NULL;

static NValue
get_local(NEvaluator *self, uint8_t index);

static void
set_local(NEvaluator *self, uint8_t index, NValue value);

static int
op_jump_unless(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_load_i16(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_call(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_return(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_global_ref(NEvaluator *self, unsigned char *stream, NError *error);

static int
op_global_set(NEvaluator *self, unsigned char *stream, NError *error);

int
ni_init_evaluator(void) {
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        NError error = n_error_ok();

        if (ni_init_errors() < 0) {
            return -1;
        }
        if (ni_init_all_values() < 0) {
            return -2;
        }
        if (ni_init_modules() < 0) {
            return -3;
        }
        n_register_error_type(&INDEX_OO_BOUNDS, &error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -4;
        }

        n_register_error_type(&UNKNOWN_OPCODE, &error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -5;
        }

        ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", &error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -6;
        }
        INITIALIZED = 1;
    }
    return 0;
}


void n_evaluator_step(NEvaluator *self, NError *error) {
    unsigned char *stream = self->current_module->code + self->pc;

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
            /* Note the assignment to pc here. Refer to the CALL instruction
             * for a rationale. */
            self->pc = op_return(self, stream, error);
            break;
        case N_OP_GLOBAL_REF:
            self->pc += op_global_ref(self, stream, error);
            break;
        case N_OP_GLOBAL_SET:
            self->pc += op_global_set(self, stream, error);
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
n_evaluator_get_global(NEvaluator *self, int index, NError *error) {
    if (index < self->current_module->num_globals) {
        return self->current_module->globals[index];
    }
    else {
        n_set_error(error, &INDEX_OO_BOUNDS, "The given index is larger "
                    "than the number of addressable globals "
                    "in this evaluator.", NULL, NULL);
        return N_UNKNOWN;

    }
}


NValue
n_evaluator_get_local(NEvaluator *self, int index, NError *error) {
    return get_local(self, index);
}


void
n_evaluator_set_local(NEvaluator *self, int index, NValue val, NError *error) {
    set_local(self, index, val);
}


void
n_prepare_evaluator(NEvaluator *self, NModule *module, NError *error) {
    NValue entry_val;
    NProcedure* entry_proc;

    entry_val = module->globals[module->entry_point];

    if (!n_is_procedure(entry_val)) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Value on the entry point of "
                    "a module must be a procedure.",
                    NULL, NULL);
        return;
    }

    entry_proc = (NProcedure*) n_unwrap_object(entry_val);

    self->current_module = module;

    self->pc = entry_proc->entry;

    /* Initialize the dummy frame.
     * A dummy frame is composed of a frame pointer of -1, followed by two
     * zeroes: one for the return value register index and another for the
     * return address. */
    self->sp = 3 + entry_proc->num_locals;
    self->fp = 0;
    self->stack[self->fp+0] = -1;
    self->stack[self->fp+1] = 0;
    self->stack[self->fp+2] = 0;

    self->halted = 0;
}

#ifdef N_TEST
void
nt_construct_evaluator(NEvaluator* self) {
    self->pc = -1;
    self->stack_size = N_STACK_SIZE;
    self->halted = 1;

    self->sp = 0;
    self->fp = 0;
}
#endif /* N_TEST */

static NValue*
get_locals_addr(NEvaluator *self) {
    return self->stack + self->fp + 3;
}


static NValue
get_local(NEvaluator *self, uint8_t index) {
    return get_locals_addr(self)[index];
}


static void
set_local(NEvaluator *self, uint8_t index, NValue value) {
    NValue* locals = get_locals_addr(self);
    locals[index] = value;
}


static int
op_call(NEvaluator *self, unsigned char *stream, NError *error) {
    int i;
    uint8_t dest, target, n_args;
    int size = n_decode_op_call(stream, &dest, &target, &n_args);
    int next_pc = self->pc + size + n_args;
    NValue callable = get_local(self, target);
    NValue result;

    if (n_is_primitive(callable)) {
        for (i = 0; i < n_args; i++) {
            uint8_t arg_index = stream[size + i];
            self->arguments[i] = get_local(self, arg_index);
        }

        result = n_call_primitive(callable, n_args, self->arguments, error);
        if (!n_is_ok(error)) {
            return 0;
        }

        set_local(self, dest, result);
        return next_pc;
    }
    else if (n_is_procedure(callable)) {
        /* Set up the new frame and jump to the procedure's entry point. */
        NProcedure* proc = (NProcedure*) n_unwrap_object(callable);
        int previous_fp = self->fp;
        NValue* old_locals = get_locals_addr(self);
        self->fp = self->sp;
        self->stack[self->fp] = previous_fp;
        self->stack[self->fp +1] = dest;
        self->stack[self->fp +2] = next_pc;
        /* Make space for the saved globals, locals and arguments. */
        self->sp += 3 + proc->num_locals + n_args;
        for (i = 0; i < n_args; i++) {
            uint8_t arg_index = stream[size + i];
            set_local(self, proc->num_locals + i, old_locals[arg_index]);
        }
        return proc->entry;
    }
    else {
        n_set_error(error, ILLEGAL_ARGUMENT, "Target to call instruction "
                    "must be a callable object.",
                    NULL, NULL);
        return self->pc;
    }

}


static int
op_jump_unless(NEvaluator *self, unsigned char *stream, NError *error) {
    uint8_t r_condition;
    int16_t offset;
    int size = n_decode_op_jump_unless(stream, &r_condition, &offset);
    NValue condition = get_local(self, r_condition);

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
    if (self->stack[self->fp] == -1) {
        /* We're on a dummy frame. Halt the machine. */
        self->halted = 1;
        return self->pc;
    }
    else {
        int stored_pc = self->stack[self->fp +2];
        int dest      = self->stack[self->fp +1];
        int stored_fp = self->stack[self->fp];
        uint8_t src;
        NValue return_value;

        n_decode_op_return(stream, &src);

        return_value = get_local(self, src);

        self->sp = self->fp;
        self->fp = stored_fp;
        set_local(self, dest, return_value);

        return stored_pc;
    }

}


static int
op_global_ref(NEvaluator *self, unsigned char *stream, NError *error) {
    uint8_t dest;
    uint16_t source;
    int size = n_decode_op_global_ref(stream, &dest, &source);

    set_local(self, dest, self->current_module->globals[source]);
    return size;
}


static int
op_global_set(NEvaluator *self, unsigned char *stream, NError *error) {
    uint16_t dest;
    uint8_t source;
    int size = n_decode_op_global_set(stream, &dest, &source);

    self->current_module->globals[dest] = get_local(self, source);
    return size;
}


static int
op_load_i16(NEvaluator *self, unsigned char *stream, NError *error) {
    uint8_t dest;
    int16_t value;
    int size = n_decode_op_load_i16(stream, &dest, &value);

    set_local(self, dest, n_wrap_fixnum(value));
    return size;
}
