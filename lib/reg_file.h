/* reg_file.h
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 * 
 * The register file
 * Basically works like an array of 32 reg_t but index 0 will always be 0
 * 
*/

#ifndef REG_FILE_H
#define REG_FILE_H

/* Includes */

#include <cstdint>
#include <cassert>
#include "common.h"

/* Function/Class Declarations */

class reg_file_t {
private:
    mutable irve::internal::reg_t regs[32];
public:
    reg_file_t();
    irve::internal::reg_t& operator[](uint8_t i);
    const irve::internal::reg_t& operator[](uint8_t i) const;
};

#endif