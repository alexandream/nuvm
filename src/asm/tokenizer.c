#include <ctype.h>
#include <string.h>

#include "tokenizer.h"

#define MINIMUM_BUFFER_SIZE 128

struct NTokenizer {
    char* buffer;
    size_t buffer_size;
    NCharReader* reader;
};

typedef struct {
    const char* text;
    NTokenType  token_type;
} NTokenMapping;


static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* OVERFLOW = NULL;


static char
peek_over_eof(NCharReader* reader, NError* error);

static void
discard_spaces(NTokenizer* self, NError* error);

static NTokenType
read_single_character_token(NTokenizer* self, NTokenType token,
                            NError* error);
static NTokenType
read_instruction_or_label(NTokenizer* self, NError* error);

static NTokenType
read_keyword(NTokenizer* self, NError* error);

static NTokenType
read_label_ref(NTokenizer* self, NError* error);

static NTokenType
read_integer(NTokenizer* self, NError* error);

void
ni_init_tokenizer(NError* error) {
#define EC ON_ERROR(error, return);
    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", error);         EC;
    OVERFLOW = n_error_type("nuvm.Overflow", error);                    EC;
#undef EC
}

void
ni_destroy_tokenizer(NTokenizer* self, NError* error) {

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

    result->buffer = malloc(sizeof(char) * (buffer_size + 1));
    if (result->buffer == NULL) {
        n_set_error(error, BAD_ALLOCATION, alloc_error);
        goto clean_up;
    }
    result->buffer[0] = '\0';
    result->buffer_size = buffer_size;
    result->reader = reader;

    return result;
clean_up:
    if (result != NULL) {
        free(result->buffer);
    }
    free(result);
    return NULL;
}

NToken
ni_get_next_token(NTokenizer* self, NError* error) {
#define EC ON_ERROR_RETURN(error, result)
    char current_char;
    NToken result;
    NCharReaderPosition position;

    self->buffer[0] = '\0';

    discard_spaces(self, error);                                          EC;
    position = ni_char_reader_get_position(self->reader);
    result.type = 0;
    result.line = position.line;
    result.column = position.column;

    current_char = ni_peek_char(self->reader, error);                     EC;
    if (current_char == '{') {
        result.type = read_single_character_token(self, N_TK_LBRACE, error);
        return result;
    }
    else if (current_char == '}') {
        result.type = read_single_character_token(self, N_TK_RBRACE, error);
        return result;
    }
    else if (current_char == '.') {
        result.type = read_keyword(self, error);
        return result;
    }
    else if (current_char == '@') {
        result.type = read_label_ref(self, error);
        return result;
    }
    else if (isalpha(current_char)) {
        result.type = read_instruction_or_label(self, error);
        return result;
    }
    else if (isdigit(current_char) || current_char == '-' ||
             current_char == '+') {
        result.type = read_integer(self, error);
        return result;
    }
    return result;
#undef EC
}

const char*
ni_get_last_token_text(NTokenizer* self, NError* error) {
    return self->buffer;
}


int
ni_has_more_tokens(NTokenizer* self, NError* error) {
    return !ni_char_reader_is_eof(self->reader, error);
}


const char*
ni_get_token_name(NTokenType type) {
    switch (type) {
        case N_TK_LBRACE: return "TK_LBRACE";
        case N_TK_RBRACE: return "TK_RBRACE";
        case N_TK_INTEGER: return "TK_INTEGER";
        case N_TK_LABEL_DEF: return "TK_LABEL_DEF";
        case N_TK_LABEL_REF: return "TK_LABEL_REF";
        case N_TK_EOF: return "TK_EOF";
        case N_TK_KW_FIXNUM32: return "TK_KW_FIXNUM32";
        case N_TK_KW_PROCEDURE: return "TK_KW_PROCEDURE";
        case N_TK_OP_NOP: return "TK_OP_NOP";
        case N_TK_OP_HALT: return "TK_OP_HALT";
        case N_TK_OP_JUMP: return "TK_OP_JUMP";
        case N_TK_OP_JUMP_UNLESS: return "TK_OP_JUMP_UNLESS";
        case N_TK_OP_GLOBAL_REF: return "TK_OP_GLOBAL_REF";
        case N_TK_OP_GLOBAL_SET: return "TK_OP_GLOBAL_SET";
        case N_TK_OP_LOAD_I16: return "TK_OP_LOAD_I16";
        case N_TK_OP_CALL: return "TK_OP_CALL";
        default: return "null";
    }
}



static char
peek_over_eof(NCharReader* reader, NError* error) {
    int is_eof = ni_char_reader_is_eof(reader, error);
    return is_eof ? '\0' : ni_peek_char(reader, error);
}


static void
discard_spaces(NTokenizer* self, NError* error) {
#define EC ON_ERROR(error, return)
    char current_char = peek_over_eof(self->reader, error);          EC;
    while (current_char != '\0' && isspace(current_char)) {
        ni_advance_char(self->reader, error);                        EC;
        current_char = peek_over_eof(self->reader, error);           EC;
    }
#undef EC
}


static NTokenType
read_single_character_token(NTokenizer* self, NTokenType token,
                            NError* error) {
#define EC ON_ERROR_RETURN(error, 0)
    char next_char;
    ni_advance_char(self->reader, error);                            EC;
    next_char = peek_over_eof(self->reader, error);                  EC;
    if (next_char == '\0' || isspace(next_char)) {
        return token;
    }
    return 0;
#undef EC
}


static int
is_identifier(char input) {
    return isalnum(input) || input == '-';
}

#include <stdio.h>

static int
copy_next_word_to_buffer(NTokenizer* self, size_t start,
                         int (*is_acceptable)(char), NError* error) {
#define EC ON_ERROR_RETURN(error, 0)
    size_t i = start;
    char current_char = peek_over_eof(self->reader, error);         EC;
    while (current_char != '\0' && !isspace(current_char) &&
           is_acceptable(current_char)) {
        if (i >= self->buffer_size) {
            n_set_error(error, OVERFLOW, "Next token is bigger than the "
                        "internal buffer.");
            self->buffer[self->buffer_size] = '\0';
            fprintf(stderr, "Overflow on: %s.\n", self->buffer);
            return 0;
        }
        self->buffer[i++] = current_char;
        ni_advance_char(self->reader, error);                        EC;
        current_char = peek_over_eof(self->reader, error);           EC;
    }
    self->buffer[i] = '\0';
    return current_char == '\0' || isspace(current_char);
#undef EC
}


static
NTokenMapping INSTRUCTIONS[] = {
    { "nop", N_TK_OP_NOP },
    { "halt", N_TK_OP_HALT },
    { "jump", N_TK_OP_JUMP },
    { "jump-unless", N_TK_OP_JUMP_UNLESS },
    { "global-ref", N_TK_OP_GLOBAL_REF },
    { "global-set", N_TK_OP_GLOBAL_SET },
    { "load-i16", N_TK_OP_LOAD_I16 },
    { "call", N_TK_OP_CALL },
    { NULL, 0 }
};


static NTokenType
read_instruction_or_label(NTokenizer* self, NError* error) {
#define EC ON_ERROR_RETURN(error, 0)
    char last_peek;
    copy_next_word_to_buffer(self, 0, is_identifier, error);       EC;
    last_peek = peek_over_eof(self->reader, error);                EC;

    if (last_peek == '\0' || isspace(last_peek)) {
        NTokenMapping* m = INSTRUCTIONS;
        while (m->text != NULL) {
            if (strcmp(self->buffer, m->text) == 0) {
                return m->token_type;
            }
            m++;
        }
    }
    else if (last_peek == ':') {
        char next_char;
        ni_advance_char(self->reader, error);                        EC;
        next_char = peek_over_eof(self->reader, error);              EC;
        if (next_char == '\0' || isspace(next_char)) {
            return N_TK_LABEL_DEF;
        }
    }
    return 0;
#undef EC
}


static
NTokenMapping KEYWORDS[] = {
    { "procedure", N_TK_KW_PROCEDURE },
    { "fixnum32",  N_TK_KW_FIXNUM32 },
    { NULL, 0 }
};
static NTokenType
read_keyword(NTokenizer* self, NError* error) {
#define EC ON_ERROR_RETURN(error, 0)
    int fully_acceptable;

    ni_advance_char(self->reader, error);                            EC;

    fully_acceptable =
        copy_next_word_to_buffer(self, 0, is_identifier, error);     EC;

    if (fully_acceptable) {
        NTokenMapping* m = KEYWORDS;
        while (m->text != NULL) {
            if (strcmp(self->buffer, m->text) == 0) {
                return m->token_type;
            }
            m++;
        }
    }
    return 0;
#undef EC
}


static NTokenType
read_label_ref(NTokenizer* self, NError* error) {
#define EC ON_ERROR_RETURN(error, 0)
    int fully_acceptable;

    ni_advance_char(self->reader, error);                            EC;

    fully_acceptable =
        copy_next_word_to_buffer(self, 0, is_identifier, error);     EC;

    if (fully_acceptable) {
        return N_TK_LABEL_REF;
    }
    return 0;
#undef EC
}


static int
is_digit(char input) {
    return isdigit(input);
}

static NTokenType
read_integer(NTokenizer* self, NError* error) {
#define EC ON_ERROR_RETURN(error, 0)
    char current_char;
    size_t start = 0;
    int fully_acceptable;

    current_char = ni_peek_char(self->reader, error);                    EC;
    if (current_char == '+' || current_char == '-') {
        ni_advance_char(self->reader, error);                            EC;
        self->buffer[0] = current_char;
        start = 1;
    }

    fully_acceptable =
        copy_next_word_to_buffer(self, start, is_digit, error);    EC;

    if (fully_acceptable) {
        return N_TK_INTEGER;
    }
    return 0;
#undef EC
}

