#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../test.h"

#include "common/char-readers.h"

#define LONG_READER_ITERATIONS 50

const char* BASE_PATTERN = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
size_t BASE_LENGTH;

NCharReader* EMPTY_READER;
NCharReader* SHORT_READER;
NCharReader*  LONG_READER;

static
NError ERR;


CONSTRUCTOR(constructor) {
	FILE* file;
	int i;

    if (ni_init_char_readers() < 0) {
        ERROR("Could not initialize char readers module for tests.", NULL);
    }

	file = fopen("build/empty-file", "w");
	if (file == NULL) {
        ERROR("Could not create empty file char reader for test.", NULL);
    }
	fclose(file);

	file = fopen("build/short-file", "w");
	if (file == NULL) {
        ERROR("Could not create short file char reader for test.", NULL);
    }
	if (fputs(BASE_PATTERN, file) < 0) {
        ERROR("Could not create short file char reader for test.", NULL);
	}
	fclose(file);

	file = fopen("build/long-file", "w");
	if (file == NULL) {
        ERROR("Could not create long file char reader for test.", NULL);
    }
	for (i = 0; i < LONG_READER_ITERATIONS; i++) {
		if (fputs(BASE_PATTERN, file) < 0) {
            ERROR("Could not create long file char reader for test.", NULL);
		}
	}
	fclose(file);
	BASE_LENGTH = strlen(BASE_PATTERN);
}


SETUP(setup) {
    ERR = n_error_ok();

	EMPTY_READER = ni_new_char_reader_from_path("build/empty-file", &ERR);
    ASSERT(IS_OK(ERR));
	SHORT_READER = ni_new_char_reader_from_path("build/short-file", &ERR);
    ASSERT(IS_OK(ERR));
	LONG_READER  = ni_new_char_reader_from_path("build/long-file", &ERR);
    ASSERT(IS_OK(ERR));
}

TEARDOWN(teardown) {
    ERR = n_error_ok();

	ni_destroy_char_reader(EMPTY_READER, &ERR);
    ASSERT(IS_OK(ERR));
	ni_destroy_char_reader(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));
	ni_destroy_char_reader(LONG_READER, &ERR);
    ASSERT(IS_OK(ERR));
}


TEST(empty_reader_starts_at_eof) {
	ASSERT(IS_TRUE(ni_char_reader_is_eof(EMPTY_READER, &ERR)));
    ASSERT(IS_OK(ERR));
}


TEST(short_reader_has_data) {
	ASSERT(IS_TRUE(ni_char_reader_is_eof(SHORT_READER, &ERR) == 0));
    ASSERT(IS_OK(ERR));
}


TEST(short_reader_has_correct_length) {
	int i = 0;
	while(!ni_char_reader_is_eof(SHORT_READER, &ERR)) {
        ASSERT(IS_OK(ERR));
		i++;
		ni_advance_char(SHORT_READER, &ERR);
        ASSERT(IS_OK(ERR));

	}
	ASSERT(EQ_INT(i, BASE_LENGTH));
}


TEST(long_reader_has_data) {
	ASSERT(IS_TRUE(ni_char_reader_is_eof(LONG_READER, &ERR) == 0));
    ASSERT(IS_OK(ERR));
}


TEST(long_reader_has_correct_length) {
	int i = 0;
	while(!ni_char_reader_is_eof(LONG_READER, &ERR)) {
        ASSERT(IS_OK(ERR));
		i++;
		ni_advance_char(LONG_READER, &ERR);
        ASSERT(IS_OK(ERR));

	}
	ASSERT(EQ_INT(i, BASE_LENGTH * LONG_READER_ITERATIONS));
}


TEST(peek_on_empty_signals_eof) {
	ni_peek_char(EMPTY_READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(advance_on_empty_signals_eof) {
	ni_advance_char(EMPTY_READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(first_peek_gives_first_char) {
	char entry = ni_peek_char(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));
	ASSERT(IS_TRUE(entry == 'A'));
}


TEST(peek_after_advance_gives_second_char) {
    char entry;

	ni_advance_char(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));

    entry = ni_peek_char(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));
	ASSERT(IS_TRUE(entry == 'B'));
}


TEST(repeated_peek_gives_first_char) {
	char first_entry = ni_peek_char(SHORT_READER, &ERR);
    char entry;

    ASSERT(IS_OK(ERR));

	entry = ni_peek_char(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));

	ASSERT(IS_TRUE(!ni_char_reader_is_eof(SHORT_READER, &ERR)));
    ASSERT(IS_OK(ERR));
	ASSERT(IS_TRUE(entry == first_entry));
	ASSERT(IS_TRUE(entry == 'A'));
}


TEST(peeks_with_advances_give_different_chars) {
	char first_entry = ni_peek_char(SHORT_READER, &ERR);
    char entry;

    ASSERT(IS_OK(ERR));

    ni_advance_char(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));
	
    entry = ni_peek_char(SHORT_READER, &ERR);
    ASSERT(IS_OK(ERR));

	ASSERT(IS_TRUE(entry != first_entry));
	ASSERT(IS_TRUE(entry == 'B'));
}


AtTest* tests[] = {
    &empty_reader_starts_at_eof,
    &short_reader_has_data,
    &short_reader_has_correct_length,
    &long_reader_has_data,
    &long_reader_has_correct_length,
    &peek_on_empty_signals_eof,
    &advance_on_empty_signals_eof,
    &first_peek_gives_first_char,
    &peek_after_advance_gives_second_char,
    &repeated_peek_gives_first_char,
    &peeks_with_advances_give_different_chars,
    NULL
};


TEST_RUNNER("CharReaders", tests, constructor, NULL, setup, teardown)
