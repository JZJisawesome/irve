/* cpu_state.cpp
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * Performs unit tests for IRVE's cpu_state.h and cpu_state.cpp
 *
*/

/* Includes */

#include <cassert>
#include <cstdint>
#include "common.h"
#include "memory.h"
#include "cpu_state.h"

#include "rvexception.h"

/* Function Implementations */

int test_cpu_state_t() {
    memory_t memory;
    cpu_state_t cpu_state(memory);

    assert(cpu_state.get_inst_count() == 0);

    for (uint32_t i = 0; i < 123; ++i) {
        cpu_state.increment_inst_count();
        assert(cpu_state.get_inst_count() == (i + 1));
    }

    cpu_state.set_pc(0x00000000);
    assert(cpu_state.get_pc() == 0x00000000);
    cpu_state.set_pc(0xABCD1234);
    assert(cpu_state.get_pc() == 0xABCD1234);
    //TODO what if the PC is misaligned?

    assert(cpu_state.get_r(0) == 0);
    cpu_state.set_r(0, 0x12345678);
    assert(cpu_state.get_r(0) == 0);
    cpu_state.set_r(1, 0x98765432);
    assert(cpu_state.get_r(1) == 0x98765432);
    cpu_state.set_r(2, 0xABCD1234);
    assert(cpu_state.get_r(2) == 0xABCD1234);
    cpu_state.set_r(3, 0x1234ABCD);
    assert(cpu_state.get_r(3) == 0x1234ABCD);
    cpu_state.set_r(4, 0x00000000);
    assert(cpu_state.get_r(4) == 0x00000000);
    cpu_state.set_r(5, 0xFFFFFFFF);
    assert(cpu_state.get_r(5) == 0xFFFFFFFF);
    cpu_state.set_r(20, -123);
    assert(cpu_state.get_r(20) == -123);
    cpu_state.set_r(31, 0xFFFFFFFF);
    assert(cpu_state.get_r(31) == -1);
    assert(cpu_state.get_r(2) == 0xABCD1234);
    cpu_state.set_r(2, 0x7FFFFFFF);
    assert(cpu_state.get_r(2) == 0x7FFFFFFF);

    //TODO test more

    return 0;
}