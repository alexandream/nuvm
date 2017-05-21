#include "instruction-encoders.h"
#include "opcodes.h"

int
n_encode_op_nop(unsigned char* stream) {
    stream[0] = N_OP_NOP;
    return 1;
}


int
n_encode_op_halt(unsigned char* stream) {
    stream[0] = N_OP_HALT;
    return 1;
}


int
n_encode_op_load_i16(unsigned char* stream, uint8_t dest, int16_t value) {
    unsigned char* value_bytes = (unsigned char*) &value;
    stream[0] = N_OP_LOAD_I16;
    stream[1] = dest;
    stream[2] = value_bytes[1];
    stream[3] = value_bytes[0];
    return 4;
}


int
n_encode_op_jump_unless(unsigned char* stream, uint8_t cond, int16_t offset) {
    unsigned char* offset_bytes = (unsigned char*) &offset;
    stream[0] = N_OP_JUMP_UNLESS;
    stream[1] = cond;
    stream[2] = offset_bytes[1];
    stream[3] = offset_bytes[0];
    return 4;
}


int
n_encode_op_jump(unsigned char* stream, int16_t offset) {
    unsigned char* offset_bytes = (unsigned char*) &offset;
    stream[0] = N_OP_JUMP;
    stream[1] = offset_bytes[1];
    stream[2] = offset_bytes[0];
    return 3;
}


int
n_encode_op_call(unsigned char* stream, uint8_t dest, uint8_t target,
                 uint8_t n_args) {
    stream[0] = N_OP_CALL;
    stream[1] = dest;
    stream[2] = target;
    stream[3] = n_args;
    return 4;
}


int
n_encode_op_global_ref(unsigned char* stream, uint8_t dest, uint16_t source) {
    unsigned char* source_bytes = (unsigned char*) &source;
    stream[0] = N_OP_GLOBAL_REF;
    stream[1] = dest;
    stream[2] = source_bytes[1];
    stream[3] = source_bytes[0];
    return 4;
}


int
n_encode_op_global_set(unsigned char* stream, uint16_t dest, uint8_t source) {
    unsigned char* dest_bytes = (unsigned char*) &dest;
    stream[0] = N_OP_GLOBAL_SET;
    stream[1] = dest_bytes[1];
    stream[2] = dest_bytes[0];
    stream[3] = source;
    return 4;
}


int
n_encode_op_return(unsigned char* stream, uint8_t source) {
    stream[0] = N_OP_RETURN;
    stream[1] = source;
    return 2;
}
