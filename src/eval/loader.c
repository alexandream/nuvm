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




int
ni_init_loader(void) {
    static int INITIALIZED = 0;
    NError error = n_error_ok();
    if (!INITIALIZED) {
        n_init_common(&error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -1;
        }


        if (ni_init_all_values() < 0) {
            return -2;
        }
        INVALID_MODULE_FORMAT =
            n_error_type("nuvm.InvalidModuleFormat", &error);

        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -3;
        }

        UNEXPECTED_EOF = n_error_type("nuvm.UnexpectedEoF", &error);
        if (!n_is_ok(&error)) {
            n_destroy_error(&error);
            return -4;
        }
    }
    return 0;
}


NModule*
n_read_module(NByteReader* reader, NError* error) {
#define CHECK_ERROR ON_ERROR_GOTO(error, clean_up)
    uint16_t num_globals;
    uint32_t code_size;
    uint16_t i;
    int bytes_read;
    NModule* module = NULL;

    num_globals = n_read_uint16(reader, error);                CHECK_ERROR;
    code_size = n_read_uint32(reader, error);                  CHECK_ERROR;

    module = n_create_module(num_globals, code_size, error);   CHECK_ERROR;

    for (i = 0; i < num_globals; i++) {
        NValue global = read_global(reader, module, error);    CHECK_ERROR;
        module->globals[i] = global;
    }

    bytes_read =
        n_read_bytes(reader, module->code, code_size, error);  CHECK_ERROR;

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
#undef CHECK_ERROR
}




static NValue
read_fixnum32_global(NByteReader* reader, NError* error) {
    return n_wrap_fixnum(n_read_int32(reader, error));
}


static NValue
read_procedure_global(NByteReader* reader, NModule *module, NError* error) {
#define CHECK_ERROR ON_ERROR_RETURN(error, 0);
    uint32_t entry;
    uint8_t min_locals, max_locals;
    uint16_t size;

    entry = n_read_uint32(reader, error);               CHECK_ERROR;
    min_locals = n_read_byte(reader, error);            CHECK_ERROR;
    max_locals = n_read_byte(reader, error);            CHECK_ERROR;
    size = n_read_uint16(reader, error);                CHECK_ERROR;

    return n_create_procedure(module, entry, min_locals, max_locals,
                              size, error);
#undef CHECK_ERROR
}


static NValue
read_global(NByteReader* reader, NModule* module, NError* error) {
#define CHECK_ERROR ON_ERROR_RETURN(error, 0);
    uint8_t type = n_read_byte(reader, error);          CHECK_ERROR;
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
#undef CHECK_ERROR
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
