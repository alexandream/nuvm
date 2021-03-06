#include <stdlib.h>

#include "../test.h"
#include "common/instruction-encoders.h"
#include "common/instruction-decoders.h"
#include "common/compatibility/stdint.h"
#include "common/opcodes.h"
#include "common/errors.h"


static unsigned char BUFFER[128];

SETUP(setup) {
    int i;
    for (i = 0; i < 128; i++) {
        BUFFER[i] = 0;
    }
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

    ASSERT(EQ_UINT(BUFFER[0], N_OP_JUMP));
}


TEST(encode_jump_uses_three_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_jump(BUFFER, 0);

    ASSERT(EQ_INT(used_bytes, 3));
}


TEST(decode_jump_uses_three_bytes) {
    int16_t offset;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_jump(BUFFER, &offset);

    ASSERT(EQ_INT(used_bytes, 3));
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


TEST(encode_global_ref_has_right_opcode) {
    n_encode_op_global_ref(BUFFER, 0, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_GLOBAL_REF));
}


TEST(encode_global_ref_uses_four_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_global_ref(BUFFER, 0, 0);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_global_ref_uses_four_bytes) {
    uint8_t dst;
    uint16_t src;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_global_ref(BUFFER, &dst, &src);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_global_ref_reverts_encode) {
    uint8_t dst = 0x12;
    uint16_t src = 0x3456;
    uint8_t d_dst;
    uint16_t d_src;

    n_encode_op_global_ref(BUFFER, dst, src);
    n_decode_op_global_ref(BUFFER, &d_dst, &d_src);

    ASSERT(EQ_UINT(d_dst, dst));
    ASSERT(EQ_UINT(d_src, src));
}


TEST(encode_global_set_has_right_opcode) {
    n_encode_op_global_set(BUFFER, 0, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_GLOBAL_SET));
}


TEST(encode_global_set_uses_four_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_global_set(BUFFER, 0, 0);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_global_set_uses_four_bytes) {
    uint16_t dst;
    uint8_t src;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_global_set(BUFFER, &dst, &src);

    ASSERT(EQ_INT(used_bytes, 4));
}


TEST(decode_global_set_reverts_encode) {
    uint16_t dst = 0x1234;
    uint8_t src = 0x56;
    uint16_t d_dst;
    uint8_t d_src;

    n_encode_op_global_set(BUFFER, dst, src);
    n_decode_op_global_set(BUFFER, &d_dst, &d_src);

    ASSERT(EQ_UINT(d_dst, dst));
    ASSERT(EQ_UINT(d_src, src));
}


TEST(encode_return_has_right_opcode) {
    n_encode_op_return(BUFFER, 0);

    ASSERT(EQ_UINT(BUFFER[0], N_OP_RETURN));
}


TEST(encode_return_uses_two_bytes) {
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_encode_op_return(BUFFER, 0);

    ASSERT(EQ_INT(used_bytes, 2));

}


TEST(decode_return_uses_two_bytes) {
    uint8_t src;
    /* The opcode and arguments are irrelevant to this test. */
    int used_bytes = n_decode_op_return(BUFFER, &src);

    ASSERT(EQ_INT(used_bytes, 2));

}


TEST(decode_return_reverts_encode) {
    uint8_t src = 0x34;
    uint8_t d_src;

    n_encode_op_return(BUFFER, src);
    n_decode_op_return(BUFFER, &d_src);

    ASSERT(EQ_UINT(d_src, src));
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

    &encode_global_ref_has_right_opcode,
    &encode_global_ref_uses_four_bytes,
    &decode_global_ref_uses_four_bytes,
    &decode_global_ref_reverts_encode,

    &encode_global_set_has_right_opcode,
    &encode_global_set_uses_four_bytes,
    &decode_global_set_uses_four_bytes,
    &decode_global_set_reverts_encode,

    &encode_return_has_right_opcode,
    &encode_return_uses_two_bytes,
    &decode_return_uses_two_bytes,
    &decode_return_reverts_encode,
    NULL
};

TEST_RUNNER("InstructionEncoding", tests, NULL, NULL, setup, NULL)
