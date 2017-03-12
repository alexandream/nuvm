#include "name-registry.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct NNamedEntry {
    const char* name;
    const void* object;
};



static int
is_valid_name_char(char c);

static int
is_valid_name(const char* name);

static int
grow_pool(NNameRegistry* self);



int
ni_construct_name_registry(NNameRegistry* self, int initial_size) {
    NNamedEntry* new_pool = NULL;

    if (initial_size <= 0) {
        return N_NAMED_REG_INVALID_SIZE;
    }

    new_pool = malloc(sizeof(NNamedEntry) * initial_size);
    if (new_pool == NULL) {
        return N_NAMED_REG_BAD_ALLOCATION;
    }
    self->pool = new_pool;
    self->pool_size = initial_size;
    self->num_objects = 0;
    return N_NAMED_REG_SUCCESS;

}


void
ni_destruct_name_registry(NNameRegistry* self) {
    free(self->pool);
    return;
}


int
ni_register_named_object(NNameRegistry* self, const char* name,
                         const void* obj) {
    if (name == NULL || *name == '\0' || !is_valid_name(name)) {
        return N_NAMED_REG_INVALID_NAME;
    }
    if (obj == NULL) {
        return N_NAMED_REG_INVALID_OBJECT;
    }
    if (self->num_objects >= self->pool_size) {
        int status = grow_pool(self);
        if (status != N_NAMED_REG_SUCCESS) {
            return status;
        }
    }

    {
        const void* original = ni_find_named_object(self, name);
        if (original != NULL) {
            return N_NAMED_REG_REPEATED_NAME;
        }
    }

    {
        int index = self->num_objects++;
        self->pool[index].name = name;
        self->pool[index].object = obj;
    }
    return N_NAMED_REG_SUCCESS;

}


int
ni_init_name_registry(void) {
    return 0;
}


const void*
ni_find_named_object(NNameRegistry* self, const char* name) {
    int i;
    for (i = 0; i < self->num_objects; i++) {
        if (! strcmp(self->pool[i].name, name)) {
            return self->pool[i].object;
        }
    }
    return NULL;
}



/********** STATIC FUNCTIONS **********/

static int
is_valid_name_char(char c) {
    return isalnum(c) || c == '_' || c == '.';
}


static int
is_valid_name(const char* name) {
    const char*p = name;
    while (*p) {
        if (!is_valid_name_char(*p++)) return 0;
    }
    return 1;
}


static int
grow_pool(NNameRegistry* self) {
    int new_size = self->pool_size * 2;
    NNamedEntry* new_pool =
        realloc(self->pool, sizeof(NNamedEntry) * new_size);
    if (new_pool == NULL) {
        return N_NAMED_REG_BAD_ALLOCATION;
    }
    self->pool = new_pool;
    self->pool_size = new_size;
    return N_NAMED_REG_SUCCESS;
}
