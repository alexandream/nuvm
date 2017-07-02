#include <stdlib.h>
#include <string.h>

#include "../test.h"

#include "common/byte-readers.h"

static
NByteReader *READER = NULL;

static
char TEST_DATA[256];

static
NError ERR;


CONSTRUCTOR(constructor) {
    if (ni_init_byte_readers() < 0) {
        ERROR("Cant initialize byte readers module.", NULL);
    }

    {
        /* Initialize TEST_DATA with the array:
         * [ 0x01, 0x02, 0x03, ..., 0xFE, 0xFF, 0x00 ] */
        int i;
        for (i = 0; i < 256; i++) {
            TEST_DATA[i] = (i+1) % 255;
        }
    }

}


SETUP(setup) {
    NError error = n_error_ok();

    READER = n_new_byte_reader_from_data(TEST_DATA, 256, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't setup byte reader for test.", NULL);
    }

    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    NError error = n_error_ok();
    n_destroy_byte_reader(READER, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't clean up byte reader after test.", NULL);
    }
}


TEST(read_byte_gets_right_value) {
    uint8_t output = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(output, 1));
}


TEST(read_uint16_get_right_value) {
    uint16_t output = n_read_uint16(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(output,  513 /*0x01, 0x02*/));
}


TEST(read_int16_get_right_value) {
    int16_t output = n_read_int16(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(output, 513 /*0x01, 0x02*/));
}


TEST(read_int32_get_right_value) {
    int32_t output = n_read_int32(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(output, 67305985 /*0x01, 0x02, 0x03, 0x04*/));
}


TEST(read_uint32_get_right_value) {
    uint32_t output = n_read_uint32(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(output, 67305985 /*0x01, 0x02, 0x03, 0x04*/));
}


TEST(read_bytes_reads_full_buffer) {
    char buffer[256];
    int bytes_read = n_read_bytes(READER, buffer, 256, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(bytes_read, 256));
    ASSERT(IS_TRUE(memcmp(buffer, TEST_DATA, 256) == 0));
}


TEST(read_bytes_reads_partial_buffer) {
    char buffer[512];
    int bytes_read = n_read_bytes(READER, buffer, 512, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(bytes_read, 256));
    ASSERT(IS_TRUE(memcmp(buffer, TEST_DATA, 256) == 0));
}


TEST(skip_bytes_moves_reader_ahead) {
    uint8_t next_byte;

    int skipped_bytes = n_skip_bytes(READER, 32, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(skipped_bytes, 32));

    next_byte = n_read_byte(READER, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(next_byte, 33));
}


TEST(skip_bytes_stops_on_reader_end) {
    int skipped_bytes = n_skip_bytes(READER, 300,  &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_INT(skipped_bytes, 256));
}


TEST(read_byte_checks_bounds) {
    n_skip_bytes(READER, 256, &ERR);
    ASSERT(IS_OK(ERR));

    n_read_byte(READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(read_uint16_checks_bounds) {
    n_skip_bytes(READER, 255, &ERR);
    ASSERT(IS_OK(ERR));

    n_read_uint16(READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(read_int16_checks_bounds) {
    n_skip_bytes(READER, 255, &ERR);
    ASSERT(IS_OK(ERR));

    n_read_int16(READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(read_int32_checks_bounds) {
    n_skip_bytes(READER, 253, &ERR);
    ASSERT(IS_OK(ERR));

    n_read_int32(READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(read_uint32_checks_bounds) {
    n_skip_bytes(READER, 253, &ERR);
    ASSERT(IS_OK(ERR));

    n_read_uint32(READER, &ERR);
    ASSERT(IS_ERROR(ERR, "nuvm.UnexpectedEoF"));
}


TEST(has_data_is_true_when_not_at_end) {
    ASSERT(IS_TRUE(n_has_bytes_to_read(READER)));
}


TEST(has_data_is_false_when_at_end) {
    n_skip_bytes(READER, 256, &ERR);
    ASSERT(IS_OK(ERR));

    ASSERT(IS_TRUE(!n_has_bytes_to_read(READER)));
}


AtTest* tests[] = {
    &read_byte_gets_right_value,
    &read_uint16_get_right_value,
    &read_int16_get_right_value,
    &read_int32_get_right_value,
    &read_uint32_get_right_value,
    &read_bytes_reads_full_buffer,
    &read_bytes_reads_partial_buffer,
    &skip_bytes_moves_reader_ahead,
    &skip_bytes_stops_on_reader_end,
    &read_byte_checks_bounds,
    &read_uint16_checks_bounds,
    &read_int16_checks_bounds,
    &read_int32_checks_bounds,
    &read_uint32_checks_bounds,
    &has_data_is_true_when_not_at_end,
    &has_data_is_false_when_at_end,
    NULL
};


TEST_RUNNER("ByteReaders", tests, constructor, NULL, setup, teardown)
