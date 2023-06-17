/* memory.hpp
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * A class for the memory of the emulator
 *
 * Manages virtual memory and physical memory
 * 
*/

#ifndef MEMORY_H
#define MEMORY_H

/* Constants And Defines */

//TODO put these into a namespace as regular C++ constants

// The directory where test files are located
#define TESTFILES_DIR   "testfiles/compiled/"

#define BYTE_MASK       0xFF

#define DATA_WIDTH_MASK 0b11

// Emulator memory size is 64 MiB
#define RAMSIZE         0x0400000

// RISC-V code that writes a series of bytes to this address will print them to stdout (flushed
// when a newline is encountered)
#define DEBUGADDR       0xFFFFFFFF

// Virtual address translation

// A RISC-V page size is 4 KiB
#define PAGESIZE        0x1000

// The physical page number field of the satp CSR
#define satp_PPN        m_CSR_ref.implicit_read(0x180).bits(21, 0).u

// The virtual page number (VPN) of a virtual address (va)
#define va_VPN(va, i)   va.bits(21 + 10 * i, 12 + 10 * i).u

// Page is writable bit
#define pte_W(pte)      pte.bit(2).u
// Page is readable bit
#define pte_R(pte)      pte.bit(1).u
// Page table entry valid bit
#define pte_V(pte)      pte.bit(0).u

// Access types
#define AT_INSTRUCTION  0
#define AT_LOAD         1
#define AT_STORE        2

#define PAGE_FAULT_BASE 12


/* Includes */

#include <iostream>
#include <memory>

#include "common.h"
#include "CSR.h"

/* Function/Class Declarations */

namespace irve::internal::memory {

    /**
     * @brief A wrapper for physical memory
     * 
     * Facilitates address translation, memory protection, and loading the memory image file
    */
    class memory_t {
    public:

        /**
         * @brief The constructor
         * @param imagec TODO
         * @param imagev TODO
         * @param CSR_ref A reference to the CSRs
        */
        memory_t(int imagec, char** imagev, CSR::CSR_t& CSR_ref);

        /**
         * @brief Fetch instruction from memory (implicit read)
         * @param addr The address to fetch from (physical or virtual depending on operating mode)
         * @return The instruction from memory
        */
        word_t instruction(word_t addr);

        /**
         * @brief Load data from memory
         * @param addr The address to load from (physical or virtual depending on operating mode)
         * @param data_type From funct3 of memory instructions, specifies data width and
         *                  signed/unsigned
         * @return The data read from memory
        */
        word_t load(word_t addr, uint8_t data_type);

        /**
         * @brief Store data to memory
         * @param addr The address to write to (physical or virtual depending on operating mode)
         * @param data_type From funct3 of memory instructions, specifies data width and
         *                  signed/unsigned
         * @param data The data to be stored in memory
        */
        void store(word_t addr, uint8_t data_type, word_t data);

    private:

        /**
         * @brief Translates a 32 bit address to a 34 bit machine address
         * @param untranslated_address 32 bit address
         * @param access_type TODO
         * @return 34 bit machine address
        */
        uint64_t translate_address(word_t untranslated_addr, uint8_t access_type) const;

        /**
         * @brief Read the specified data type from memory
         * @param addr 34 bit machine address
         * @param data_type From funct3 of memory instructions, specifies data width and
         *                  signed/unsigned
         * @return 32 bit version of data that was read
        */
        word_t read_physical(uint64_t addr, uint8_t data_type) const;

        /**
         * @brief Write data to memory
         * @param addr 34 bit machine address
         * @param data_type TODO
         * @param data TODO
        */
        void write_physical(uint64_t addr, uint8_t data_type, word_t data);

        /**
         * @brief TODO
         * @param imagec TODO
         * @param imagev TODO
        */
        void load_memory_image_files(int imagec, char** imagev);

        /**
         * @brief TODO
         * @param image_path TODO
        */
        void load_verilog_32(std::string image_path);

        /**
         * @brief "physical" memory instance
        */
        pmemory_t m_mem;

        /**
         * @brief Reference to the CSRs since memory operations depend on them
        */
        CSR::CSR_t& m_CSR_ref;
    };

    /**
     * @brief Stores the 'physical' memory and handles memory mapped IO
    */
    class pmemory_t {
    public:

        /**
         * @brief The constructor
        */
        pmemory_t();

        /**
         * @brief The destructor
        */
        ~pmemory_t();

        //TODO if/when we implement PMP, this will need an m_CSR_ref too

        /**
         * @brief Read a byte from memory & throw exception if the byte is not readable
         * @param addr 34 bit machine address
         * @return The byte read
        */
        uint8_t read_byte(uint64_t addr) const;

        /**
         * @brief Write a byte to memory
         * @param addr 34 bit machine address
         * @param data The data to be written
         * This function does NOT raise exceptions if the byte is not writable. To verify that the
         * byte is writable, `check_writable_byte` should always be called first.
        */
        void write_byte(uint64_t addr, uint8_t data);

        /**
         * @brief Throws an exception if the byte is not writable
         * @param addr 34 bit machine address
         * This function should always be used to check that a byte is writable before writing to
         * the byte since `write_byte` assumes the byte is writable.
        */
       void check_writable_byte(uint64_t addr);

    private:

        /**
         * @brief Pointer to the ram array
        */
        std::unique_ptr<uint8_t[]> m_ram;

        /**
         * @brief TODO
        */
        std::string m_debugstr;
    };

}

#endif//MEMORY_H
