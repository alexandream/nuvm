#include <stdio.h>


#include "char-readers.h"

typedef struct NCharReaderVTable NCharReaderVTable;
typedef struct NMemoryCharReader NMemoryCharReader;

struct NCharReaderVTable {
    char (*peek)(NCharReader*, NError* error);
    void (*advance)(NCharReader*, NError* error);
    int (*is_eof)(NCharReader*, NError* error);
    void (*destroy)(NCharReader*, NError* error);
};


struct NCharReader {
    NCharReaderVTable *vtable;
};


struct NMemoryCharReader {
    NCharReader parent;
    char* buffer;
    size_t size;
    size_t cursor;
};

static char memory_peek(NCharReader*, NError* error);
static void memory_advance(NCharReader*, NError* error);
static int  memory_is_eof(NCharReader*, NError* error);
static void memory_destroy(NCharReader*, NError* error);


static
NCharReaderVTable MEMORY_VTABLE;

static
NErrorType* BAD_ALLOCATION = NULL;

static
NErrorType* UNEXPECTED_EOF = NULL;

static
NErrorType* IO_ERROR = NULL;


int
ni_init_char_readers(void) {
    NError error = n_error_ok();

    if (ni_init_errors() < 0) {
        return -1;
    }

    MEMORY_VTABLE.peek = memory_peek;
    MEMORY_VTABLE.advance = memory_advance;
    MEMORY_VTABLE.is_eof = memory_is_eof;
    MEMORY_VTABLE.destroy = memory_destroy;
    
    BAD_ALLOCATION = n_error_type("nuvm.BadAllocation", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -2;
    }

    UNEXPECTED_EOF = n_error_type("nuvm.UnexpectedEoF", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -3;
    }

    IO_ERROR = n_error_type("nuvm.IoError", &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        return -4;
    }
    
    return 0;
}

NCharReader*
ni_new_char_reader_from_path(const char* path, NError* error) {
	/* TODO : This should not load the file into memory. */
	/*   This function is very very stupid. It creates a "stream" by loading
	 * the whole file into memory and acting as if it's a big string.
	 *   For now this behaviour is good enough, but it should be turned into
	 * a real stream and do buffered input from the file. */
	int status, i;
	long file_size;
	char* buffer = NULL;
	NMemoryCharReader* result = NULL;

	FILE* file = NULL;

	file = fopen(path, "rb");
	if (file == NULL) {
		n_set_error(error, IO_ERROR, "Failed to open file");
		goto cleanup;
	}

	status = fseek(file, 0, SEEK_END);
	if (status != 0) {
		n_set_error(error, IO_ERROR, "Failed to position file cursor to "
                    "check file size");
		goto cleanup;
	}

	file_size = ftell(file);
	if (file_size < 0) {
		n_set_error(error, IO_ERROR, "Failed to discover file size");
		goto cleanup;
	}

	status = fseek(file, 0, SEEK_SET);
	if (status != 0) {
		n_set_error(error, IO_ERROR, "Failed to position file cursor for "
                    "reading");
		goto cleanup;
	}

	result = malloc(sizeof(NMemoryCharReader));
	if (result == NULL) {
		n_set_error(error, BAD_ALLOCATION, "Could not allocate character "
                    "reader");
		goto cleanup;
	}

	if (file_size > 0) {
		buffer = malloc(sizeof(char) * (file_size + 1));
		if (buffer == NULL) {
            n_set_error(error, BAD_ALLOCATION, "Could not allocate character "
                        "reader");
			goto cleanup;
		}

		i = 0;
		while (!feof(file)) {
			size_t n = fread(buffer + i, sizeof(char), file_size - i + 1, file);
			if (ferror(file)) {
				n_set_error(error, IO_ERROR, "Failed to read file contents");
				goto cleanup;
			}
			i += n;
		}
		buffer[file_size] = '\0';
	}
    result->parent.vtable = &MEMORY_VTABLE;
	result->buffer = buffer;
    /* TODO: Properly verify bounds on the conversion; */
	result->size = (size_t) file_size;
	result->cursor = 0;
	fclose(file);
	return (NCharReader*) result;

cleanup:
	if (file != NULL) {
		fclose(file);
	}
	if (result != NULL) {
		free(result);
	}
	if (buffer != NULL) {
		free(buffer);
	}
	return NULL;
}

void
ni_destroy_char_reader(NCharReader* reader, NError* error) {
    reader->vtable->destroy(reader, error);
}

int
ni_char_reader_is_eof(NCharReader* reader, NError* error) {
    return reader->vtable->is_eof(reader, error);
}

void
ni_advance_char(NCharReader* reader, NError* error) {
    reader->vtable->advance(reader, error);
}

char
ni_peek_char(NCharReader* reader, NError* error) {
    return reader->vtable->peek(reader, error);
}


/* Implementations for the memory backed character reader. */

static char
memory_peek(NCharReader* reader, NError* error) {
    NMemoryCharReader* self = (NMemoryCharReader*) reader;
    int is_eof = memory_is_eof(reader, error);

    if (is_eof) {
        n_set_error(error, UNEXPECTED_EOF, "Tried to peek from character "
                    "reader, got EOF instead.");
        return 0;
    }
    return self->buffer[self->cursor];
}


static void
memory_advance(NCharReader* reader, NError* error) {
    NMemoryCharReader* self = (NMemoryCharReader*) reader;
    int is_eof = memory_is_eof(reader, error);

    if (is_eof) {
        n_set_error(error, UNEXPECTED_EOF, "Tried to advance on character "
                    "reader, got EOF instead.");
        return;
    }
    
    self->cursor++;
}


static int
memory_is_eof(NCharReader* reader, NError* error) {
    NMemoryCharReader* self = (NMemoryCharReader*) reader;

    return self->cursor == self->size;
}


static void
memory_destroy(NCharReader* reader, NError* error) {

}

