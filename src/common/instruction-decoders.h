#ifndef N_C_INSTRUCTION_DECODERS_H
#define N_C_INSTRUCTION_DECODERS_H


static int
n_decode_op_load_i16(unsigned char* stream, uint8_t *dest, int16_t *value) {
    return 0;
}


static int
n_decode_op_jump_unless(unsigned char* stream, uint8_t *cond, int16_t *offset) {
    return 0;
}


static int
n_decode_op_jump(unsigned char* stream, int16_t *offset) {
    return 0;
}


int
n_decode_op_call(unsigned char* stream, uint8_t *dest, uint8_t *target,
                 uint8_t *n_args) {
    return 0;
}
#endif /* N_C_INSTRUCTION_ENCODING_H*/
