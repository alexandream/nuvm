#include "../common/errors.h"
#include "../common/byte-readers.h"

#include "loader.h"
#include "procedures.h"

#ifdef N_TEST

NValue
nt_read_fixnum32_global(NByteReader* reader, NError* error) {
    return n_read_int32(reader, error);
}


NValue
nt_read_procedure_global(NByteReader* reader, NError* error) {
#define ROR if (!n_is_ok(error)) return 0
    uint32_t entry;
    uint8_t min_locals, max_locals;
    uint16_t size;

    entry = n_read_uint32(reader, error);               ROR;
    min_locals = n_read_byte(reader, error);           ROR;
    max_locals = n_read_byte(reader, error);           ROR;
    size = n_read_uint16(reader, error);                ROR;

    return n_create_procedure(entry, min_locals, max_locals, size, error);
#undef ROR
}

#endif /*N_TEST*/
