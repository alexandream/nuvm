#include <stdlib.h>

#include "compatibility/stdint.h"
#include "opcodes.h"

const char*
n_get_opcode_name(NOpcode opcode) {
    switch(opcode) {
        case N_OP_NOP:         return "nop";
        case N_OP_HALT:        return "halt";
        case N_OP_LOAD_I16:    return "load-i16";
        case N_OP_JUMP_UNLESS: return "jump-unless";
        case N_OP_JUMP:        return "jump";
        case N_OP_CALL:        return "call";
        case N_OP_RETURN:      return "return";
        case N_OP_GLOBAL_REF:  return "global-ref";
        case N_OP_GLOBAL_SET:  return "global-set";
    }
    return NULL;
}

uint8_t
n_get_opcode_size(NOpcode opcode) {
    switch(opcode) {
        case N_OP_NOP:         return 1;
        case N_OP_HALT:        return 1;
        case N_OP_LOAD_I16:    return 4;
        case N_OP_JUMP_UNLESS: return 4;
        case N_OP_JUMP:        return 3;
        case N_OP_CALL:        return 4;
        case N_OP_RETURN:      return 2;
        case N_OP_GLOBAL_REF:  return 4;
        case N_OP_GLOBAL_SET:  return 4;
    }
    return 0;
}


