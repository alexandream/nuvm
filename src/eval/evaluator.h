#ifndef N_E_EVALUATOR_H
#define N_E_EVALUATOR_H

#include <stdlib.h>

#include "values.h"

typedef struct NEvaluator NEvaluator;


struct NEvaluator {
    unsigned char *code;
    int code_size;
    NValue *registers;
    int num_registers;

    int pc;
    int halted;
};


int
ni_init_evaluator(void);

void n_evaluator_step(NEvaluator *self, NError *error);

void n_evaluator_run(NEvaluator *self, NError *error);

NValue
n_evaluator_get_register(NEvaluator *self, int index, NError *error);

#ifdef N_TEST
void
nt_construct_evaluator(NEvaluator* self, unsigned char* code,
                       int code_size, NValue* registers, int num_registers);

#endif /* N_TEST */

#endif /* N_E_EVALUATOR_H */

