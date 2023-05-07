/* rvexception.h
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * C++ exceptions corresponding to RISC-V exceptions
 *
 * This is HORRIBLY inefficient and should be replaced with something better
 * Which is why, when we do XRVE in Rust, we'll use Result<T, E> instead with a custom error enum
 *
*/

#ifndef RVEXCEPTION_H
#define RVEXCEPTION_H

/* Includes */

#include <stdexcept>

#include "common.h"

/* Types */

typedef enum {
    //Interrupts
    SUPERVISOR_SOFTWARE_INTERRUPT               = 1,
    MACHINE_SOFTWARE_INTERRUPT                  = 3,
    SUPERVISOR_TIMER_INTERRUPT                  = 5,
    MACHINE_TIMER_INTERRUPT                     = 7,
    SUPERVISOR_EXTERNAL_INTERRUPT               = 9,
    MACHINE_EXTERNAL_INTERRUPT                  = 11,
    //Exceptions
    INSTRUCTION_ADDRESS_MISALIGNED_EXCEPTION    = 0,
    INSTRUCTION_ACCESS_FAULT_EXCEPTION          = 1,
    ILLEGAL_INSTRUCTION_EXCEPTION               = 2,
    BREAKPOINT_EXCEPTION                        = 3,
    LOAD_ADDRESS_MISALIGNED_EXCEPTION           = 4,
    LOAD_ACCESS_FAULT_EXCEPTION                 = 5,
    STORE_OR_AMO_ADDRESS_MISALIGNED_EXCEPTION   = 6,
    STORE_OR_AMO_ACCESS_FAULT_EXCEPTION         = 7,
    UMODE_ECALL_EXCEPTION                       = 8,
    SMODE_ECALL_EXCEPTION                       = 9,
    MMODE_ECALL_EXCEPTION                       = 11,
    INSTRUCTION_PAGE_FAULT_EXCEPTION            = 12,
    LOAD_PAGE_FAULT_EXCEPTION                   = 13,
    STORE_OR_AMO_PAGE_FAULT_EXCEPTION           = 15
} cause_t;

/* Function/Class Declarations */

class rvexception_t : public std::runtime_error {
public:
    rvexception_t(bool is_interrupt, cause_t cause);
    
    bool is_interrupt() const;
    cause_t cause() const;
    word_t raw_cause() const;
private:
    bool m_is_interrupt;
    cause_t m_cause;
};

#endif//RVEXCEPTION_H