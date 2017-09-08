#include <stdio.h>

#include "compatibility/stdint.h"
#include "byte-writers.h"

typedef struct NByteWriterVTable NByteWriterVTable;

struct NByteWriterVTable {
    void (*write_byte)(NByteWriter*, uint8_t,  NError*);
    void (*write_u16)(NByteWriter*,  uint16_t, NError*);
    void (*write_i16)(NByteWriter*,  int16_t,  NError*);
    void (*write_u32)(NByteWriter*,  uint32_t, NError*);
    void (*write_i32)(NByteWriter*,  int32_t,  NError*);
    void (*flush)(NByteWriter*, NError*);
    void (*destroy)(NByteWriter*, NError*);
};


struct NByteWriter {
    const NByteWriterVTable *vtable;
};

typedef struct NMemoryByteWriter NMemoryByteWriter;
struct NMemoryByteWriter {
    NByteWriter parent;
    const void *buffer;
    size_t current_index;
    size_t size;
};

typedef struct NFileByteWriter NFileByteWriter;
struct NFileByteWriter {
    NByteWriter parent;
    FILE* stream;
};

static
NByteWriterVTable MEMORY_VTABLE;

static
NByteWriterVTable FILE_VTABLE;

static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* OVERFLOW = NULL;

static
NErrorType* IO_ERROR = NULL;

static void
construct_byte_writer(NByteWriter*, NByteWriterVTable*);

static void
memory_write_byte(NByteWriter*, uint8_t,  NError*);

static void
memory_write_u16(NByteWriter*,  uint16_t, NError*);

static void
memory_write_i16(NByteWriter*,  int16_t,  NError*);

static void
memory_write_u32(NByteWriter*,  uint32_t, NError*);

static void
memory_write_i32(NByteWriter*,  int32_t,  NError*);

static void
memory_flush(NByteWriter*, NError*);

static void
memory_destroy(NByteWriter*, NError*);


static void
file_write_byte(NByteWriter*, uint8_t,  NError*);

static void
file_write_u16(NByteWriter*,  uint16_t, NError*);

static void
file_write_i16(NByteWriter*,  int16_t,  NError*);

static void
file_write_u32(NByteWriter*,  uint32_t, NError*);

static void
file_write_i32(NByteWriter*,  int32_t,  NError*);

static void
file_flush(NByteWriter*, NError*);

static void
file_destroy(NByteWriter*, NError*);


int
ni_init_byte_writers(void){
    NError error = n_error_ok();

    if (ni_init_errors() < 0) {
        return -1;
    }

    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    BAD_ALLOCATION = n_error_type("nuvm.Overflow", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -3;
    }

    IO_ERROR = n_error_type("nuvm.IoError", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -4;
    }

    MEMORY_VTABLE.write_byte = memory_write_byte;
    MEMORY_VTABLE.write_u16  = memory_write_u16;
    MEMORY_VTABLE.write_i16  = memory_write_i16;
    MEMORY_VTABLE.write_u32  = memory_write_u32;
    MEMORY_VTABLE.write_i32  = memory_write_i32;
    MEMORY_VTABLE.flush      = memory_flush;
    MEMORY_VTABLE.destroy    = memory_destroy;

    FILE_VTABLE.write_byte = file_write_byte;
    FILE_VTABLE.write_u16  = file_write_u16;
    FILE_VTABLE.write_i16  = file_write_i16;
    FILE_VTABLE.write_u32  = file_write_u32;
    FILE_VTABLE.write_i32  = file_write_i32;
    FILE_VTABLE.flush      = file_flush;
    FILE_VTABLE.destroy    = file_destroy;
    return 0;
}


NByteWriter*
n_create_memory_byte_writer(void* dest, size_t size, NError* error){
    NMemoryByteWriter* self = malloc(sizeof(NMemoryByteWriter));
    if (self == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate a byte writer.",
                    NULL, NULL);
    }
    construct_byte_writer((NByteWriter*) self, &MEMORY_VTABLE);

    self->buffer = dest;
    self->size = size;
    self->current_index = 0;

    return (NByteWriter*) self;
}


NByteWriter*
n_create_file_byte_writer(const char* file_name, NError* error){
    FILE *file;
    NFileByteWriter* self = malloc(sizeof(NFileByteWriter));
    if (self == NULL) {
        n_set_error(error, BAD_ALLOCATION, "Could not allocate a byte writer.",
                    NULL, NULL);
    }

    file = fopen(file_name, "w");
    if (file == NULL) {
        free(self);
        n_set_error(error, IO_ERROR, "Could not open file.", NULL, NULL);
    }


    construct_byte_writer((NByteWriter*) self, &FILE_VTABLE);

    self->stream = file;
    return (NByteWriter*) self;;
}


void
n_write_byte(NByteWriter* self, uint8_t value, NError* error){
    self->vtable->write_byte(self, value, error);
}


void
n_write_uint16(NByteWriter* self, uint16_t value, NError* error){
    self->vtable->write_u16(self, value, error);
}


void
n_write_int16(NByteWriter* self, int16_t value, NError* error){
    self->vtable->write_i16(self, value, error);
}


void
n_write_uint32(NByteWriter* self, uint32_t value, NError* error){
    self->vtable->write_u32(self, value, error);
}


void
n_write_int32(NByteWriter* self, int32_t value, NError* error){
    self->vtable->write_i32(self, value, error);
}


void
n_flush_byte_writer(NByteWriter* self, NError* error) {
    self->vtable->flush(self, error);
}


void
n_destroy_byte_writer(NByteWriter* self, NError* error){
    self->vtable->destroy(self, error);
}




static void
construct_byte_writer(NByteWriter* self, NByteWriterVTable* vtable) {
    self->vtable = vtable;
}


static size_t
memory_trunc_size_to_available(NMemoryByteWriter* self, size_t size) {
    size_t available_bytes = self->size - self->current_index;
    return (available_bytes < size) ? available_bytes : size;
}


static int
assert_readable_size(NMemoryByteWriter *self, size_t size, NError *error) {
    size_t available = memory_trunc_size_to_available(self, size);
    if (available < size) {
        n_set_error(error, OVERFLOW, "Depleted buffer while trying to "
                    "write on byte writer.", NULL, NULL);
        return 0;
    }
    return 1;
}


static void
memory_write_byte(NByteWriter* generic_self, uint8_t value,  NError* error) {
    NMemoryByteWriter* self = (NMemoryByteWriter*) generic_self;
    uint8_t* b = (uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 1, error)) return;

    self->current_index += 1;
    *b = value;
}


static void
memory_write_u16(NByteWriter* generic_self,  uint16_t value, NError* error) {
    NMemoryByteWriter* self = (NMemoryByteWriter*) generic_self;
    uint8_t* b = (uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 2, error)) return;

    self->current_index += 2;
    b[0] = (uint8_t) (value & 0x00FF);
    b[1] = (uint8_t) ((value & 0xFFFF) >> 8);
}


static void
memory_write_i16(NByteWriter* generic_self,  int16_t value,  NError* error) {
    NMemoryByteWriter* self = (NMemoryByteWriter*) generic_self;
    uint8_t* b = (uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 2, error)) return;

    self->current_index += 2;
    b[0] = (uint8_t) (value & 0x00FF);
    b[1] = (uint8_t) ((value & 0xFF00) >> 8);
}


static void
memory_write_u32(NByteWriter* generic_self,  uint32_t value, NError* error) {
    NMemoryByteWriter* self = (NMemoryByteWriter*) generic_self;
    uint8_t* b = (uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 4, error)) return;

    self->current_index += 4;
    b[0] = (uint8_t) (value & 0x000000FF);
    b[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    b[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    b[3] = (uint8_t) ((value & 0xFF000000) >> 24);
}


static void
memory_write_i32(NByteWriter* generic_self,  int32_t value,  NError* error) {
    NMemoryByteWriter* self = (NMemoryByteWriter*) generic_self;
    uint8_t* b = (uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 4, error)) return;

    self->current_index += 4;
    b[0] = (uint8_t) (value & 0x000000FF);
    b[1] = (uint8_t) ((value & 0x0000FF00) >> 8);
    b[2] = (uint8_t) ((value & 0x00FF0000) >> 16);
    b[3] = (uint8_t) ((value & 0xFF000000) >> 24);
}


static void
memory_flush(NByteWriter* self, NError* error) {

}


static void
memory_destroy(NByteWriter* self, NError* error) {
    free(self);
}


static void
write_to_file(FILE* stream, void* src, size_t size, NError* error) {
    if (fwrite(src, 1, size, stream) != size) {
        n_set_error(error, IO_ERROR, "Could not write to file.", NULL, NULL);
    }
}


static void
file_write_byte(NByteWriter* generic_self, uint8_t value,  NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;
    write_to_file(self->stream, &value, sizeof(uint8_t), error);
}


static void
file_write_u16(NByteWriter* generic_self,  uint16_t value, NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;
    write_to_file(self->stream, &value, sizeof(uint16_t), error);
}


static void
file_write_i16(NByteWriter* generic_self,  int16_t value,  NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;
    write_to_file(self->stream, &value, sizeof(int16_t), error);
}


static void
file_write_u32(NByteWriter* generic_self,  uint32_t value, NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;
    write_to_file(self->stream, &value, sizeof(uint32_t), error);
}


static void
file_write_i32(NByteWriter* generic_self,  int32_t value,  NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;
    write_to_file(self->stream, &value, sizeof(int32_t), error);
}


static void
file_flush(NByteWriter* generic_self, NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;

    fflush(self->stream);
}


static void
file_destroy(NByteWriter* generic_self, NError* error){
    NFileByteWriter* self = (NFileByteWriter*) generic_self;

    fclose(self->stream);
    free(self);
}

