#include <stdio.h>

#include "values.h"
#include "evaluator.h"

#include "../common/opcodes.h"
#include "../common/instruction-decoders.h"


static
NErrorType INDEX_OO_BOUNDS =  { "nuvm.IndexOutOfBounds", NULL };

static
NErrorType UNKNOWN_OPCODE  =  { "nuvm.UnknownOpcode", NULL };

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
nt_construct_evaluator(NEvaluator* self, unsigned char* code,
                       int code_size, NValue* registers, int num_registers) {
    self->code = code;
    self->code_size = code_size;
    self->registers = registers;
    self->num_registers = num_registers;
    self->pc = 0;
    self->halted = 0;
}
#endif /* N_TEST */

