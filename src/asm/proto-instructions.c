#include <string.h>

#include "../common/compatibility/stdint.h"
#include "../common/common.h"
#include "../common/errors.h"
#include "../common/opcodes.h"
#include "../common/byte-writers.h"

#include "proto-instructions.h"

static
NProtoInstructionVTable NOP_VTABLE         = { 0, 0, 0, 0 },
                        HALT_VTABLE        = { 0, 0, 0, 0 },
                        JUMP_UNLESS_VTABLE = { 0, 0, 0, 0 },
                        JUMP_VTABLE        = { 0, 0, 0, 0 },
                        CALL_VTABLE        = { 0, 0, 0, 0 },
                        RETURN_VTABLE      = { 0, 0, 0, 0 },
                        GLOBAL_REF_VTABLE  = { 0, 0, 0, 0 },
                        GLOBAL_SET_VTABLE  = { 0, 0, 0, 0 };

static
void init_vtables(void);

static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

void
ni_init_proto_instructions(NError* error) {
#define EC ON_ERROR(error, return)
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        INITIALIZED = 1;

        BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);       EC;
        ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", error);   EC;

        init_vtables();
    }
#undef EC
}


uint16_t
n_proto_instruction_size(NProtoInstruction* self) {
    return self->vtable->size(self);
}


void
n_emit_instruction(NByteWriter* writer, NProtoInstruction* instr,
                   NError* error) {
    instr->vtable->emit(writer, instr, error);
}


void
n_resolve_instruction_anchors(NProtoInstruction* self, uint16_t offset,
                              NAnchorMap* anchor_map, NError* error) {
    if (self->vtable->resolve_anchors) {
        self->vtable->resolve_anchors(self, offset, anchor_map, error);
    }
}


void
n_destruct_proto_instruction(NProtoInstruction* self) {
    if (self->vtable->destruct) {
        self->vtable->destruct(self);
    }
}


NProtoInstruction
n_proto_nop() {
    NProtoInstruction result;
    result.vtable = &NOP_VTABLE;
    return result;
}


NProtoInstruction
n_proto_halt() {
    NProtoInstruction result;
    result.vtable = &HALT_VTABLE;
    return result;
}


NProtoInstruction
n_proto_jump_unless(uint8_t cond, uint16_t anchor) {
    NProtoInstruction result;
    result.vtable = &JUMP_UNLESS_VTABLE;
    result.u8s[0] = cond;
    result.u16s[0] = anchor;
    /* Mark flag to indicate the labels haven't been resolved. */
    result.u8s[2] = 1;
    return result;
}


NProtoInstruction
n_proto_jump(uint16_t anchor) {
    NProtoInstruction result;
    result.vtable = &JUMP_VTABLE;
    result.u16s[0] = anchor;
    /* Mark flag to indicate the labels haven't been resolved. */
    result.u8s[2] = 1;
    return result;
}


NProtoInstruction
n_proto_call(uint8_t dest, uint8_t target, uint8_t n_args, uint8_t* args,
             NError* error) {
    NProtoInstruction result;
    uint8_t *internal_args = NULL;
    if (n_args > 0) {
        internal_args = malloc(sizeof(uint8_t) * n_args);
        if (internal_args == NULL) {
            n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                        "the argument list on call instruction.");
            return  result;
        }
        memcpy(internal_args, args, sizeof(uint8_t) * n_args);
    }

    result.vtable = &CALL_VTABLE;
    result.u8s[0] = dest;
    result.u8s[1] = target;
    result.u8s[2] = n_args;
    result.u8s_extra = internal_args;
    return result;
}


NProtoInstruction
n_proto_global_ref(uint8_t dest, uint16_t source) {
    NProtoInstruction result;
    result.vtable = &GLOBAL_REF_VTABLE;
    result.u8s[0] = dest;
    result.u16s[0] = source;
    return result;

}


NProtoInstruction
n_proto_global_set(uint16_t dest, uint8_t source) {
    NProtoInstruction result;
    result.vtable = &GLOBAL_SET_VTABLE;
    result.u8s[0] = source;
    result.u16s[0] = dest;
    return result;
}


NProtoInstruction
n_proto_return(uint8_t source) {
    NProtoInstruction result;
    result.vtable = &RETURN_VTABLE;
    result.u8s[0] = source;
    return result;
}




static uint16_t
nop_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_NOP);
}


static void
nop_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
    n_write_byte(writer, N_OP_NOP, error);
}


static uint16_t
halt_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_HALT);
}


static void
halt_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
    n_write_byte(writer, N_OP_HALT, error);
}


static uint16_t
jump_unless_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_JUMP_UNLESS);
}


static void
jump_unless_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
#define EC ON_ERROR(error, return)
    if (instr->u8s[2]) {
        /* Anchors were not resolved, somethings is very wrong. */
        n_set_error(error, ILLEGAL_ARGUMENT, "Trying to emit a Jump Unless "
                    "proto instruction without resolving anchors.");
        return;
    }
    n_write_byte(writer, N_OP_JUMP_UNLESS, error);                 EC;
    n_write_byte(writer, instr->u8s[0], error);                    EC;
    n_write_int16(writer, instr->i16s[0], error);
#undef EC
}


static void
jump_unless_resolve_anchors(NProtoInstruction* self, uint16_t own_offset,
                            NAnchorMap* anchor_map, NError* error) {
    uint16_t anchor_offset;
    int16_t offset;

    if (!anchor_map->vtable->has_anchor(anchor_map, self->u16s[0])) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Anchor not found while trying "
                    "to resolve Jump Unless instruction.");
        return;
    }
    anchor_offset = anchor_map->vtable->get_offset(anchor_map, self->u16s[0]);
    offset = (int16_t) (((int32_t) anchor_offset) - ((int32_t) own_offset));
    self->i16s[0] = offset;
    /* Unmark the flag to indicate anchors were already resolved. */
    self->u8s[2] = 0;
}


static uint16_t
jump_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_JUMP);
}


static void
jump_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
#define EC ON_ERROR(error, return)
    if (instr->u8s[2]) {
        /* Anchors were not resolved, somethings is very wrong. */
        n_set_error(error, ILLEGAL_ARGUMENT, "Trying to emit a Jump proto "
                    "instruction without resolving anchors.");
        return;
    }
    n_write_byte(writer, N_OP_JUMP, error);                       EC;
    n_write_int16(writer, instr->i16s[0], error);
#undef EC

}


static void
jump_resolve_anchors(NProtoInstruction* self, uint16_t own_offset,
                  NAnchorMap* anchor_map, NError* error) {
    uint16_t anchor_offset;
    int16_t offset;

    if (!anchor_map->vtable->has_anchor(anchor_map, self->u16s[0])) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Anchor not found while trying "
                    "to resolve Jump instruction.");
        return;
    }
    anchor_offset = anchor_map->vtable->get_offset(anchor_map, self->u16s[0]);
    offset = (int16_t) (((int32_t) anchor_offset) - ((int32_t) own_offset));
    self->i16s[0] = offset;
    /* Unmark the flag to indicate anchors were already resolved. */
    self->u8s[2] = 0;
}


static uint16_t
call_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_CALL) + self->u8s[2];
}


static void
call_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
#define EC ON_ERROR(error, return)
    int i;
    n_write_byte(writer, N_OP_CALL, error);                       EC;
    n_write_byte(writer, instr->u8s[0], error);                   EC;
    n_write_byte(writer, instr->u8s[1], error);                   EC;
    n_write_byte(writer, instr->u8s[2], error);                   EC;
    if (instr->u8s_extra != NULL) {
        for (i = 0; i < instr->u8s[2]; i++) {
            n_write_byte(writer, instr->u8s_extra[i], error);     EC;
        }
    }
#undef EC

}


static void
call_destruct(NProtoInstruction* self) {
    if (self->u8s_extra != NULL) {
        free(self->u8s_extra);
    }
}


static uint16_t
return_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_RETURN);
}


static void
return_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
#define EC ON_ERROR(error, return)
    n_write_byte(writer, N_OP_RETURN, error);                       EC;
    n_write_byte(writer, instr->u8s[0], error);
#undef EC

}


static uint16_t
global_ref_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_GLOBAL_REF);
}


static void
global_ref_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
#define EC ON_ERROR(error, return)
    n_write_byte(writer, N_OP_GLOBAL_REF, error);                  EC;
    n_write_byte(writer, instr->u8s[0], error);                    EC;
    n_write_uint16(writer, instr->u16s[0], error);
#undef EC
}


static uint16_t
global_set_size(NProtoInstruction* self) {
    return n_get_opcode_size(N_OP_GLOBAL_SET);
}


static void
global_set_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {
#define EC ON_ERROR(error, return)
    n_write_byte(writer, N_OP_GLOBAL_SET, error);                  EC;
    n_write_uint16(writer, instr->u16s[0], error);                 EC;
    n_write_byte(writer, instr->u8s[0], error);
#undef EC

}


static
void init_vtables(void) {
    NOP_VTABLE.size = nop_size;
    NOP_VTABLE.emit = nop_emit;

    HALT_VTABLE.size = halt_size;
    HALT_VTABLE.emit = halt_emit;

    JUMP_UNLESS_VTABLE.size = jump_unless_size;
    JUMP_UNLESS_VTABLE.emit = jump_unless_emit;
    JUMP_UNLESS_VTABLE.resolve_anchors = jump_unless_resolve_anchors;

    JUMP_VTABLE.size = jump_size;
    JUMP_VTABLE.emit = jump_emit;
    JUMP_VTABLE.resolve_anchors = jump_resolve_anchors;

    CALL_VTABLE.size = call_size;
    CALL_VTABLE.emit = call_emit;
    CALL_VTABLE.destruct = call_destruct;

    RETURN_VTABLE.size = return_size;
    RETURN_VTABLE.emit = return_emit;

    GLOBAL_REF_VTABLE.size = global_ref_size;
    GLOBAL_REF_VTABLE.emit = global_ref_emit;

    GLOBAL_SET_VTABLE.size = global_set_size;
    GLOBAL_SET_VTABLE.emit = global_set_emit;
}
