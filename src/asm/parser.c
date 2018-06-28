#include "parser.h"
#include "proto-values.h"
#include "proto-module.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static NProtoValue*
parse_fixnum32(NTokenizer* tokenizer, NError* error);

static NProtoValue*
parse_procedure(NTokenizer* tokenizer, NError* error);

static
NErrorType* UNEXPECTED_TOKEN = NULL;

void
ni_init_parser(NError* error) {
#define EC ON_ERROR(error, return);
    UNEXPECTED_TOKEN = n_error_type("nuvm.UnexpectedToken", error);     EC;
#undef EC

}


NProtoModule*
n_parse_module(NTokenizer* tokenizer, NError* error) {
#define EC ON_ERROR_GOTO(error, clean_up)
    NProtoModule* result;
    NToken next_token;
    NProtoValue* next_value;
    int has_more_tokens;

    result = ni_create_proto_module(error);                          EC;
    
    has_more_tokens = ni_has_more_tokens(tokenizer, error);          EC;
    while (has_more_tokens) {
        next_token = ni_get_next_token(tokenizer, error);            EC;
        next_value = NULL;

        switch (next_token.type) {
        case N_TK_KW_FIXNUM32:
            next_value = parse_fixnum32(tokenizer, error);           EC;
            ni_add_proto_value(result, next_value, error);           EC;
            break;
        case N_TK_KW_PROCEDURE:
            next_value = parse_procedure(tokenizer, error);          EC;
            ni_add_proto_value(result, next_value, error);           EC;
            break;
        default:
            /* TODO: Better error reporting on parser. */
            n_set_error(error, UNEXPECTED_TOKEN, "Unexpected token while "
                        "parsing.");
            goto clean_up;
        }
        has_more_tokens = ni_has_more_tokens(tokenizer, error);      EC;
    }

    return result;

clean_up:
    if (result == NULL) {
        ni_destroy_proto_module(result);
    }
    return NULL;
#undef EC
}


static long
parse_integer(NTokenizer *tokenizer, long min, long max, NError* error) {
#define EC ON_ERROR_RETURN(error, 0);
    const char* token_text;
    long long_value;
    NToken next_token = ni_get_next_token(tokenizer, error);             EC;
    if (next_token.type != N_TK_INTEGER) {
        /* TODO: Better error reporting on parser. */
        n_set_error(error, UNEXPECTED_TOKEN, "Unexpected token while "
                    "parsing integer.");
        return 0;
    }

    token_text = ni_get_last_token_text(tokenizer, error);               EC;
    errno = 0;
    long_value = strtol(token_text, NULL, 10);
    if (errno != 0 || long_value < min || long_value > max) {
        n_set_error(error, UNEXPECTED_TOKEN, "Unexpected token while "
                    "parsing integer.");
        return 0;
    }
    return long_value;
#undef EC
}


static NProtoValue*
parse_fixnum32(NTokenizer* tokenizer, NError* error) {
#define EC ON_ERROR_RETURN(error, NULL);
    long long_value =
        parse_integer(tokenizer, INT32_MIN, INT32_MAX, error);            EC;

    return 
        (NProtoValue*) ni_create_proto_fixnum32((int32_t) long_value, error);
#undef EC
}


static void expect_token(NTokenizer* tokenizer, NTokenType expected_type,
                         NError* error) {
#define EC ON_ERROR(error, return)
    NToken token = ni_get_next_token(tokenizer, error);                  EC;
    if (token.type != expected_type) {
        n_set_error(error, UNEXPECTED_TOKEN, "Unexpected token found.");
        return;
    }
#undef EC
}


static void
parse_and_add_instruction(NTokenizer* tokenizer, NProtoProcedure* procedure,
                          NToken initial_token, NError* error) {
#define EC ON_ERROR(error, return)
    switch (initial_token.type) {
        case N_TK_OP_NOP:
            ni_add_proto_nop(procedure, error);                          EC;
            break;
        default:
            n_set_error(error, UNEXPECTED_TOKEN, "Unexpected token found");
    }
#undef EC
}


static NProtoValue*
parse_procedure(NTokenizer* tokenizer, NError* error) {
#define EC ON_ERROR_RETURN(error, NULL);
    long long_min_locals;
    long long_max_locals;
    NProtoProcedure* result;
    NToken next_token;

    long_min_locals = parse_integer(tokenizer, 0, UINT8_MAX, error);     EC;
    long_max_locals = parse_integer(tokenizer, 0, UINT8_MAX, error);     EC;

    result = ni_create_proto_procedure((uint8_t) long_min_locals,
                                       (uint8_t) long_max_locals,
                                       error);                           EC;
    expect_token(tokenizer, N_TK_LBRACE, error);                         EC;
    
    next_token = ni_get_next_token(tokenizer, error);                    EC;
    while (next_token.type != N_TK_RBRACE) {
        parse_and_add_instruction(tokenizer, result, next_token, error); EC;
        next_token = ni_get_next_token(tokenizer, error);                EC;
    }
    return (NProtoValue*) result;
#undef EC
}
