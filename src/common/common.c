#include "common.h"

#include "byte-readers.h"
#include "char-readers.h"
#include "byte-writers.h"

void
n_init_common(NError* error) {
#define ECC ON_ERROR(error, return)
    ni_init_errors(error);                                          ECC;
    ni_init_byte_readers(error);                                    ECC;
    ni_init_byte_writers(error);                                    ECC;
    ni_init_char_readers(error);                                    ECC;
#undef ECC
}

