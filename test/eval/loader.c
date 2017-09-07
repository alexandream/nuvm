#include <stdlib.h>
#include <stdio.h>
#include "../test.h"

#include "common/errors.h"

#include "common/byte-readers.h"

#include "eval/loader.h"
#include "eval/procedures.h"

NError ERR;

CONSTRUCTOR(constructor) {
    if (ni_init_byte_readers() < 0) {
        ERROR("Can't initialize byte-readers module.", NULL);
    }

    if (ni_init_all_values() < 0) {
        ERROR("Can't initialize values module.", NULL);
    }

    if (ni_init_loader() < 0) {
        ERROR("Can't initialize loader module.", NULL);
    }
}


SETUP(setup) {
    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
}


TEST(load_fixnum32_needs_4_bytes) {
    uint8_t data[3];
    NByteReader* reader = n_new_byte_reader_from_data(data, 3, &ERR);
    ASSERT(IS_OK(ERR));

    nt_read_fixnum32_global(reader, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(load_fixnum32_loads_min) {
    NValue fixnum;
    uint8_t data[] = { 0X00, 0x00, 0x00, 0x80 };
    NByteReader* reader = n_new_byte_reader_from_data(data, 4, &ERR);
    ASSERT(IS_OK(ERR));

    fixnum = nt_read_fixnum32_global(reader, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(n_unwrap_fixnum(fixnum), -2147483648));
}


TEST(load_fixnum32_loads_max) {
    NValue fixnum;
    uint8_t data[] = { 0xFF, 0xFF, 0xFF, 0x7F };
    NByteReader* reader = n_new_byte_reader_from_data(data, 4, &ERR);
    ASSERT(IS_OK(ERR));

    fixnum = nt_read_fixnum32_global(reader, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(n_unwrap_fixnum(fixnum), 2147483647));
}


TEST(load_fixnum32_loads_zero) {
    NValue fixnum;
    uint8_t data[] = { 0X00, 0x00, 0x00, 0x00 };
    NByteReader* reader = n_new_byte_reader_from_data(data, 4, &ERR);
    ASSERT(IS_OK(ERR));

    fixnum = nt_read_fixnum32_global(reader, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(n_unwrap_fixnum(fixnum), 0));
}


TEST(load_procedure_needs_8_bytes) {
    NValue proc;
    uint8_t data[7];
    NByteReader* reader = n_new_byte_reader_from_data(data, 7, &ERR);
    ASSERT(IS_OK(ERR));

    proc = nt_read_procedure_global(reader, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(load_procedure_loads_correctly) {
    NValue proc;
    NProcedure* proc_ptr;
    uint8_t data[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

    NByteReader* reader = n_new_byte_reader_from_data(data, 8, &ERR);
    ASSERT(IS_OK(ERR));

    proc = nt_read_procedure_global(reader, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_is_procedure(proc)));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);

    ASSERT(EQ_INT(proc_ptr->entry, 0x03020100));
    ASSERT(EQ_INT(proc_ptr->num_locals, 0x04));
    ASSERT(EQ_INT(proc_ptr->max_locals, 0x05));
    ASSERT(EQ_INT(proc_ptr->size, 0x0706));
}


TEST(load_global_rejects_unknown_id) {
    uint8_t data[] = { 0x02 };

    NByteReader* reader = n_new_byte_reader_from_data(data, 1, &ERR);
    ASSERT(IS_OK(ERR));

    nt_read_global(reader, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.InvalidModuleFormat"));
}


TEST(load_global_detects_procedure) {
    NValue proc;
    NProcedure* proc_ptr;
    uint8_t data[] = { 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };

    NByteReader* reader = n_new_byte_reader_from_data(data, 9, &ERR);
    ASSERT(IS_OK(ERR));

    proc = nt_read_global(reader, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(IS_TRUE(n_is_procedure(proc)));

    proc_ptr = (NProcedure*) n_unwrap_object(proc);

    ASSERT(EQ_INT(proc_ptr->entry, 0x03020100));
    ASSERT(EQ_INT(proc_ptr->num_locals, 0x04));
    ASSERT(EQ_INT(proc_ptr->max_locals, 0x05));
    ASSERT(EQ_INT(proc_ptr->size, 0x0706));
}


TEST(load_global_detects_fixnum32) {
    NValue fixnum;
    uint8_t data[] = { 0x00, 0xFF, 0xFF, 0xFF, 0x7F };
    NByteReader* reader = n_new_byte_reader_from_data(data, 5, &ERR);
    ASSERT(IS_OK(ERR));

    fixnum = nt_read_global(reader, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(n_unwrap_fixnum(fixnum), 2147483647));
}



AtTest* tests[] = {
    &load_fixnum32_needs_4_bytes,
    &load_fixnum32_loads_min,
    &load_fixnum32_loads_max,
    &load_fixnum32_loads_zero,
    &load_procedure_needs_8_bytes,
    &load_procedure_loads_correctly,
    &load_global_rejects_unknown_id,
    &load_global_detects_procedure,
    &load_global_detects_fixnum32,
    NULL
};



TEST_RUNNER("Loader", tests, constructor, NULL, setup, teardown)
