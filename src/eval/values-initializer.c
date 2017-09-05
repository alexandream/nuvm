#include "values.h"
#include "singletons.h"
#include "primitives.h"
#include "procedures.h"

int
ni_init_all_values(void) {
    if (ni_init_values() < 0) {
        return -1;
    }
    if (ni_init_singletons() < 0) {
        return -2;
    }
    if (ni_init_primitives() < 0) {
        return -3;
    }
    if (ni_init_procedures() < 0) {
        return -4;
    }
    return 0;
}

