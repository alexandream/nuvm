#include "byte-readers.h"

typedef struct NMemoryByteReader NMemoryByteReader;

struct NByteReader {
    const NByteReaderVTable *vtable;
};

struct NMemoryByteReader {
    NByteReader parent;
};



static
NByteReaderVTable MEMORY_VTABLE;

static
NErrorType *UNEXPECTED_EOF =  NULL;


static void
construct_byte_reader(NByteReader*, NByteReaderVTable*);

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
memory_read_bytes(NByteReader*, char*, int, NError*);

static int
memory_skip_bytes(NByteReader* generic_self, int size, NError* error);

static int
memory_has_data(NByteReader*);

static void
memory_destroy(NByteReader*, NError*);



int
ni_init_byte_readers(void) {
    NError error = n_error_ok();

    if (ni_init_errors() < 0) {
        return -1;
    }

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
n_new_byte_reader_from_data(char* data, int size, NError* error) {
    NMemoryByteReader* self = malloc(sizeof(NMemoryByteReader));
    construct_byte_reader((NByteReader*) self, &MEMORY_VTABLE);
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
n_read_bytes(NByteReader* self, char* dest, int size, NError* error) {
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



static uint8_t
memory_read_byte(NByteReader* generic_self, NError* error) {
    return 0;
}


static uint16_t
memory_read_u16(NByteReader* generic_self, NError* error) {
    return 0;
}


static int16_t
memory_read_i16(NByteReader* generic_self, NError* error) {
    return 0;
}


static uint32_t
memory_read_u32(NByteReader* generic_self, NError* error) {
    return 0;
}


static int32_t
memory_read_i32(NByteReader* generic_self, NError* error) {
    return 0;
}


static int
memory_read_bytes(NByteReader* generic_self, char* dest,
                  int size, NError* error) {
    return 0;
}


static int
memory_skip_bytes(NByteReader* generic_self, int size, NError* error) {
    return 0;
}

static int
memory_has_data(NByteReader* generic_self) {
    return 0;
}


static void
memory_destroy(NByteReader* generic_self, NError* error) {

}


static void
construct_byte_reader(NByteReader* self, NByteReaderVTable* vtable) {
    self->vtable = vtable;
}
