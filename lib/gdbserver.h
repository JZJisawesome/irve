/**
 * @file    gdbserver.h
 * @brief   Minimal GDB server implementation to ease debugging
 * 
 * @copyright Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 * 
 * TODO longer description
 *
*/

#ifndef GDBSERVER_H
#define GDBSERVER_H

/* ------------------------------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------------------------- */

#include <cstdint>

//Needed to deal with circular headers
namespace irve::internal::emulator {
    class emulator_t;
}

#include "emulator.h"

/* ------------------------------------------------------------------------------------------------
 * Function Declarations
 * --------------------------------------------------------------------------------------------- */

namespace irve::internal::gdbserver {

    /**
     * @brief Start the GDB server
     * @param emulator The emulator to use
     * @param port The port to listen on
    */
    void start(
        emulator::emulator_t& emulator,
        cpu_state::cpu_state_t& cpu_state,
        memory::memory_t& memory,
        uint16_t port
    );
}

#endif//GDBSERVER_H
