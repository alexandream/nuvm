#ifndef N_C_CHAR_READERS_H
#define N_C_CHAR_READERS_H

#include <stdlib.h>

#include "errors.h"
#include "compatibility/stdint.h"

typedef struct NCharReader NCharReader;

typedef struct {
    size_t line;
    size_t column;
} NCharReaderPosition;

void
ni_init_char_readers(NError* error);

NCharReader*
ni_new_char_reader_from_path(const char* path, NError* error);

NCharReader*
ni_new_char_reader_from_data(char* buffer, size_t bufer_size,
                             NError* error);

NCharReaderPosition
ni_char_reader_get_position(NCharReader* reader);

void
ni_destroy_char_reader(NCharReader* reader, NError* error);

int
ni_char_reader_is_eof(NCharReader* reader, NError* error);

void
ni_advance_char(NCharReader* reader, NError* error);

char
ni_peek_char(NCharReader* reader, NError* error);

#endif /*N_C_CHAR_READERS_H*/

