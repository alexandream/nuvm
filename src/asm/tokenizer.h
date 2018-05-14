#ifndef N_A_TOKENIZER_H
#define N_A_TOKENIZER_H

#include "../common/compatibility/stdint.h"

#include "../common/char-readers.h"

typedef struct NTokenizer NTokenizer;

enum NTokenType {
    N_TK_LBRACE = 1,
    N_TK_RBRACE,
    N_TK_INTEGER,
    N_TK_EOF,

    N_TK_XX_BEGIN_KEYWORDS,
    N_TK_KW_FIXNUM32,
    N_TK_KW_PROCEDURE,
    N_TK_XX_END_KEYWORDS,

    N_TK_XX_BEGIN_OPS,
    N_TK_OP_NOP,
    N_TK_OP_HALT,
    N_TK_OP_JUMP,
    N_TK_OP_JUMP_UNLESS,
    N_TK_OP_GLOBAL_REF,
    N_TK_OP_GLOBAL_SET,
    N_TK_OP_LOAD_I16,
    N_TK_OP_CALL,
    N_TK_XX_END_OPS,

    N_TK_XX_END_TOKENS
};

typedef enum NTokenType NTokenType;

void
ni_init_tokenizer(NError* error);

void
ni_destroy_tokenizer(NTokenizer* tokenizer, NError* error);

NTokenizer*
ni_new_tokenizer(NCharReader* reader, size_t buffer_size, NError* error);

NTokenType
ni_get_next_token(NTokenizer* tokenizer, NError* error);

const char*
ni_get_last_token_text(NTokenizer* tokenizer, NError* error);

const char*
ni_get_token_name(NTokenType type);



#endif /*N_A_TOKENIZER_H*/
