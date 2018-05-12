#include "../common/common.h"
#include "../common/errors.h"

#include "eval.h"

#include "type-registry.h"
#include "values.h"
#include "singletons.h"
#include "primitives.h"
#include "procedures.h"
#include "modules.h"
#include "loader.h"
#include "evaluator.h"

void
n_init_eval(NError* error) {
#define EC ON_ERROR(error, return)
    n_init_common(error);                                            EC;

    ni_init_type_registry(error);                                    EC;
    ni_init_values(error);                                           EC;
    ni_init_singletons(error);                                       EC;
    ni_init_primitives(error);                                       EC;
    ni_init_procedures(error);                                       EC;
    ni_init_modules(error);                                          EC;
    ni_init_loader(error);                                           EC;
    ni_init_evaluator(error);                                        EC;
#undef EC
}
