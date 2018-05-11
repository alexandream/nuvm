#include <string.h>

#include "byte-readers.h"

typedef struct NMemoryByteReader NMemoryByteReader;

struct NByteReader {
    const NByteReaderVTable *vtable;
};

struct NMemoryByteReader {
    NByteReader parent;
    const void *buffer;
    int current_index;
    int size;
};



static
NByteReaderVTable MEMORY_VTABLE;

static
NErrorType *UNEXPECTED_EOF =  NULL;


static void
construct_byte_reader(NByteReader*, NByteReaderVTable*);

static NMemoryByteReader*
new_memory_byte_reader(const void*, int);

static uint8_t
memory_read_byte(NByteReader*, NError*);

static uint16_t
memory_read_u16(NByteReader*, NError*);

static int16_t
memory_read_i16(NByteReader*, NError*);

static uint32_t
memory_read_u32(NByteReader*, NError*);

static int32_t
memory_read_i32(NByteReader*, NError*);

static int
memory_read_bytes(NByteReader*, void*, int, NError*);

static int
memory_skip_bytes(NByteReader* generic_self, int size, NError* error);

static int
memory_has_data(NByteReader*);

static void
memory_destroy(NByteReader*, NError*);



int
ni_init_byte_readers(void) {
    NError error = n_error_ok();

    UNEXPECTED_EOF = n_error_type("nuvm.UnexpectedEoF", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    MEMORY_VTABLE.read_byte  = memory_read_byte;
    MEMORY_VTABLE.read_u16   = memory_read_u16;
    MEMORY_VTABLE.read_i16   = memory_read_i16;
    MEMORY_VTABLE.read_u32   = memory_read_u32;
    MEMORY_VTABLE.read_i32   = memory_read_i32;
    MEMORY_VTABLE.read_bytes = memory_read_bytes;
    MEMORY_VTABLE.skip_bytes = memory_skip_bytes;
    MEMORY_VTABLE.has_data   = memory_has_data;
    MEMORY_VTABLE.destroy    = memory_destroy;

    return 0;
}


NByteReader*
n_new_byte_reader_from_data(void* data, int size, NError* error) {
    NMemoryByteReader* self =  new_memory_byte_reader(data, size);
    return (NByteReader*) self;
}


NByteReader*
n_new_byte_reader_from_file(const char* file_name, NError* error) {
    return NULL;
}


uint8_t
n_read_byte(NByteReader* self, NError* error) {
    return self->vtable->read_byte(self, error);
}


uint16_t
n_read_uint16(NByteReader* self, NError* error) {
    return self->vtable->read_u16(self, error);
}


int16_t
n_read_int16(NByteReader* self, NError* error) {
    return self->vtable->read_i16(self, error);
}


uint32_t
n_read_uint32(NByteReader* self, NError* error) {
    return self->vtable->read_u32(self, error);
}


int32_t
n_read_int32(NByteReader* self, NError* error) {
    return self->vtable->read_i32(self, error);
}


int
n_read_bytes(NByteReader* self, void* dest, int size, NError* error) {
    return self->vtable->read_bytes(self, dest, size, error);
}


int
n_has_bytes_to_read(NByteReader* self) {
    return self->vtable->has_data(self);
}


int
n_skip_bytes(NByteReader*self, int num_bytes, NError* error) {
    return self->vtable->skip_bytes(self, num_bytes, error);
}


void
n_destroy_byte_reader(NByteReader* self, NError* error) {
    self->vtable->destroy(self, error);
}


static NMemoryByteReader*
new_memory_byte_reader(const void* data, int size) {
    NMemoryByteReader* self = malloc(sizeof(NMemoryByteReader));
    if (self == NULL) {
        return NULL;
    }
    construct_byte_reader((NByteReader*) self, &MEMORY_VTABLE);

    self->buffer = data;
    self->size = size;
    self->current_index = 0;

    return self;
}


static int
memory_trunc_to_available(NMemoryByteReader* self, int size) {
    int available_bytes = self->size - self->current_index;
    return (available_bytes < size) ? available_bytes : size;
}

static int assert_readable_size(NMemoryByteReader *self, int size,
                                NError *error) {
    int available = memory_trunc_to_available(self, size);
    if (available < size) {
        n_set_error(error, UNEXPECTED_EOF, "Depleted buffer while trying to "
                    "read from byte reader.");
        return 0;
    }
    return 1;
}
static uint8_t
memory_read_byte(NByteReader* generic_self, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    const uint8_t* b = (const uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 1, error)) return 0;

    self->current_index += 1;
    return *b;
}

static uint16_t
memory_read_u16(NByteReader* generic_self, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    const uint8_t* b = (const uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 2, error)) return 0;

    self->current_index += 2;
    return (((uint16_t) b[1]) << 8) + b[0];
}


static int16_t
memory_read_i16(NByteReader* generic_self, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    const uint8_t* b = (const uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 2, error)) return 0;

    self->current_index += 2;
    return (int16_t) ((((uint16_t) b[1]) << 8) + b[0]);
}


static uint32_t
memory_read_u32(NByteReader* generic_self, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    const uint8_t* b = (const uint8_t*) self->buffer + self->current_index;

    if (!assert_readable_size(self, 4, error)) return 0;

    self->current_index += 4;
    return (((uint32_t) b[3]) << 24)
         + (((uint32_t) b[2]) << 16)
         + (((uint32_t) b[1]) << 8)
         + b[0];
}

static int32_t
memory_read_i32(NByteReader* generic_self, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    const uint8_t* b = (const uint8_t*) self->buffer + self->current_index;
    if (!assert_readable_size(self, 4, error)) return 0;
    self->current_index += 4;
    return (int32_t) ((((int32_t) b[3]) << 24)
                    + (((int32_t) b[2]) << 16)
                    + (((int32_t) b[1]) << 8)
                    +             b[0]);
}


static int
memory_read_bytes(NByteReader* generic_self, void* dest,
                  int size, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    int readable_size = memory_trunc_to_available(self, size);

    memcpy(dest, (char*) self->buffer + self->current_index, readable_size);
    return readable_size;
}


static int
memory_skip_bytes(NByteReader* generic_self, int size, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    int skippable_size = memory_trunc_to_available(self, size);
    self->current_index += skippable_size;
    return skippable_size;
}

static int
memory_has_data(NByteReader* generic_self) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    return self->current_index < self->size;
}


static void
memory_destroy(NByteReader* generic_self, NError* error) {
    NMemoryByteReader* self = (NMemoryByteReader*) generic_self;
    free(self);
}


static void
construct_byte_reader(NByteReader* self, NByteReaderVTable* vtable) {
    self->vtable = vtable;
}
