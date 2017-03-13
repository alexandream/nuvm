#ifndef N_C_INSTRUCTION_DECODERS_H
#define N_C_INSTRUCTION_DECODERS_H


static int
n_decode_op_load_i16(unsigned char* stream, uint8_t *dest, int16_t *value) {
    unsigned char* value_bytes = (unsigned char*) value;
    *dest          = stream[1];
    value_bytes[1] = stream[2];
    value_bytes[0] = stream[3];
    return 4;
}


static int
n_decode_op_jump_unless(unsigned char* stream, uint8_t *cond, int16_t *offset) {
    unsigned char* offset_bytes = (unsigned char*) offset;
    *cond           = stream[1];
    offset_bytes[1] = stream[2];
    offset_bytes[0] = stream[3];
    return 4;
}


static int
n_decode_op_jump(unsigned char* stream, int16_t *offset) {
    unsigned char* offset_bytes = (unsigned char*) offset;
    offset_bytes[1] = stream[1];
    offset_bytes[0] = stream[2];
    return 3;
}


int
n_decode_op_call(unsigned char* stream, uint8_t *dest, uint8_t *target,
                 uint8_t *n_args) {
    *dest   = stream[1];
    *target = stream[2];
    *n_args = stream[3];
    return 4;
}
#endif /* N_C_INSTRUCTION_ENCODING_H*/
