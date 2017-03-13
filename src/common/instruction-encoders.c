#include "instruction-encoders.h"

int
n_encode_op_nop(unsigned char* stream) {
    return 0;
}


int
n_encode_op_halt(unsigned char* stream) {
    return 0;
}


int
n_encode_op_load_i16(unsigned char* stream, uint8_t dest, int16_t value) {
    return 0;
}


int
n_encode_op_jump_unless(unsigned char* stream, uint8_t cond, int16_t offset) {
    return 0;
}


int
n_encode_op_jump(unsigned char* stream, int16_t offset) {
    return 0;
}


int
n_encode_op_call(unsigned char* stream, uint8_t dest, uint8_t target,
                 uint8_t n_args) {
    return 0;
}
