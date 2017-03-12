#ifndef N_E_TYPES_H
#define N_E_TYPES_H

#include "../common/errors.h"

typedef struct NTypeRegistry NTypeRegistry;
typedef struct NType NType;

struct NType {
    const char* name;
};


int
ni_init_types();

void
n_construct_type(NType* type, const char* name);

NType*
n_find_type(const char* name, NError* error);

void
n_register_type(NType* type, NError* error);


#ifdef N_TEST

NTypeRegistry*
nt_create_type_registry();

void
nt_destroy_type_registry(NTypeRegistry*);

void
nt_register_type(NTypeRegistry*, NType*, NError*);

NType*
nt_type(NTypeRegistry* registry, const char* name, NError* error);
#endif /* N_TEST */

#endif /* N_E_TYPES_H */
