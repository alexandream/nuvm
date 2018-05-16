#ifndef N_A_PROTO_MODULE_H
#define N_A_PROTO_MODULE_H

#include "proto-values.h"

#include "../common/errors.h"
#include "../common/compatibility/stdint.h"

typedef struct NProtoModule NProtoModule;

void
ni_init_proto_module(NError* error);

NProtoModule*
ni_create_proto_module(NError* error);

void
ni_add_proto_value(NProtoModule* module, NProtoValue* value, NError* error);

size_t
ni_proto_value_count(NProtoModule* module);

NProtoValue*
ni_get_proto_value(NProtoModule* module, size_t i, NError* error);

#endif /*N_A_PROTO_MODULE_H*/
