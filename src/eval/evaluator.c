#include <stdio.h>

#include "values.h"
#include "evaluator.h"

#include "../common/opcodes.h"
#include "../common/instruction-decoders.h"


int
ni_init_evaluator(void) {
    return 0;
}


void n_evaluator_step(NEvaluator *self, NError *error) {
}


void n_evaluator_run(NEvaluator *self, NError *error) {
}


NValue
n_evaluator_get_register(NEvaluator *self, int index, NError *error) {
    return N_UNKNOWN;
}


#ifdef N_TEST
void
nt_construct_evaluator(NEvaluator* self, unsigned char* code,
                       int code_size, NValue* registers, int num_registers) {
}
#endif /* N_TEST */

