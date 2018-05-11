#ifndef N_A_PROTO_INSTRUCTIONS_H
#define N_A_PROTO_INSTRUCTIONS_H

#include "../common/compatibility/stdint.h"
#include "../common/errors.h"
#include "../common/byte-writers.h"

typedef struct NAnchorMap NAnchorMap;
typedef struct NAnchorMapVTable NAnchorMapVTable;
typedef struct NProtoInstruction NProtoInstruction;
typedef struct NProtoInstructionVTable NProtoInstructionVTable;

struct NProtoInstructionVTable {
    uint16_t (*size)(NProtoInstruction*);
    void (*emit)(NByteWriter*, NProtoInstruction*, NError*);
    void (*resolve_anchors)(NProtoInstruction*, uint16_t, NAnchorMap*, NError*);
    void (*destruct)(NProtoInstruction*);
};


struct NAnchorMapVTable {
    int      (*has_anchor)(NAnchorMap*, int);
    uint16_t (*get_offset)(NAnchorMap*, int);
};


struct NAnchorMap {
    NAnchorMapVTable* vtable;
};


struct NProtoInstruction {
    NProtoInstructionVTable* vtable;
    uint16_t u16s[1];
    int16_t i16s[1];
    uint8_t u8s[3];
    uint8_t *u8s_extra;
};


void
ni_init_proto_instructions(NError* error);

uint16_t
n_proto_instruction_size(NProtoInstruction* self);

void
n_emit_instruction(NByteWriter* writer, NProtoInstruction* instr,
                   NError* error);

void
n_resolve_instruction_anchors(NProtoInstruction* self, uint16_t offset,
                              NAnchorMap* anchor_map, NError* error);

void
n_destruct_proto_instruction(NProtoInstruction* self);

NProtoInstruction
n_proto_nop();

NProtoInstruction
n_proto_halt();

NProtoInstruction
n_proto_jump_unless(uint8_t cond, uint16_t anchor);

NProtoInstruction
n_proto_jump(uint16_t anchor);

NProtoInstruction
n_proto_call(uint8_t dest, uint8_t target, uint8_t n_args, uint8_t* args,
             NError* error);

NProtoInstruction
n_proto_global_ref(uint8_t dest, uint16_t source);

NProtoInstruction
n_proto_global_set(uint16_t dest, uint8_t source);

NProtoInstruction
n_proto_return(uint8_t source);

#endif /*N_A_PROTO_INSTRUCTIONS_H*/
