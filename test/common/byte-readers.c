#include <stdlib.h>

#include "../test.h"

#include "common/byte-readers.h"

static
NByteReader *READER = NULL;

static
char TEST_DATA[256];

CONSTRUCTOR(constructor) {
    if (ni_init_byte_readers() < 0) {
        ERROR("Cant initialize byte readers module.", NULL);
    }

    {
        /* Initialize TEST_DATA with the first 256 bytes such that
         * byte(N) = N, for N in 0 .. 255; */
        int i;
        for (i = 0; i < 256; i++) {
            TEST_DATA[i] = i;
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
}


TEARDOWN(teardown) {
    NError error = n_error_ok();
    n_destroy_byte_reader(READER, &error);
    if (!n_is_ok(&error)) {
        n_destroy_error(&error);
        ERROR("Can't clean up byte reader after test.", NULL);
    }
}



AtTest* tests[] = {
    NULL
};


TEST_RUNNER("ByteReaders", tests, constructor, NULL, setup, teardown)
