#include "tokenizer.h"

#define MINIMUM_BUFFER_SIZE 128

struct NTokenizer {
    char* buffer;
    size_t buffer_size;
    NCharReader* reader;
};

static
NErrorType* BAD_ALLOCATION = NULL;

void
ni_init_tokenizer(NError* error) {
#define EC ON_ERROR(error, return);
    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);         EC;
#undef EC
}

void
ni_destroy_tokenizer(NTokenizer* tokenizer, NError* error) {

}

NTokenizer*
ni_new_tokenizer(NCharReader* reader, size_t buffer_size, NError* error) {
    NTokenizer* result = NULL;
    const char* alloc_error = "Could not allocate space for tokenizer.";

    if (buffer_size < MINIMUM_BUFFER_SIZE) {
        buffer_size = MINIMUM_BUFFER_SIZE;
    }
    result = malloc(sizeof(NTokenizer));
    if (result == NULL) {
        n_set_error(error, BAD_ALLOCATION, alloc_error);
        goto clean_up;
    }

    result->buffer = malloc(sizeof(char) * buffer_size);
    if (result->buffer == NULL) {
        n_set_error(error, BAD_ALLOCATION, alloc_error);
        goto clean_up;
    }
    result->buffer[0] = '\0';
    result->reader = reader;

    return result;
clean_up:
    if (result != NULL) {
        free(result->buffer);
    }
    free(result);
    return NULL;
}

NTokenType
ni_get_next_token(NTokenizer* tokenizer, NError* error) {
    return N_TK_XX_END_TOKENS;
}

const char*
ni_get_last_token_text(NTokenizer* tokenizer, NError* error) {
    return "wakka";
}

const char*
ni_get_token_name(NTokenType type) {
    return "null";
}
