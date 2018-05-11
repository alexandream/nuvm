#include "common.h"

#include "byte-readers.h"
#include "char-readers.h"
#include "byte-writers.h"

void
n_init_common(NError* error) {
#define EC ON_ERROR(error, return)
    ni_init_errors(error);                                          EC;
    ni_init_byte_readers(error);                                    EC;
    ni_init_byte_writers(error);                                    EC;
    ni_init_char_readers(error);                                    EC;
#undef EC
}

