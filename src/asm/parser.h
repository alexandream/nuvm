#ifndef N_A_PARSER_H
#define N_A_PARSER_H

#include "../common/compatibility/stdint.h"

#include "proto-module.h"
#include "tokenizer.h"

void
ni_init_parser(NError* error);

NProtoModule*
n_parse_module(NTokenizer* tokenizer, NError* error);

#endif /*N_A_PARSER_H*/
