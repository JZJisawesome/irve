/* gdbserver.h
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * Minimal GDB server implementation to ease debugging
 *
*/

#ifndef GDBSERVER_H
#define GDBSERVER_H

/* Includes */

//Needed to deal with circular headers
namespace irve::internal::emulator {
    class emulator_t;
}

#include "emulator.h"

/* Function/Class Declarations */

namespace irve::internal::gdbserver {

    /**
     * @brief Start the GDB server
     * @param emulator The emulator to use
     * @param port The port to listen on
    */
    void start(irve::internal::emulator::emulator_t& emulator, uint16_t port);

}

#endif//GDBSERVER_H
