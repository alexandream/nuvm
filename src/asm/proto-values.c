#include "../common/common.h"
#include "../common/errors.h"
#include "../common/byte-writers.h"
#include "../common/compatibility/stdint.h"

#include "proto-instructions.h"

#include "proto-values.h"

/* Instantiate the vector "template" for the NProtoProcedure's instructions. */
#define VECTOR_T_CLEANUP
#include "../common/templates/vector.h"

#define VECTOR_T_STRUCT InstructionVector
#define VECTOR_T_ELEMENT_T NProtoInstruction
#include "../common/templates/vector.h"


typedef struct InstructionVector InstructionVector;
VECTOR_T_D_STRUCT;

static VECTOR_T_I_CONSTRUCT(ivec_construct)
static VECTOR_T_I_DESTRUCT(ivec_destruct)
static VECTOR_T_I_GET_REF_UNCHECKED(ivec_get_ref)
static VECTOR_T_I_PUSH(ivec_push)

/* Instantiate the vector "template" for the NProtoProcedure's anchors. */
#define VECTOR_T_CLEANUP
#include "../common/templates/vector.h"

#define VECTOR_T_STRUCT AnchorVector
#define VECTOR_T_ELEMENT_T uint16_t
#include "../common/templates/vector.h"
typedef struct AnchorVector AnchorVector;
VECTOR_T_D_STRUCT;

static VECTOR_T_I_CONSTRUCT(avec_construct)
static VECTOR_T_I_DESTRUCT(avec_destruct)
static VECTOR_T_I_GET_REF_UNCHECKED(avec_get_ref)
static VECTOR_T_I_PUSH(avec_push)
static VECTOR_T_I_SET(avec_set)


typedef struct NProtoValueVTable NProtoValueVTable;

struct NProtoValueVTable {
    uint16_t (*code_size)(NProtoValue*);
    void (*emit_declaration)(NByteWriter*, NProtoValue*, uint32_t, NError*);
    void (*emit_code)(NByteWriter*, NProtoValue*, NError*);
    void (*resolve_anchors)(NProtoValue*, NError*);
    void (*destroy)(NProtoValue*);
};

struct NProtoValue {
    NProtoValueVTable* vtable;
};


struct NProtoFixnum32 {
    NProtoValue parent;
    int32_t value;
};


struct NProtoProcedure {
    NProtoValue parent;
    InstructionVector instructions;
    AnchorVector anchors;
    uint8_t min_locals;
    uint8_t max_locals;
};


static
NProtoValueVTable FIXNUM32_VTABLE,
                  PROCEDURE_VTABLE;

static
NAnchorMapVTable  PROC_ANCHOR_MAP_VTABLE;

static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* ILLEGAL_ARGUMENT = NULL;

static void
init_vtables(void);

static void
construct_proto_value(NProtoValue* self, NProtoValueVTable* vtable);


int
ni_init_proto_values(void) {
    NError error = n_error_ok();

    n_init_common(&error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -1;
    }

    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    ILLEGAL_ARGUMENT = n_error_type("nuvm.IllegalArgument", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -3;
    }

    if (ni_init_proto_instructions() < 0) {
        return -4;
    }
    init_vtables();

    return 0;
}

uint16_t
ni_proto_value_code_size(NProtoValue* self) {
    return self->vtable->code_size(self);
}


void
ni_resolve_anchors(NProtoValue* self, NError *error) {
    if (self->vtable->resolve_anchors != NULL) {
        self->vtable->resolve_anchors(self, error);
    }
}


void
ni_emit_proto_value_declaration(NByteWriter* writer, NProtoValue* value,
                                uint32_t code_offset, NError* error) {
    if (value->vtable->emit_declaration != NULL) {
        value->vtable->emit_declaration(writer, value, code_offset, error);
    }
}


void
ni_emit_proto_value_code(NByteWriter* writer, NProtoValue* value,
                         NError* error) {
    if (value->vtable->emit_code != NULL) {
        value->vtable->emit_code(writer, value, error);
    }

}


NProtoFixnum32*
ni_create_proto_fixnum32(int32_t value, NError* error) {
    NProtoFixnum32* self = malloc(sizeof(NProtoFixnum32));
    if (self == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                    "the NProtoFixnum32");
        return NULL;
    }
    construct_proto_value((NProtoValue*) self, &FIXNUM32_VTABLE);

    self->value = value;
    return self;
}


NProtoProcedure *
ni_create_proto_procedure(uint8_t min_locals, uint8_t max_locals,
                          NError *error) {
    NProtoProcedure* self = malloc(sizeof(NProtoProcedure));
    int vec_error = 0;
    if (self == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                    "the NProtoProcedure");
        return NULL;
    }
    construct_proto_value((NProtoValue*) self, &PROCEDURE_VTABLE);

    ivec_construct(&self->instructions, 8, &vec_error);
    if (vec_error != 0) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                    "the NProtoProcedure");
        free(self);
        return NULL;
    }

    avec_construct(&self->anchors, 8, &vec_error);
    if (vec_error != 0) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space for "
                    "the NProtoProcedure");
        free(self);
        return NULL;
    }

    self->min_locals = min_locals;
    self->max_locals = max_locals;

    return self;
}


uint16_t
ni_create_anchor(NProtoProcedure* self, NError *error) {
    int vec_error = 0;
    uint16_t next_anchor = (uint16_t) self->anchors.size;
    uint16_t place_holder = N_UNDEFINED_ANCHOR;

    avec_push(&self->anchors, &place_holder, &vec_error);

    if (vec_error != 0) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space to grow "
                    "the anchors vector.");
        return 0;
    }

    return next_anchor;
}


void
ni_add_anchor(NProtoProcedure* self, uint16_t id, NError* error) {
    int vec_error = 0;
    /* The anchor must point to where the next instruction will be. */
    uint16_t current_offset = self->instructions.size;
    uint16_t anchor_offset;

    if (id >= self->anchors.size) {
        n_set_error(error, ILLEGAL_ARGUMENT, "Unknown anchor id while trying "
                    "to add anchor definition.");
        return;
    }

    anchor_offset = *avec_get_ref(&self->anchors, id);
    if (anchor_offset != N_UNDEFINED_ANCHOR) {
        /* Anchor is already defined. What's wrong? */
        n_set_error(error, ILLEGAL_ARGUMENT, "Redefinition of anchor.");
        return;
    }

    avec_set(&self->anchors, id, &current_offset, &vec_error);
}



static void
add_proto_instruction(NProtoProcedure* self, NProtoInstruction* instr,
                      NError* error) {
    int vec_error = 0;
    ivec_push(&self->instructions, instr, &vec_error);
    if (vec_error != 0) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate space to grow "
                    "the instructions vector.");
    }
}



void
ni_add_proto_nop(NProtoProcedure* self, NError* error) {
    NProtoInstruction instr = n_proto_nop();
    add_proto_instruction(self, &instr, error);
}


void
ni_add_proto_halt(NProtoProcedure* self, NError* error) {
    NProtoInstruction instr = n_proto_halt();
    add_proto_instruction(self, &instr, error);
}


void
ni_add_proto_jump_unless(NProtoProcedure* self, uint8_t cond, uint16_t anchor,
                         NError *error) {
    NProtoInstruction instr = n_proto_jump_unless(cond, anchor);
    add_proto_instruction(self, &instr, error);

}


void
ni_add_proto_jump(NProtoProcedure* self, uint16_t anchor, NError *error) {

    NProtoInstruction instr = n_proto_jump(anchor);
    add_proto_instruction(self, &instr, error);
}


void
ni_add_proto_call(NProtoProcedure* self, uint8_t dest, uint8_t target,
                  uint8_t n_args, uint8_t* args, NError* error) {
    NProtoInstruction instr = n_proto_call(dest, target, n_args, args, error);
    if (!n_is_ok(error)) return;
    add_proto_instruction(self, &instr, error);
}


void
ni_add_proto_global_ref(NProtoProcedure* self, uint8_t dest, uint16_t source,
                        NError* error) {
    NProtoInstruction instr = n_proto_global_ref(dest, source);
    add_proto_instruction(self, &instr, error);
}


void
ni_add_proto_global_set(NProtoProcedure* self, uint16_t dest, uint8_t source,
                        NError* error) {
    NProtoInstruction instr = n_proto_global_set(dest, source);
    add_proto_instruction(self, &instr, error);
}


void
ni_add_proto_return(NProtoProcedure* self, uint8_t source, NError* error) {
    NProtoInstruction instr = n_proto_return(source);
    add_proto_instruction(self, &instr, error);
}




static void
construct_proto_value(NProtoValue* self, NProtoValueVTable* vtable) {
    self->vtable = vtable;
}


static uint16_t
fixnum_code_size(NProtoValue* self) {
    return 0;
}


static void
fixnum_emit_declaration(NByteWriter* writer, NProtoValue* value,
                        uint32_t code_offset, NError* error) {
#define CHECK_ERROR ON_ERROR(error, return);
    NProtoFixnum32 * self = (NProtoFixnum32*) value;

    n_write_byte(writer, 0x00, error);                      CHECK_ERROR;
    n_write_int32(writer, self->value, error);
#undef CHECK_ERROR
}


static uint16_t
procedure_code_size(NProtoValue* generic_self) {
    NProtoProcedure *self = (NProtoProcedure*) generic_self;
    uint32_t total_size = 0;
    size_t i;

    for (i = 0; i < self->instructions.size; i++) {
        NProtoInstruction* instr = ivec_get_ref(&self->instructions, i);
        total_size += n_proto_instruction_size(instr);
    }
    return total_size;
}


static void
procedure_emit_declaration(NByteWriter* writer, NProtoValue* value,
                           uint32_t code_offset, NError* error) {
#define CHECK_ERROR ON_ERROR(error, return);
    NProtoProcedure * self = (NProtoProcedure*) value;
    n_write_byte(writer, 0x01, error);                          CHECK_ERROR;
    n_write_uint32(writer, code_offset, error);                 CHECK_ERROR;
    n_write_byte(writer, self->min_locals, error);              CHECK_ERROR;
    n_write_byte(writer, self->max_locals, error);              CHECK_ERROR;
    n_write_uint16(writer, procedure_code_size(value), error);
#undef CHECK_ERROR

}

typedef struct ProcedureAnchorMap ProcedureAnchorMap;
struct ProcedureAnchorMap {
    NAnchorMap parent;
    NProtoProcedure* proc;
};



static ProcedureAnchorMap
procedure_anchor_map(NProtoProcedure* self) {
    ProcedureAnchorMap result;
    result.parent.vtable = &PROC_ANCHOR_MAP_VTABLE;
    result.proc = self;
    return result;
}


static void
procedure_emit_code(NByteWriter* writer, NProtoValue* value, NError* error) {
#define CHECK_ERROR ON_ERROR(error, return);
    NProtoProcedure * self = (NProtoProcedure*) value;
    size_t i;
    for (i = 0; i < self->instructions.size; i++) {
        NProtoInstruction *instr = ivec_get_ref(&self->instructions, i);
        n_emit_instruction(writer, instr, error);               CHECK_ERROR;
    }
#undef CHECK_ERROR
}


static void
procedure_resolve_anchors(NProtoValue* generic_self, NError* error) {
#define CHECK_ERROR ON_ERROR(error, return);
    NProtoProcedure* self = (NProtoProcedure*) generic_self;
    uint16_t cur_offset = 0;
    size_t i;
    ProcedureAnchorMap proc_anchor_map = procedure_anchor_map(self);
    NAnchorMap* anchor_map = (NAnchorMap*) &proc_anchor_map;
    for (i = 0; i < self->instructions.size; i++) {
        NProtoInstruction *instr = ivec_get_ref(&self->instructions, i);
        n_resolve_instruction_anchors(instr, cur_offset, anchor_map, error);
        CHECK_ERROR;
    }
}


static void
procedure_destroy(NProtoValue* generic_self) {
    NProtoProcedure* self = (NProtoProcedure*) generic_self;
    size_t i;
    for (i = 0; i < self->instructions.size; i++) {
        n_destruct_proto_instruction(ivec_get_ref(&self->instructions, i));
    }
    ivec_destruct(&self->instructions);
    avec_destruct(&self->anchors);

}

static int
proc_anchor_map_has_anchor(NAnchorMap* generic_self, int id) {
    ProcedureAnchorMap* self = (ProcedureAnchorMap*) generic_self;
    if (id >= 0 && (unsigned) id < self->proc->anchors.size) {
        return (*avec_get_ref(&self->proc->anchors, id)) != N_UNDEFINED_ANCHOR;
    }
    return 0;
}


static uint16_t
proc_anchor_map_get_offset(NAnchorMap* generic_self, int id) {
    ProcedureAnchorMap* self = (ProcedureAnchorMap*) generic_self;
    return *avec_get_ref(&self->proc->anchors, id);
}



static void
init_vtables(void) {
    FIXNUM32_VTABLE.code_size        = fixnum_code_size;
    FIXNUM32_VTABLE.emit_declaration = fixnum_emit_declaration;
    FIXNUM32_VTABLE.emit_code        = NULL;
    FIXNUM32_VTABLE.resolve_anchors  = NULL;
    FIXNUM32_VTABLE.destroy          = NULL;

    PROCEDURE_VTABLE.code_size        = procedure_code_size;
    PROCEDURE_VTABLE.emit_declaration = procedure_emit_declaration;
    PROCEDURE_VTABLE.emit_code        = procedure_emit_code;
    PROCEDURE_VTABLE.resolve_anchors  = procedure_resolve_anchors;
    PROCEDURE_VTABLE.destroy          = procedure_destroy;

    PROC_ANCHOR_MAP_VTABLE.has_anchor = proc_anchor_map_has_anchor;
    PROC_ANCHOR_MAP_VTABLE.get_offset = proc_anchor_map_get_offset;
}

