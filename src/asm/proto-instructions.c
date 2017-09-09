#include "../common/compatibility/stdint.h"
#include "../common/errors.h"
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


int
ni_init_proto_instructions(void) {
    static int INITIALIZED = 0;
    if (!INITIALIZED) {
        NError error = n_error_ok();
        INITIALIZED = 1;
        if (ni_init_errors() < 0) {
            return -1;
        }

        BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", &error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -2;
        }

        init_vtables();
    }
    return 0;
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
n_proto_jump_unless(uint8_t cond, int16_t offset) {
    NProtoInstruction result;
    result.vtable = &JUMP_UNLESS_VTABLE;
    result.u8s[0] = cond;
    result.i16s[0] = offset;
    return result;
}


NProtoInstruction
n_proto_jump(int16_t offset) {
    NProtoInstruction result;
    result.vtable = &JUMP_VTABLE;
    result.i16s[0] = offset;
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
                        "the argument list on call instruction.", NULL, NULL);
            return  result;
        }
    }

    result.vtable = &GLOBAL_REF_VTABLE;
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
    result.u8s[0] = dest;
    result.u16s[0] = source;
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
    return 0;
}


static void
nop_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static uint16_t
halt_size(NProtoInstruction* self) {
    return 0;
}


static void
halt_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static uint16_t
jump_unless_size(NProtoInstruction* self) {
    return 0;
}


static void
jump_unless_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static void
jump_unless_resolve_anchors(NProtoInstruction* self, uint16_t offset,
                            NAnchorMap* anchor_map, NError* eror) {

}


static uint16_t
jump_size(NProtoInstruction* self) {
    return 0;
}


static void
jump_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static void
jump_resolve_anchors(NProtoInstruction* self, uint16_t offset,
                  NAnchorMap* anchor_map, NError* eror) {

}


static uint16_t
call_size(NProtoInstruction* self) {
    return 0;
}


static void
call_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static void
call_destruct(NProtoInstruction* self) {
    if (self->u8s_extra != NULL) {
        free(self->u8s_extra);
    }
}


static uint16_t
return_size(NProtoInstruction* self) {
    return 0;
}


static void
return_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static uint16_t
global_ref_size(NProtoInstruction* self) {
    return 0;
}


static void
global_ref_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

}


static uint16_t
global_set_size(NProtoInstruction* self) {
    return 0;
}


static void
global_set_emit(NByteWriter* writer, NProtoInstruction* instr, NError* error) {

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
