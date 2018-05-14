#include "../test.h"

#include "asm/asm.h"
#include "asm/tokenizer.h"

#include "common/char-readers.h"
static
NCharReader* READER = NULL;

static
NTokenizer* TOKENIZER = NULL;

static
char BUFFER[513];

static
NError ERR;

static void
clean_up(NError* error);

static void
with_contents(const char* data, NError* error);

static AtCheckResult
next_token_is(NTokenType expected_type, const char* expected_text);


#define WITH_CONTENTS(CHARACTERS) do {              \
    with_contents(CHARACTERS, &ERR);                \
    ASSERT(IS_OK(ERR));                             \
} while(0)

#define EXPECT_TOKEN(TYPE) ASSERT(next_token_is(TYPE, NULL))
#define EXPECT_DETAILED_TOKEN(TYPE, TEXT) ASSERT(next_token_is(TYPE, TEXT))
#define EXPECT_EOF() do {                                  \
    ni_get_next_token(TOKENIZER, &ERR);                    \
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));           \
} while(0)

CONSTRUCTOR(constructor) {
    NT_INITIALIZE_MODULE(n_init_asm);
}


SETUP(setup) {
    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    ERR = n_error_ok();
    clean_up(&ERR);
    ASSERT(IS_OK(ERR));
}

TEST(empty_contents_produces_eof) {
    WITH_CONTENTS("");
    EXPECT_EOF();
}

TEST(non_empty_contents_may_have_tokens) {
    WITH_CONTENTS("{");
    ASSERT(IS_TRUE(ni_has_more_tokens(TOKENIZER, &ERR)));
    ASSERT(IS_OK(ERR));
}

TEST(space_only_contents_produces_eof) {
	WITH_CONTENTS("  \n \t \n \n \t  ");
	EXPECT_EOF();
}


TEST(ignores_spaces) {
	WITH_CONTENTS(" \n\t { \t\n ");
	EXPECT_TOKEN(N_TK_LBRACE);
	EXPECT_EOF();
}


TEST(reads_integer_zero) {
	WITH_CONTENTS("0");
	EXPECT_DETAILED_TOKEN(N_TK_INTEGER, "0");
	EXPECT_EOF();
}


TEST(reads_integer_with_leading_zero) {
	WITH_CONTENTS("01234");
	EXPECT_DETAILED_TOKEN(N_TK_INTEGER, "01234");
	EXPECT_EOF();
}


TEST(reads_integer) {
	WITH_CONTENTS("10588");
	EXPECT_DETAILED_TOKEN(N_TK_INTEGER, "10588");
	EXPECT_EOF();
}


TEST(reads_negative_decimal_integer) {
	WITH_CONTENTS("-123456789");
	EXPECT_DETAILED_TOKEN(N_TK_INTEGER, "-123456789");
	EXPECT_EOF();
}

TEST(reads_token_lbrace) {
    WITH_CONTENTS("{");
    EXPECT_TOKEN(N_TK_LBRACE);
    EXPECT_EOF();
}


TEST(reads_token_rbrace) {
    WITH_CONTENTS("}");
    EXPECT_TOKEN(N_TK_RBRACE);
    EXPECT_EOF();
}


TEST(reads_token_kw_fixnum32) {
    WITH_CONTENTS(".fixnum32");
    EXPECT_TOKEN(N_TK_KW_FIXNUM32);
    EXPECT_EOF();
}


TEST(reads_token_kw_procedure) {
    WITH_CONTENTS(".procedure");
    EXPECT_TOKEN(N_TK_KW_PROCEDURE);
    EXPECT_EOF();
}



TEST(reads_token_op_nop) {
    WITH_CONTENTS("nop");
    EXPECT_TOKEN(N_TK_OP_NOP);
    EXPECT_EOF();
}


TEST(reads_token_op_halt) {
    WITH_CONTENTS("halt");
    EXPECT_TOKEN(N_TK_OP_HALT);
    EXPECT_EOF();
}


TEST(reads_token_op_jump) {
    WITH_CONTENTS("jump");
    EXPECT_TOKEN(N_TK_OP_JUMP);
    EXPECT_EOF();
}


TEST(reads_token_op_jump_unless) {
    WITH_CONTENTS("jump-unless");
    EXPECT_TOKEN(N_TK_OP_JUMP_UNLESS);
    EXPECT_EOF();
}


TEST(reads_token_op_global_ref) {
    WITH_CONTENTS("global-ref");
    EXPECT_TOKEN(N_TK_OP_GLOBAL_REF);
    EXPECT_EOF();
}


TEST(reads_token_op_global_set) {
    WITH_CONTENTS("global-set");
    EXPECT_TOKEN(N_TK_OP_GLOBAL_SET);
    EXPECT_EOF();
}


TEST(reads_token_op_load_i16) {
    WITH_CONTENTS("load-i16");
    EXPECT_TOKEN(N_TK_OP_LOAD_I16);
    EXPECT_EOF();
}


TEST(reads_token_op_call) {
    WITH_CONTENTS("call");
    EXPECT_TOKEN(N_TK_OP_CALL);
    EXPECT_EOF();
}


TEST(reads_sequence_of_tokens) {
	WITH_CONTENTS("  123 halt .procedure  ");
	EXPECT_DETAILED_TOKEN(N_TK_INTEGER, "123");
	EXPECT_TOKEN(N_TK_OP_HALT);
	EXPECT_TOKEN(N_TK_KW_PROCEDURE);
	EXPECT_EOF();
}



AtTest* tests[] = {
    &empty_contents_produces_eof,
    &non_empty_contents_may_have_tokens,
    &space_only_contents_produces_eof,
    &ignores_spaces,
    &reads_integer_zero,
    &reads_integer_with_leading_zero,
    &reads_integer,
    &reads_negative_decimal_integer,
    &reads_token_lbrace,
    &reads_token_rbrace,
    &reads_token_kw_fixnum32,
    &reads_token_kw_procedure,
    &reads_token_op_nop,
    &reads_token_op_halt,
    &reads_token_op_jump,
    &reads_token_op_jump_unless,
    &reads_token_op_global_ref,
    &reads_token_op_global_set,
    &reads_token_op_load_i16,
    &reads_token_op_call,
    &reads_sequence_of_tokens,
    NULL
};

TEST_RUNNER("Tokenizer", tests, constructor, NULL, setup, teardown)


#include <string.h>
static void
clean_up(NError* error) {
    NError E = n_error_ok();

    if (READER != NULL) {
        ni_destroy_char_reader(READER, &E);
        READER = NULL;
        *error = E;
    }
    if (TOKENIZER != NULL) {
        ni_destroy_tokenizer(TOKENIZER, &E);
        TOKENIZER = NULL;
        if (n_is_ok(error)) {
            *error = E;
        }
    }
}


static void
with_contents(const char* data, NError* error) {
#define EC ON_ERROR(error, return)
    clean_up(error);

    strncpy(BUFFER, data, 512);
    BUFFER[512] = '\0';

    READER = ni_new_char_reader_from_data(BUFFER, strlen(data), error);
    TOKENIZER = ni_new_tokenizer(READER, 512, error);
#undef EC
}


static AtCheckResult
next_token_is(NTokenType expected_type, const char* expected_text) {
    int texts_match;
    const char* text = NULL;

    NTokenType type = ni_get_next_token(TOKENIZER, &ERR);
    if (!n_is_ok(&ERR)) {
        const char* msg =
            at_allocf("Got an \"%s\" error while trying to get next token.",
                      ERR.type->name);
        if (msg == NULL) {
            return at_make_error("Allocation error while building failure "
                                 "message.", NULL);
        }
        return at_make_failure(msg, at_freef);
    }

    if (expected_text != NULL) {
        text = ni_get_last_token_text(TOKENIZER, &ERR);
        if (!n_is_ok(&ERR)) {
            const char* msg =
                at_allocf("Got an \"%s\" error while trying to get token text.",
                          ERR.type->name);
            if (msg == NULL) {
                return at_make_error("Allocation error while building failure "
                                     "message.", NULL);
            }
            return at_make_failure(msg, at_freef);
        }
        texts_match = strcmp(expected_text, text) == 0;
    }
    else {
        texts_match = 1;
    }

    if (expected_type != type || !texts_match) {
        const char* type_name = ni_get_token_name(type);
        const char* expected_name = ni_get_token_name(expected_type);
        const char* msg =
            at_allocf("Expected token type %s with lexeme %s. "
                      "Got token type %s with lexeme %s",
                      expected_name, expected_text, type_name, text);
        if (msg == NULL) {
            return at_make_error("Allocation error while building failure "
                                 "message.", NULL);
        }
        return at_make_failure(msg, at_freef);
    }
    return at_make_success();
}
