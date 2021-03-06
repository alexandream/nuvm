#include "../common/common.h"
#include "../common/errors.h"
#include "../common/byte-readers.h"

#include "loader.h"
#include "procedures.h"

static
NErrorType* INVALID_MODULE_FORMAT = NULL;

static
NErrorType* UNEXPECTED_EOF = NULL;

static NValue
read_global(NByteReader* reader, NModule* module, NError* error);

void
ni_init_loader(NError* error) {
#define EC ON_ERROR(error, return)
    INVALID_MODULE_FORMAT =
        n_error_type("nuvm.InvalidModuleFormat", error);           EC;

    UNEXPECTED_EOF = n_error_type("nuvm.UnexpectedEoF", error);    EC;
#undef EC
}


NModule*
n_read_module(NByteReader* reader, NError* error) {
#define EC ON_ERROR_GOTO(error, clean_up)
    uint16_t num_globals;
    uint32_t code_size;
    uint16_t i;
    int bytes_read;
    NModule* module = NULL;

    num_globals = n_read_uint16(reader, error);                EC;
    code_size = n_read_uint32(reader, error);                  EC;

    module = n_create_module(num_globals, code_size, error);   EC;

    for (i = 0; i < num_globals; i++) {
        NValue global = read_global(reader, module, error);    EC;
        module->globals[i] = global;
    }

    bytes_read =
        n_read_bytes(reader, module->code, code_size, error);  EC;

    if ((uint32_t) bytes_read != code_size) {
        n_set_error(error, UNEXPECTED_EOF, "Not enough bytes in the stream "
                    "to load the code for the module.");
        goto clean_up;
    }

    return module;
clean_up:
    if (module != NULL) {
        n_destroy_module(module);
    }
    return NULL;
#undef EC
}




static NValue
read_fixnum32_global(NByteReader* reader, NError* error) {
    return n_wrap_fixnum(n_read_int32(reader, error));
}


static NValue
read_procedure_global(NByteReader* reader, NModule *module, NError* error) {
#define EC ON_ERROR_RETURN(error, 0);
    uint32_t entry;
    uint8_t min_locals, max_locals;
    uint16_t size;

    entry = n_read_uint32(reader, error);               EC;
    min_locals = n_read_byte(reader, error);            EC;
    max_locals = n_read_byte(reader, error);            EC;
    size = n_read_uint16(reader, error);                EC;

    return n_create_procedure(module, entry, min_locals, max_locals,
                              size, error);
#undef EC
}


static NValue
read_global(NByteReader* reader, NModule* module, NError* error) {
#define EC ON_ERROR_RETURN(error, 0);
    uint8_t type = n_read_byte(reader, error);                       EC;
    switch (type) {
        case 0x00:
            return read_fixnum32_global(reader, error);
        case 0x01:
            return read_procedure_global(reader, module, error);
        default:
            n_set_error(error, INVALID_MODULE_FORMAT,
                    "Unrecognized global descriptor id.");
    }
    return 0;
#undef EC
}




#ifdef N_TEST

NValue
nt_read_fixnum32_global(NByteReader* reader, NError* error) {
    return read_fixnum32_global(reader, error);
}


NValue
nt_read_procedure_global(NByteReader* reader, NModule* module, NError* error) {
    return read_procedure_global(reader, module, error);
}


NValue
nt_read_global(NByteReader* reader, NModule* module, NError* error) {
    return read_global(reader, module, error);
}

#endif /*N_TEST*/
