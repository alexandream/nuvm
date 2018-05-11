#ifndef N_C_NAMED_REG_H
#define N_C_NAMED_REG_H


#define N_NAMED_REG_INVALID_OBJECT -5
#define N_NAMED_REG_UNKNOWN_NAME   -5
#define N_NAMED_REG_REPEATED_NAME  -4
#define N_NAMED_REG_INVALID_NAME   -3
#define N_NAMED_REG_BAD_ALLOCATION -2
#define N_NAMED_REG_INVALID_SIZE   -1
#define N_NAMED_REG_SUCCESS         0


typedef struct NNameRegistry NNameRegistry;
typedef struct NNamedEntry NNamedEntry;

struct NNameRegistry {
    int pool_size;
    int num_objects;
    NNamedEntry *pool;
};


int
ni_construct_name_registry(NNameRegistry* self, int initial_size);

void
ni_destruct_name_registry(NNameRegistry* self);

int
ni_register_named_object(NNameRegistry* self, const char* name,
                         const void* obj);

const void*
ni_find_named_object(NNameRegistry*, const char* name);

#endif /*N_C_NAMED_REG_H*/

