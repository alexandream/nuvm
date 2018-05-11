#ifndef N_C_BYTE_WRITERS_H
#define N_C_BYTE_WRITERS_H

#include <stdlib.h>

#include "errors.h"
#include "compatibility/stdint.h"

typedef struct NByteWriter NByteWriter;


void
ni_init_byte_writers(NError* error);

NByteWriter*
n_create_memory_byte_writer(void* dest, size_t size, NError* error);

NByteWriter*
n_create_file_byte_writer(const char* file_name, NError* error);

void
n_write_byte(NByteWriter* self, uint8_t value, NError* error);

void
n_write_uint16(NByteWriter* self, uint16_t value, NError* error);

void
n_write_int16(NByteWriter* self, int16_t value, NError* error);

void
n_write_uint32(NByteWriter* self, uint32_t value, NError* error);

void
n_write_int32(NByteWriter* self, int32_t value, NError* error);

void
n_flush_byte_writer(NByteWriter* self, NError* error);

void
n_destroy_byte_writer(NByteWriter* self, NError* error);

#endif /*N_C_INPUT_BYTE_STREAM_H*/

