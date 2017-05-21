#ifndef N_C_INSTRUCTION_ENCODERS_H
#define N_C_INSTRUCTION_ENCODERS_H

#include "compatibility/stdint.h"

int
n_encode_op_nop(unsigned char* stream);

int
n_encode_op_halt(unsigned char* stream);

int
n_encode_op_load_i16(unsigned char* stream, uint8_t dest, int16_t value);

int
n_encode_op_jump_unless(unsigned char* stream, uint8_t cond, int16_t offset);

int
n_encode_op_jump(unsigned char* stream, int16_t offset);

int
n_encode_op_call(unsigned char* stream, uint8_t dest, uint8_t target,
                 uint8_t n_args);

int
n_encode_op_global_ref(unsigned char* stream, uint8_t dest, uint16_t source);

int
n_encode_op_global_set(unsigned char* stream, uint16_t dest, uint8_t source);

int
n_encode_op_arg_ref(unsigned char* stream, uint8_t dest, uint8_t source);

int
n_encode_op_return(unsigned char* stream, uint8_t source);
#endif /* N_C_INSTRUCTION_ENCODING_H*/
