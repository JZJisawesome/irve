/* logging.h
 * By: John Jekel
 *
 * Logging facilities for
 *
 * Based on code from rv32esim
*/

#ifndef LOGGING_H
#define LOGGING_H

/* Includes */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "config.h"

/* Constants And Defines */

#ifndef INST_COUNT
#error "INST_COUNT must be defined before including logging.h"
#endif

#if IRVE_INTERNAL_CONFIG_DISABLE_LOGGING

//Compiles down to nothing, but prevents warnings/errors if logging is disabled
#define irvelog(indent, ...) do { \
    ((void)(INST_COUNT)); \
    ((void)(indent)); \
} while (0)

#else

#define irvelog(indent, ...) do { \
    irvelog_always(indent, __VA_ARGS__); \
} while (0)

#endif

//USE THIS SPARINGLY
#define irvelog_always(indent, ...) do { \
    irve::internal::logging::irvelog_internal_function_dont_use_this_directly(stderr, INST_COUNT, indent, __VA_ARGS__); \
} while (0)

//SHOULD ONLY BE USED FOR RISC-V OUTPUT
#define irvelog_always_stdout(indent, ...) do { \
    irve::internal::logging::irvelog_internal_function_dont_use_this_directly(stdout, INST_COUNT, indent, __VA_ARGS__); \
} while (0)

/* Function/Class Declarations */

namespace irve::internal::logging {
    //TODO have -1 mean end
    void irvelog_internal_function_dont_use_this_directly(FILE* destination, uint64_t inst_num, uint8_t indent, const char* str, ...);
    void irvelog_internal_function_dont_use_this_directly(FILE* destination, uint64_t inst_num, uint8_t indent, const char* str, va_list list);
}

#endif//LOGGING_H
