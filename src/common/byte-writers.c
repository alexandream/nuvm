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


static
NByteWriterVTable MEMORY_VTABLE;

static
NErrorType* BAD_ALLOCATION = NULL;

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

    MEMORY_VTABLE.write_byte = memory_write_byte;
    MEMORY_VTABLE.write_u16  = memory_write_u16;
    MEMORY_VTABLE.write_i16  = memory_write_i16;
    MEMORY_VTABLE.write_u32  = memory_write_u32;
    MEMORY_VTABLE.write_i32  = memory_write_i32;
    MEMORY_VTABLE.flush      = memory_flush;
    MEMORY_VTABLE.destroy    = memory_destroy;

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
    return NULL;
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


static void
memory_write_byte(NByteWriter* self, uint8_t value,  NError* error) {

}


static void
memory_write_u16(NByteWriter* self,  uint16_t value, NError* error) {

}


static void
memory_write_i16(NByteWriter* self,  int16_t value,  NError* error) {

}


static void
memory_write_u32(NByteWriter* self,  uint32_t value, NError* error) {

}


static void
memory_write_i32(NByteWriter* self,  int32_t value,  NError* error) {

}


static void
memory_flush(NByteWriter* self, NError* error) {

}


static void
memory_destroy(NByteWriter* self, NError* error) {

}

