#include "../test.h"

#include "asm/asm.h"
#include "asm/tokenizer.h"
#include "asm/parser.h"
#include "asm/proto-module.h"
#include "asm/proto-values.h"

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


#define WITH_CONTENTS(CHARACTERS) do {              \
    with_contents(CHARACTERS, &ERR);                \
    ASSERT(IS_OK(ERR));                             \
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


TEST(parses_fixnum32_zero) {
    NProtoModule* module;
    NProtoValue* fixnum;

    WITH_CONTENTS(".fixnum32 0");

    module = n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(ni_proto_value_count(module), 1));
    
    fixnum = ni_get_proto_value(module, 0, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(nt_matches_proto_fixnum32(fixnum, 0)));
}


TEST(parses_lowest_fixnum32) {
    NProtoModule* module;
    NProtoValue* fixnum;

    WITH_CONTENTS(".fixnum32 -2147483648");

    module = n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(ni_proto_value_count(module), 1));
    
    fixnum = ni_get_proto_value(module, 0, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(nt_matches_proto_fixnum32(fixnum, -2147483648)));
}


TEST(rejects_underflow_fixnum32) {
    WITH_CONTENTS(".fixnum32 -2147483649");

    n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedToken"));
}


TEST(parses_highest_fixnum32) {
    NProtoModule* module;
    NProtoValue* fixnum;

    WITH_CONTENTS(".fixnum32 2147483647");

    module = n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(ni_proto_value_count(module), 1));
    
    fixnum = ni_get_proto_value(module, 0, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(nt_matches_proto_fixnum32(fixnum, 2147483647)));
}


TEST(rejects_overflow_fixnum32) {
    WITH_CONTENTS(".fixnum32 2147483648");

    n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedToken"));
}


TEST(rejects_incomplete_fixnum32) {
    WITH_CONTENTS(".fixnum32");

    n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(parses_nop_procedure) {
    NProtoModule* module;
    NProtoValue* procedure;

    WITH_CONTENTS(".procedure 1 2 { "
                  "    nop          "
                  "}                ");

    module = n_parse_module(TOKENIZER, &ERR);
    ASSERT(IS_OK(ERR));

    procedure = ni_get_proto_value(module, 0, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(nt_matches_proto_procedure(procedure, 1, 2, 1)));
}


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
        else {
            n_destroy_error(&E);
        }
    }
}


#include <string.h>

static void
with_contents(const char* data, NError* error) {
#define EC ON_ERROR(error, return)
    clean_up(error);

    strncpy(BUFFER, data, 512);
    BUFFER[512] = '\0';

    READER = ni_new_char_reader_from_data(BUFFER, strlen(data), error);  EC;
    TOKENIZER = ni_new_tokenizer(READER, 512, error);                    EC;
#undef EC
}


AtTest* tests[] = {
    &parses_fixnum32_zero,
    &parses_lowest_fixnum32,
    &rejects_underflow_fixnum32,
    &parses_highest_fixnum32,
    &rejects_overflow_fixnum32,
    &rejects_incomplete_fixnum32,
    &parses_nop_procedure,
    NULL
};

TEST_RUNNER("Parser", tests, constructor, NULL, setup, teardown)

