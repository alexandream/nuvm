#include "../test.h"

#include "common/errors.h"
#include "common/byte-readers.h"
#include "common/byte-writers.h"
#include "common/opcodes.h"

#include "asm/asm.h"
#include "asm/proto-values.h"

static uint8_t BUFFER[256];
static NByteWriter* WRITER = NULL;
static NByteReader* READER = NULL;
static NError ERR;

CONSTRUCTOR(constructor) {
    NT_INITIALIZE_MODULE(n_init_asm);
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
    ERR = n_error_ok();
    n_destroy_byte_writer(WRITER, &ERR);

    if (!n_is_ok(&ERR)) {
        ERROR("Could not destroy the byte writer after test.", NULL);
    }
}

TEST(fixnum_code_size_is_zero) {
    NProtoFixnum32 *proto_fixnum = ni_create_proto_fixnum32(-1234567, &ERR);
    NProtoValue *proto_value = (NProtoValue*) proto_fixnum;
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(proto_fixnum != NULL));

    ASSERT(EQ_UINT(ni_proto_value_code_size(proto_value), 0));
}


TEST(fixnum_emits_fixnum32_decl) {
    NProtoFixnum32 *proto_fixnum = ni_create_proto_fixnum32(-1234567, &ERR);
    NProtoValue *proto_value = (NProtoValue*) proto_fixnum;
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(proto_fixnum != NULL));

    ni_emit_proto_value_declaration(WRITER, proto_value, 0, &ERR);
    ASSERT(IS_OK(ERR));

    /*0x00, int32*/
    ASSERT(EQ_UINT(n_read_byte(READER, &ERR), 0x00));
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_INT(n_read_int32(READER, &ERR), -1234567));
    ASSERT(IS_OK(ERR));
}


TEST(fixnum_emit_code_has_no_error) {
    NProtoFixnum32 *proto_fixnum = ni_create_proto_fixnum32(-1234567, &ERR);
    NProtoValue *proto_value = (NProtoValue*) proto_fixnum;
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(proto_fixnum != NULL));

    ni_emit_proto_value_code(WRITER, proto_value, &ERR);
    ASSERT(IS_OK(ERR));
}


TEST(fixnum_resolve_anchors_has_no_error) {
    NProtoFixnum32 *proto_fixnum = ni_create_proto_fixnum32(-1234567, &ERR);
    NProtoValue *proto_value = (NProtoValue*) proto_fixnum;
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(proto_fixnum != NULL));

    ni_resolve_anchors(proto_value, &ERR);
    ASSERT(IS_OK(ERR));
}


TEST(proc_code_size_adds_instructions) {
    NProtoProcedure* proc = ni_create_proto_procedure(1, 1, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_proto_nop(proc, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_proto_nop(proc, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_proto_nop(proc, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(ni_proto_value_code_size((NProtoValue*) proc), 3));
}


TEST(proc_emits_procedure_declaration) {
    NProtoProcedure* proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    NProtoValue *proto_value = (NProtoValue*) proto_proc;
    uint16_t code_size;

    /* Procedures must have at least one instruction. */
    ni_add_proto_nop(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    ni_emit_proto_value_declaration(WRITER, proto_value, 123, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(n_read_byte(READER, &ERR), 0x01));
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(n_read_uint32(READER, &ERR), 123));
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(n_read_byte(READER, &ERR), 1));
    ASSERT(IS_OK(ERR));

    ASSERT(EQ_UINT(n_read_byte(READER, &ERR), 5));
    ASSERT(IS_OK(ERR));

    code_size = ni_proto_value_code_size(proto_value);
    ASSERT(EQ_UINT(n_read_uint16(READER, &ERR), code_size));
    ASSERT(IS_OK(ERR));
}


TEST(proc_emits_instructions_as_code) {
    NProtoProcedure* proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    NProtoValue* proto_value = (NProtoValue*) proto_proc;
    uint8_t opcode, dest;
    uint16_t source;

    ni_add_proto_nop(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_proto_global_ref(proto_proc, 123, 32145, &ERR);
    ASSERT(IS_OK(ERR));

    ni_emit_proto_value_code(WRITER, proto_value, &ERR);
    ASSERT(IS_OK(ERR));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(opcode, N_OP_NOP));

    opcode = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(opcode, N_OP_GLOBAL_REF));

    dest = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(dest, 123));

    source = n_read_uint16(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(source, 32145));
}


TEST(emit_proc_fails_on_unresolved_anchors) {
    NProtoProcedure* proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    NProtoValue* proto_value = (NProtoValue*) proto_proc;

    ni_add_proto_nop(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    /* The anchor "3" is not defined anywhere. */
    ni_add_proto_jump(proto_proc, 3, &ERR);
    ASSERT(IS_OK(ERR));

    ni_emit_proto_value_code(WRITER, proto_value, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(proc_resolve_anchors_works_on_known_anchor) {
    NProtoProcedure* proto_proc;
    uint16_t anchor;

    proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    ASSERT(IS_OK(ERR));
    anchor = ni_create_anchor(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_anchor(proto_proc, anchor, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_proto_jump(proto_proc, anchor, &ERR);
    ASSERT(IS_OK(ERR));

    ni_resolve_anchors((NProtoValue*) proto_proc, &ERR);
    ASSERT(IS_OK(ERR));
}


TEST(proc_add_anchor_rejects_repeated_anchor) {
    NProtoProcedure* proto_proc;
    uint16_t anchor;

    proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    ASSERT(IS_OK(ERR));

    anchor = ni_create_anchor(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    /* Adding the anchor the first time should be ok. */
    ni_add_anchor(proto_proc, anchor, &ERR);
    ASSERT(IS_OK(ERR));

    /* Adding it the second time should break. */
    ni_add_anchor(proto_proc, anchor, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(proc_resolve_anchors_detects_undefined_anchor) {
    NProtoProcedure* proto_proc;
    uint16_t anchor;

    proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    ASSERT(IS_OK(ERR));
    anchor = ni_create_anchor(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    /* The anchor below was created, but not added. */
    ni_add_proto_jump(proto_proc, anchor, &ERR);
    ASSERT(IS_OK(ERR));

    ni_resolve_anchors((NProtoValue*) proto_proc, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


TEST(proc_resolve_anchors_detects_unknown_anchor) {
    NProtoProcedure* proto_proc;
    uint16_t anchor;

    proto_proc = ni_create_proto_procedure(1, 5, &ERR);
    ASSERT(IS_OK(ERR));
    anchor = ni_create_anchor(proto_proc, &ERR);
    ASSERT(IS_OK(ERR));

    ni_add_anchor(proto_proc, anchor, &ERR);
    ASSERT(IS_OK(ERR));

    /* Adding the wrong anchor by adding 1 to its id. */
    ni_add_proto_jump(proto_proc, anchor +1, &ERR);
    ASSERT(IS_OK(ERR));

    ni_resolve_anchors((NProtoValue*) proto_proc, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.IllegalArgument"));
}


AtTest* tests[] = {
    &fixnum_code_size_is_zero,
    &fixnum_emits_fixnum32_decl,
    &fixnum_emit_code_has_no_error,
    &fixnum_resolve_anchors_has_no_error,
    &proc_code_size_adds_instructions,
    &proc_emits_procedure_declaration,
    &proc_emits_instructions_as_code,
    &proc_resolve_anchors_works_on_known_anchor,
    &proc_resolve_anchors_detects_undefined_anchor,
    &proc_resolve_anchors_detects_unknown_anchor,
    &emit_proc_fails_on_unresolved_anchors,
    &proc_add_anchor_rejects_repeated_anchor,
    NULL
};

TEST_RUNNER("ProtoValues", tests, constructor, NULL, setup, teardown)
