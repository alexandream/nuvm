#include "common.h"

#include "byte-readers.h"
#include "char-readers.h"
#include "byte-writers.h"

void
n_init_common(NError* error) {
    ni_init_errors();
    ni_init_byte_readers();
    ni_init_byte_writers();
    ni_init_char_readers();
}

