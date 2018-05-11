#ifndef N_A_PROTO_VALUES_H
#define N_A_PROTO_VALUES_H

#include "../common/errors.h"
#include "../common/compatibility/stdint.h"
#include "../common/byte-writers.h"

typedef struct NProtoValue NProtoValue;
typedef struct NProtoFixnum32 NProtoFixnum32;
typedef struct NProtoProcedure NProtoProcedure;

#define N_UNDEFINED_ANCHOR 0xFFFF

void
ni_init_proto_values(NError* error);

NProtoFixnum32 *
ni_create_proto_fixnum32(int32_t value, NError *error);


NProtoProcedure *
ni_create_proto_procedure(uint8_t min_locals, uint8_t max_locals,
                          NError *error);

uint16_t
ni_proto_value_code_size(NProtoValue* self);

void
ni_resolve_anchors(NProtoValue* self, NError *error);

void
ni_emit_proto_value_declaration(NByteWriter* writer, NProtoValue* value,
                                uint32_t code_offset, NError* error);
void
ni_emit_proto_value_code(NByteWriter* writer, NProtoValue* value,
                         NError* error);

uint16_t
ni_create_anchor(NProtoProcedure* self, NError *error);

void
ni_add_anchor(NProtoProcedure* self, uint16_t id, NError* error);


void
ni_add_proto_jump_unless(NProtoProcedure* self, uint8_t cond,
                             uint16_t anchor, NError *error);

void
ni_add_proto_jump(NProtoProcedure* self, uint16_t anchor, NError *error);

void
ni_add_proto_nop(NProtoProcedure* self, NError* error);

void
ni_add_proto_halt(NProtoProcedure* self, NError* error);

void
ni_add_proto_call(NProtoProcedure* self, uint8_t dest, uint8_t target,
                       uint8_t n_args, uint8_t* args, NError* error);

void
ni_add_proto_global_ref(NProtoProcedure* self, uint8_t dest,
                             uint16_t source, NError* error);

void
ni_add_proto_global_set(NProtoProcedure* self, uint16_t dest,
                             uint8_t source, NError* error);

void
ni_add_proto_return(NProtoProcedure* self, uint8_t source, NError* error);


#endif /*N_A_PROTO_VALUES_H*/
