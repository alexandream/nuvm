#include "../test.h"

#include "common/errors.h"
#include "common/opcodes.h"
#include "common/byte-writers.h"
#include "common/byte-readers.h"

#include "asm/asm.h"
#include "asm/proto-instructions.h"

typedef struct NDummyAnchorMap NDummyAnchorMap;
struct NDummyAnchorMap {
    NAnchorMap parent;
    uint16_t key;
    uint16_t offset;
};

static uint8_t BUFFER[256];

static NByteWriter* WRITER = NULL;
static NByteReader* READER = NULL;
static NError ERR;

static NAnchorMapVTable dummy_vtable;

static int
has_anchor(NAnchorMap* map, int key);

static uint16_t
get_offset(NAnchorMap* map, int key);

static NDummyAnchorMap
create_anchor_map(int key, uint16_t value);



CONSTRUCTOR(constructor) {
    NT_INITIALIZE_MODULE(n_init_asm);

    dummy_vtable.has_anchor = has_anchor;
    dummy_vtable.get_offset = get_offset;
}


SETUP(setup) {
    ERR = n_error_ok();
    WRITER = n_create_memory_byte_writer(BUFFER, 256, &ERR);
    if (!n_is_ok(&ERR)) {
        n_destroy_error(&ERR);
        ERROR("Could not create memory byte writer for test.", NULL);
    }
    READER = n_new_byte_reader_from_data(BUFFER, 256, &ERR);
    if (!n_is_ok(&ERR)) {
        n_destroy_error(&ERR);
        ERR = n_error_ok();
        n_destroy_byte_writer(WRITER, &ERR);
        ERROR("Could not create memory byte reader for test.", NULL);
    }

}


TEARDOWN(teardown) {
    NError e1 = n_error_ok();
    NError e2 = n_error_ok();
    n_destroy_byte_reader(READER, &e1);
    n_destroy_byte_writer(WRITER, &e2);

    if (!n_is_ok(&e1)) {
        ERROR("Could not destroy the byte reader after test.", NULL);
    }
    if (!n_is_ok(&e2)) {
        ERROR("Could not destroy the byte writer after test.", NULL);
    }
}




TEST(nop_has_correct_size) {
    NProtoInstruction instr = n_proto_nop();
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_NOP)));
}


TEST(nop_emits_correctly) {
    NProtoInstruction instr = n_proto_nop();
    uint8_t opcode;

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(opcode, N_OP_NOP));
}


TEST(halt_has_correct_size) {
    NProtoInstruction instr = n_proto_halt();
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_HALT)));
}


TEST(halt_emits_correctly) {
    NProtoInstruction instr = n_proto_halt();
    uint8_t opcode;

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(opcode, N_OP_HALT));
}


TEST(jump_unless_has_correct_size) {
    NProtoInstruction instr = n_proto_jump_unless(1, 2);
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_JUMP_UNLESS)));
}


TEST(jump_unless_emits_correctly) {
    NProtoInstruction instr = n_proto_jump_unless(17, 123);
    uint8_t opcode, cond;
    int16_t offset;
    NDummyAnchorMap anchor_map = create_anchor_map(123, 55);

    n_resolve_instruction_anchors(&instr, 21, (NAnchorMap*) &anchor_map, &ERR);
    ASSERT(IS_OK(ERR));

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    cond = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    offset = n_read_int16(READER, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(opcode, N_OP_JUMP_UNLESS));
    ASSERT(EQ_UINT(cond, 17));
    ASSERT(EQ_INT(offset, 34));
}


TEST(jump_unless_needs_resolving) {
    NProtoInstruction instr = n_proto_jump_unless(1, 2);
    n_emit_instruction(WRITER, &instr, &ERR);

    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(jump_unless_needs_known_anchor) {
    NDummyAnchorMap anchor_map = create_anchor_map(3, 55);
    NProtoInstruction instr = n_proto_jump_unless(1, 2);

    n_resolve_instruction_anchors(&instr, 1, (NAnchorMap*) &anchor_map, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(jump_has_correct_size) {
    NProtoInstruction instr = n_proto_jump(1);
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_JUMP)));
}


TEST(jump_emits_correctly) {
    NProtoInstruction instr = n_proto_jump(12);
    uint8_t opcode;
    int16_t offset;
    NDummyAnchorMap anchor_map = create_anchor_map(12, 13);

    n_resolve_instruction_anchors(&instr, 34, (NAnchorMap*) &anchor_map, &ERR);
    ASSERT(IS_OK(ERR));

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    offset = n_read_int16(READER, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(opcode, N_OP_JUMP));
    ASSERT(EQ_INT(offset, -21));
}


TEST(jump_needs_resolving) {
    NProtoInstruction instr = n_proto_jump(12);
    n_emit_instruction(WRITER, &instr, &ERR);

    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(jump_needs_known_anchor) {
    NDummyAnchorMap anchor_map = create_anchor_map(3, 55);
    NProtoInstruction instr = n_proto_jump(29);

    n_resolve_instruction_anchors(&instr, 1, (NAnchorMap*) &anchor_map, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(call_has_correct_size) {
    NError error = n_error_ok();
    uint8_t args[] = { 4, 5, 6 };
    NProtoInstruction instr = n_proto_call(1,2,3, args, &error);
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_CALL) + instr.u8s[2]));
}

TEST(call_emits_correctly) {
    uint8_t args[] = { 8, 13, 21 };
    NProtoInstruction instr;
    uint8_t opcode, dest, target, n_args;
    int i;

    instr = n_proto_call(7, 1, 3, args, &ERR);
    ASSERT(IS_OK(ERR));

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    dest = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    target = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    n_args = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(opcode, N_OP_CALL));
    ASSERT(EQ_UINT(dest, 7));
    ASSERT(EQ_UINT(target, 1));
    ASSERT(EQ_UINT(n_args, 3));

    for (i = 0; i < 3; i++) {
        uint8_t arg = n_read_byte(READER, &ERR);
        ASSERT(IS_OK(ERR));
        ASSERT(EQ_UINT(arg, args[i]));
    }
}


TEST(return_has_correct_size) {
    NProtoInstruction instr = n_proto_return(1);
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_RETURN)));
}


TEST(return_emits_correctly) {
    NProtoInstruction instr = n_proto_return(42);
    uint8_t opcode, source;

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    source = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(opcode, N_OP_RETURN));
    ASSERT(EQ_UINT(source, 42));
}


TEST(global_ref_has_correct_size) {
    NProtoInstruction instr = n_proto_global_ref(1, 2);
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_GLOBAL_REF)));
}


TEST(global_ref_emits_correctly) {
    NProtoInstruction instr = n_proto_global_ref(127, 32767);
    uint8_t opcode, dest;
    uint16_t source;

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    dest = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    source = n_read_uint16(READER, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(opcode, N_OP_GLOBAL_REF));
    ASSERT(EQ_UINT(dest, 127));
    ASSERT(EQ_UINT(source, 32767));
}


TEST(global_set_has_correct_size) {
    NProtoInstruction instr = n_proto_global_set(1, 2);
    uint16_t size = n_proto_instruction_size(&instr);
    ASSERT(EQ_UINT(size, n_get_opcode_size(N_OP_GLOBAL_SET)));
}


TEST(global_set_emits_correctly) {
    NProtoInstruction instr = n_proto_global_set(12345, 29);
    uint8_t opcode, source;
    uint16_t dest;

    n_emit_instruction(WRITER, &instr, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    dest = n_read_uint16(READER, &ERR);
    ASSERT(IS_OK(ERR));

    source = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(opcode, N_OP_GLOBAL_SET));
    ASSERT(EQ_UINT(dest, 12345));
    ASSERT(EQ_UINT(source, 29));
}


AtTest* tests[] = {
    &nop_has_correct_size,
    &nop_emits_correctly,
    &halt_has_correct_size,
    &halt_emits_correctly,
    &jump_unless_has_correct_size,
    &jump_unless_emits_correctly,
    &jump_unless_needs_resolving,
    &jump_unless_needs_known_anchor,
    &jump_has_correct_size,
    &jump_emits_correctly,
    &jump_needs_resolving,
    &jump_needs_known_anchor,
    &call_has_correct_size,
    &call_emits_correctly,
    &return_has_correct_size,
    &return_emits_correctly,
    &global_ref_has_correct_size,
    &global_ref_emits_correctly,
    &global_set_has_correct_size,
    &global_set_emits_correctly,
    NULL
};

TEST_RUNNER("ProtoInstructions", tests, constructor, NULL, setup, teardown)


static NDummyAnchorMap
create_anchor_map(int key, uint16_t offset) {
    NDummyAnchorMap self;
    self.parent.vtable = &dummy_vtable;
    self.key = key;
    self.offset = offset;
    return self;
}


static int
has_anchor(NAnchorMap* map, int key) {
    return ((NDummyAnchorMap *) map)->key == key;
}


static uint16_t
get_offset(NAnchorMap* map, int key) {
    NDummyAnchorMap* self = (NDummyAnchorMap*) map;
    if (self->key == key) {
        return self->offset;
    }
    return 0xFFFF;
}
