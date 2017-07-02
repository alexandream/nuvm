#ifndef N_C_BYTE_READERS_H
#define N_C_BYTE_READERS_H

#include <stdlib.h>

#include "errors.h"
#include "compatibility/stdint.h"

typedef struct NByteReader NByteReader;
typedef struct NByteReaderVTable NByteReaderVTable;


struct NByteReaderVTable {
    uint8_t (*read_byte)(NByteReader*, NError*);
    uint16_t (*read_u16)(NByteReader*, NError*);
    int16_t (*read_i16)(NByteReader*, NError*);
    uint32_t (*read_u32)(NByteReader*, NError*);
    int32_t (*read_i32)(NByteReader*, NError*);
    int (*read_bytes)(NByteReader*, char*, int, NError*);
    int (*has_data)(NByteReader*);
	int (*skip_bytes)(NByteReader*, int, NError*);
    void (*destroy)(NByteReader*, NError*);
};

int
ni_init_byte_readers(void);

NByteReader*
n_new_byte_reader_from_data(char* data, int size, NError* error);

NByteReader*
n_new_byte_reader_from_file(const char* file_name, NError* error);

uint8_t
n_read_byte(NByteReader* self, NError* error);

uint16_t
n_read_uint16(NByteReader* self, NError* error);

int16_t
n_read_int16(NByteReader* self, NError* error);

uint32_t
n_read_uint32(NByteReader* self, NError* error);

int32_t
n_read_int32(NByteReader* self, NError* error);

int
n_read_bytes(NByteReader* self, char* dest, int size, NError* error);

int
n_skip_bytes(NByteReader*self, int num_bytes, NError* error);

void
n_destroy_byte_reader(NByteReader* self, NError* error);

#endif /*N_C_INPUT_BYTE_STREAM_H*/

