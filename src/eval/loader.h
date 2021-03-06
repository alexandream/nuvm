#ifndef N_E_LOADER_H
#define N_E_LOADER_H

#include <stdlib.h>

#include "../common/byte-readers.h"
#include "modules.h"

void
ni_init_loader(NError* error);

NModule*
n_read_module(NByteReader* reader, NError* error);




#ifdef N_TEST

NValue
nt_read_fixnum32_global(NByteReader* reader, NError* error);

NValue
nt_read_procedure_global(NByteReader* reader, NModule* module, NError* error);

NValue
nt_read_global(NByteReader* reader, NModule* module, NError* error);

#endif /* N_TEST */

#endif /* N_E_LOADER_H */

