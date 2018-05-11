#include "../common/common.h"
#include "../common/errors.h"

#include "asm.h"
#include "proto-instructions.h"
#include "proto-values.h"

void
n_init_asm(NError* error) {
#define EC ON_ERROR(error, return)
    n_init_common(error);                                          EC;

    ni_init_proto_instructions(error);                             EC;
    ni_init_proto_values(error);                                   EC;

#undef EC
}
