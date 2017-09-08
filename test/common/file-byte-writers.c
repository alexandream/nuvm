#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../test.h"

#include "common/byte-writers.h"

static
const char* OUTPUT = "./build/file-byte-writers.data";

static
NByteWriter *WRITER = NULL;

static
int WRITER_IS_DESTROYED = 0;

static
NError ERR;

static void
read_from_file(const char* file_name, void* dest, size_t start, size_t size);

static void
destroy_writer(void);

CONSTRUCTOR(constructor) {
    if (ni_init_byte_writers() < 0) {
        ERROR("Cant initialize byte writers module.", NULL);
    }
}


SETUP(setup) {
    ERR = n_error_ok();

    WRITER = n_create_file_byte_writer(OUTPUT, &ERR);
    if (!n_is_ok(&ERR)) {
        n_destroy_error(&ERR);
        ERROR("Can't setup byte writer for test.", NULL);
    }
    WRITER_IS_DESTROYED = 0;

    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    n_destroy_error(&ERR);
    if (!WRITER_IS_DESTROYED) {
        n_destroy_byte_writer(WRITER, &ERR);
        if (!n_is_ok(&ERR)) {
            ERROR("Can't destroy the byte writer after tests.", NULL);
        }
    }
}


TEST(write_byte_puts_maximum) {
    uint8_t value;
    n_write_byte(WRITER, 0xFF, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(uint8_t));
    
    ASSERT(EQ_UINT(value, 0xFF));
}


TEST(write_byte_puts_minimum) {
    uint8_t value;
    n_write_byte(WRITER, 0x00, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(uint8_t));
    
    ASSERT(EQ_UINT(value, 0x00));
}


TEST(write_byte_uses_1_byte) {
    uint8_t value; 
    n_write_byte(WRITER, 0x00, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_byte(WRITER, 0x42, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 1, sizeof(uint8_t));

    ASSERT(EQ_UINT(value, 0x42));
}


TEST(write_int16_puts_zero) {
    int16_t value;

    n_write_int16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(int16_t));
    ASSERT(EQ_INT(value, 0));
}


TEST(write_int16_puts_maximum) {
    int16_t value;

    n_write_int16(WRITER, 32767, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(int16_t));
    ASSERT(EQ_INT(value, 32767));
}


TEST(write_int16_puts_minimum) {
    int16_t value;

    n_write_int16(WRITER, -32768, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(int16_t));
    ASSERT(EQ_INT(value, -32768));
}


TEST(write_int16_takes_2_bytes) {
    int16_t value;

    n_write_int16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_int16(WRITER, 12345, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 2, sizeof(int16_t));
    ASSERT(EQ_INT(value, 12345));
}


TEST(write_uint16_puts_zero) {
    uint16_t value;

    n_write_uint16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(uint16_t));
    ASSERT(EQ_UINT(value, 0));
}


TEST(write_uint16_puts_maximum) {
    uint16_t value;

    n_write_uint16(WRITER, 65535, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(uint16_t));
    ASSERT(EQ_UINT(value, 65535));
}


TEST(write_uint16_takes_2_bytes) {
    uint16_t value;

    n_write_uint16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_uint16(WRITER, 12345, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 2, sizeof(uint16_t));
    ASSERT(EQ_UINT(value, 12345));
}


TEST(write_int32_puts_zero) {
    int32_t value;

    n_write_int32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(int32_t));
    ASSERT(EQ_INT(value, 0));
}


TEST(write_int32_puts_maximum) {
    int32_t value;

    n_write_int32(WRITER, 2147483647, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(int32_t));
    ASSERT(EQ_INT(value, 2147483647));
}


TEST(write_int32_puts_minimum) {
    int32_t value;

    n_write_int32(WRITER, -2147483648, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(int32_t));
    ASSERT(EQ_INT(value, -2147483648));
}


TEST(write_int32_takes_4_bytes) {
    int32_t value;

    n_write_int32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_int32(WRITER, 1234567890, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 4, sizeof(int32_t));
    ASSERT(EQ_INT(value, 1234567890));
}


TEST(write_uint32_puts_zero) {
    uint32_t value;

    n_write_uint32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(uint32_t));
    ASSERT(EQ_UINT(value, 0));
}


TEST(write_uint32_puts_maximum) {
    uint32_t value;

    n_write_uint32(WRITER, 4294967295, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 0, sizeof(uint32_t));
    ASSERT(EQ_UINT(value, 4294967295));
}


TEST(write_uint32_takes_4_bytes) {
    uint32_t value;

    n_write_uint32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_uint32(WRITER, 1234567890, &ERR);
    ASSERT(IS_OK(ERR));

    destroy_writer();
    read_from_file(OUTPUT, &value, 4, sizeof(uint32_t));
    ASSERT(EQ_UINT(value, 1234567890));
}


AtTest* tests[] = {
    &write_byte_puts_maximum,
    &write_byte_puts_minimum,
    &write_byte_uses_1_byte,
    &write_int16_puts_zero,
    &write_int16_puts_maximum,
    &write_int16_puts_minimum,
    &write_int16_takes_2_bytes,
    &write_uint16_puts_zero,
    &write_uint16_puts_maximum,
    &write_uint16_takes_2_bytes,
    &write_int32_puts_zero,
    &write_int32_puts_maximum,
    &write_int32_puts_minimum,
    &write_int32_takes_4_bytes,
    &write_uint32_puts_zero,
    &write_uint32_puts_maximum,
    &write_uint32_takes_4_bytes,
    NULL
};


TEST_RUNNER("FileByteWriters", tests, constructor, NULL, setup, teardown)

static void 
read_from_file(const char* file_name, void* dest, size_t start, size_t size) {
    FILE* file = fopen(file_name, "rb");
    if (file == NULL) {
        ERROR("Could not open file for reading results.", NULL);
    }

    if (fseek(file, start, SEEK_SET) < 0) {
        ERROR("Could not move file cursor to the desired byte.", NULL);
    }

    if (fread(dest, 1, size, file) < size) {
        ERROR("Could not read enough bytes from the results file.", NULL);
    }
}

static void
destroy_writer(void) {
    n_destroy_byte_writer(WRITER, &ERR);
    if (!n_is_ok(&ERR)) {
        ERROR("Could not destroy the byte writer after using it.", NULL);
    }
    WRITER_IS_DESTROYED = 1;
}
