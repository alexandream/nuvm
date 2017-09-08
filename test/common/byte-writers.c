#include <stdlib.h>
#include <string.h>

#include "../test.h"

#include "common/byte-writers.h"

static
NByteWriter *WRITER = NULL;

#define BUFFER_SIZE 256

static
char BUFFER[BUFFER_SIZE];

static
NError ERR;


CONSTRUCTOR(constructor) {
    if (ni_init_byte_writers() < 0) {
        ERROR("Cant initialize byte writers module.", NULL);
    }
}


SETUP(setup) {
    NError error = n_error_ok();
    int i;
    /* Set all bits to alternating one and zero in the buffer. */
    for (i = 0; i < BUFFER_SIZE; i++) {
        BUFFER[i] = 0xAA;
    }

    WRITER = n_create_memory_byte_writer(BUFFER, BUFFER_SIZE, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't setup byte writer for test.", NULL);
    }

    ERR = n_error_ok();
}


TEARDOWN(teardown) {
    NError error = n_error_ok();
    n_destroy_byte_writer(WRITER, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't clean up byte writer after test.", NULL);
    }
}


TEST(write_byte_puts_maximum) {
    n_write_byte(WRITER, 0xFF, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(BUFFER[0], 0xFF));
}


TEST(write_byte_puts_minimum) {
    n_write_byte(WRITER, 0x00, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(BUFFER[0], 0x00));
}


TEST(write_byte_uses_1_byte) {
    n_write_byte(WRITER, 0x00, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_byte(WRITER, 0x42, &ERR);
    ASSERT(IS_OK(ERR));
    ASSERT(EQ_UINT(BUFFER[1], 0x42));
}


TEST(write_int16_puts_zero) {
    int16_t value;

    n_write_int16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(int16_t));
    ASSERT(EQ_INT(value, 0));
}


TEST(write_int16_puts_maximum) {
    int16_t value;

    n_write_int16(WRITER, 32767, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(int16_t));
    ASSERT(EQ_INT(value, 32767));
}


TEST(write_int16_puts_minimum) {
    int16_t value;

    n_write_int16(WRITER, -32768, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(int16_t));
    ASSERT(EQ_INT(value, -32768));
}


TEST(write_int16_takes_2_bytes) {
    int16_t value;

    n_write_int16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_int16(WRITER, 12345, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER+2, sizeof(int16_t));
    ASSERT(EQ_INT(value, 12345));
}


TEST(write_uint16_puts_zero) {
    uint16_t value;

    n_write_uint16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(uint16_t));
    ASSERT(EQ_UINT(value, 0));
}


TEST(write_uint16_puts_maximum) {
    uint16_t value;

    n_write_uint16(WRITER, 65535, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(uint16_t));
    ASSERT(EQ_UINT(value, 65535));
}


TEST(write_uint16_takes_2_bytes) {
    uint16_t value;

    n_write_uint16(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_uint16(WRITER, 12345, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER+2, sizeof(uint16_t));
    ASSERT(EQ_UINT(value, 12345));
}


TEST(write_int32_puts_zero) {
    int32_t value;

    n_write_int32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(int32_t));
    ASSERT(EQ_INT(value, 0));
}


TEST(write_int32_puts_maximum) {
    int32_t value;

    n_write_int32(WRITER, 2147483647, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(int32_t));
    ASSERT(EQ_INT(value, 2147483647));
}


TEST(write_int32_puts_minimum) {
    int32_t value;

    n_write_int32(WRITER, -2147483648, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(int32_t));
    ASSERT(EQ_INT(value, -2147483648));
}


TEST(write_int32_takes_4_bytes) {
    int32_t value;

    n_write_int32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_int32(WRITER, 1234567890, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER+4, sizeof(int32_t));
    ASSERT(EQ_INT(value, 1234567890));
}


TEST(write_uint32_puts_zero) {
    uint32_t value;

    n_write_uint32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(uint32_t));
    ASSERT(EQ_UINT(value, 0));
}


TEST(write_uint32_puts_maximum) {
    uint32_t value;

    n_write_uint32(WRITER, 4294967295, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER, sizeof(uint32_t));
    ASSERT(EQ_UINT(value, 4294967295));
}


TEST(write_uint32_takes_2_bytes) {
    uint32_t value;

    n_write_uint32(WRITER, 0, &ERR);
    ASSERT(IS_OK(ERR));

    n_write_uint32(WRITER, 1234567890, &ERR);
    ASSERT(IS_OK(ERR));

    memcpy(&value, BUFFER+2, sizeof(uint32_t));
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
    &write_uint32_takes_2_bytes,
    NULL
};


TEST_RUNNER("ByteWriters", tests, constructor, NULL, setup, teardown)
