#ifndef N_E_EVALUATOR_H
#define N_E_EVALUATOR_H

#include <stdlib.h>

#define N_STACK_SIZE 16384
#define N_ARGUMENTS_SIZE 64

#include "values.h"
#include "modules.h"

typedef struct NEvaluator NEvaluator;


struct NEvaluator {
    NModule *current_module;

    int pc;
    int sp;
    int fp;
    int halted;

    NValue arguments[N_ARGUMENTS_SIZE];

    NValue stack[N_STACK_SIZE];
    int stack_size;
};


int
ni_init_evaluator(void);

void
n_evaluator_step(NEvaluator *self, NError *error);

void
n_evaluator_run(NEvaluator *self, NError *error);

NValue
n_evaluator_get_register(NEvaluator *self, int index, NError *error);

NValue
n_evaluator_get_local(NEvaluator *self, int index, NError *error);

void
n_evaluator_set_local(NEvaluator *self, int index, NValue value, NError *error);

void
n_prepare_evaluator(NEvaluator *self, NModule *module, NError *error);

#ifdef N_TEST
void
nt_construct_evaluator(NEvaluator* self);

#endif /* N_TEST */

#endif /* N_E_EVALUATOR_H */

