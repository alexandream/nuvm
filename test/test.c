#include <string.h>


#include "test.h"

AtCheckResult
is_error(const char* expr, NError* err, const char* type_name) {
	if (!err) {
		return at_make_error("Received a null pointer to test for errors.",
		                     NULL);
	}
	else if (!(err->type)) {
		const char* msg =
			at_allocf("Expression (%s) expected to have error type "
		              "\"%s\" but found to be successful.",
                       expr, type_name);
		if (msg == NULL) {
			at_make_error("Allocation error building failure message.", NULL);
		}
		return at_make_failure(msg, at_freef);
	}
	else if (!(err->type->name)) {
		return at_make_error("Received an error with an unnamed type to test "
	                         "For errors.",
		                     NULL);
	}
	else if (strcmp(err->type->name, type_name)) {
		const char* msg =
			at_allocf("Expression (%s) expected to have error type "
		              "\"%s\" but found to have type \"%s\".",
                       expr, type_name, err->type->name);
		if (msg == NULL) {
			at_make_error("Allocation error building failure message.", NULL);
		}
		return at_make_failure(msg, at_freef);
	}
	return at_make_success();
}


AtCheckResult
is_error_type(const char* expr, NError* err, NErrorType* type) {
	if (!err) {
		return at_make_error("Received a null pointer to test for errors.",
		                     NULL);
	}
    else if (err->type != type) {
        const char* val_name = (err->type)? err->type->name : NULL;
        const char* exp_name = (type) ? type->name : NULL;
        const char* msg =
            at_allocf("Expression (%s) expected to have error type %s "
                      "but found to have type %s.",
                      expr, exp_name, val_name);
        if (msg == NULL) {
            at_make_error("Allocation error while building failure message.",
                          NULL);
        }
        return at_make_failure(msg, at_freef);
    }
    return at_make_success();
}
