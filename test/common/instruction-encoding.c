#include <stdlib.h>

#include "../test.h"
#include "common/instruction-encoders.h"
#include "common/instruction-decoders.h"
#include "common/compatibility/stdint.h"
#include "common/opcodes.h"
#include "common/errors.h"


static
NError ERR;

static unsigned char BUFFER[128];

CONSTRUCTOR(constructor) {

}


SETUP(setup) {
    int i;
    ERR = n_error_ok();
    for (i = 0; i < 128; i++) {
        BUFFER[i] = 0;
    }
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
}


TEST(encode_halt_has_right_opcode) {
    n_encode_op_halt(BUFFER);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_HALT));
}


TEST(encode_halt_uses_one_byte) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_halt(BUFFER);

    ASSERT(EQ_INT(used_bytes, 1));
}


TEST(encode_nop_has_right_opcode) {
    n_encode_op_nop(BUFFER);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_NOP));
}


TEST(encode_nop_uses_one_byte) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_nop(BUFFER);

    ASSERT(EQ_INT(used_bytes, 1));
}


TEST(encode_load_i16_has_right_opcode) {
    n_encode_op_load_i16(BUFFER, 0, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_LOAD_I16));
}


TEST(encode_load_i16_uses_four_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_load_i16(BUFFER, 0, 0);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_load_i16_uses_four_bytes) {
    uint8_t dest;
    int16_t value;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_load_i16(BUFFER, &dest, &value);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_load_i16_reverts_encode) {
    int16_t arg = 13422;
    int16_t d_arg;
    uint8_t dest = 5;
    uint8_t d_dest;
    n_encode_op_load_i16(BUFFER, dest, arg);
    n_decode_op_load_i16(BUFFER, &d_dest, &d_arg);

    ASSERT(EQ_INT(d_arg, arg));
    ASSERT(EQ_UINT(d_dest, dest));
}


TEST(encode_jump_unless_has_right_opcode) {
    n_encode_op_jump_unless(BUFFER, 0, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_JUMP_UNLESS));
}


TEST(encode_jump_unless_uses_four_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_jump_unless(BUFFER, 0, 0);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_jump_unless_uses_four_bytes) {
    uint8_t dest;
    int16_t offset;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_jump_unless(BUFFER, &dest, &offset);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_jump_unless_reverts_encode) {
    int16_t offset = 13422;
    int16_t d_offset;
    uint8_t cond = 5;
    uint8_t d_cond;
    n_encode_op_jump_unless(BUFFER, cond, offset);
    n_decode_op_jump_unless(BUFFER, &d_cond, &d_offset);

    ASSERT(EQ_UINT(d_cond, cond));
    ASSERT(EQ_INT(d_offset, offset));
}


TEST(encode_jump_has_right_opcode) {
    n_encode_op_jump(BUFFER, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_JUMP_UNLESS));
}


TEST(encode_jump_uses_three_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_jump(BUFFER, 0);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_jump_uses_three_bytes) {
    int16_t offset;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_jump(BUFFER, &offset);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_jump_reverts_encode) {
    int16_t offset = 13422;
    int16_t d_offset;
    n_encode_op_jump(BUFFER, offset);
    n_decode_op_jump(BUFFER, &d_offset);

    ASSERT(EQ_INT(d_offset, offset));
}


TEST(encode_call_has_right_opcode) {
    n_encode_op_call(BUFFER, 0, 0, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_CALL));
}


TEST(encode_call_uses_four_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_call(BUFFER, 0, 0, 0);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_call_uses_four_bytes) {
    uint8_t dest;
    uint8_t target;
    uint8_t n_args;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_call(BUFFER, &dest, &target, &n_args);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_call_reverts_encode) {
    uint8_t dest = 134;
    uint8_t d_dest;
    uint8_t target = 22;
    uint8_t d_target;
    uint8_t n_args = 5;
    uint8_t d_n_args;
    
    n_encode_op_call(BUFFER, dest, target, n_args);
    n_decode_op_call(BUFFER, &d_dest, &d_target, &d_n_args);

    ASSERT(EQ_UINT(dest, d_dest));
    ASSERT(EQ_UINT(target, d_target));
    ASSERT(EQ_UINT(n_args, d_n_args));
}


AtTest* tests[] = {
    &encode_halt_has_right_opcode,
    &encode_halt_uses_one_byte,

    &encode_nop_has_right_opcode,
    &encode_nop_uses_one_byte,

    &encode_load_i16_has_right_opcode,
    &encode_load_i16_uses_four_bytes,
    &decode_load_i16_uses_four_bytes,
    &decode_load_i16_reverts_encode,

    &encode_jump_unless_has_right_opcode,
    &encode_jump_unless_uses_four_bytes,
    &decode_jump_unless_uses_four_bytes,
    &decode_jump_unless_reverts_encode,

    &encode_jump_has_right_opcode,
    &encode_jump_uses_three_bytes,
    &decode_jump_uses_three_bytes,
    &decode_jump_reverts_encode,

    &encode_call_has_right_opcode,
    &encode_call_uses_four_bytes,
    &decode_call_uses_four_bytes,
    &decode_call_reverts_encode,

    NULL
};

TEST_RUNNER("InstructionEncoding", tests, constructor, NULL, setup, teardown)
