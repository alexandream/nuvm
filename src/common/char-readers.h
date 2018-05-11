#ifndef N_C_CHAR_READERS_H
#define N_C_CHAR_READERS_H

#include <stdlib.h>

#include "errors.h"
#include "compatibility/stdint.h"

typedef struct NCharReader NCharReader;

int
ni_init_char_readers(void);

NCharReader*
ni_new_char_reader_from_path(const char* path, NError* error);

void
ni_destroy_char_reader(NCharReader* reader, NError* error);

int
ni_char_reader_is_eof(NCharReader* reader, NError* error);

void
ni_advance_char(NCharReader* reader, NError* error);

char
ni_peek_char(NCharReader* reader, NError* error);

#endif /*N_C_CHAR_READERS_H*/

