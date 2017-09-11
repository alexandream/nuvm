#ifndef N_C_OPCODES_H
#define N_C_OPCODES_H

#include "compatibility/stdint.h"

enum NOpcode {
 N_OP_NOP          = 0x00,
 N_OP_HALT         = 0x01,
 N_OP_LOAD_I16     = 0x02,
 N_OP_JUMP_UNLESS  = 0x03,
 N_OP_JUMP         = 0x04,
 N_OP_CALL         = 0x05,
 N_OP_RETURN       = 0x06,
 N_OP_GLOBAL_REF   = 0x07,
 N_OP_GLOBAL_SET   = 0x08
};

typedef enum NOpcode NOpcode;

const char*
n_get_opcode_name(NOpcode opcode);

uint8_t
n_get_opcode_size(NOpcode opcode);

#endif /* N_C_OPCODES_H */
