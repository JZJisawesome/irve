/**
 * @file    decode.h
 * @brief   Code to decode RISC-V instructions
 * 
 * @copyright Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 * 
 * TODO longer description
 *
 * Based on code from rv32esim
 *
*/

#ifndef DECODE_H
#define DECODE_H

/* ------------------------------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------------------------- */

#include <cstdint>
#include <string>

#include "common.h"

/* ------------------------------------------------------------------------------------------------
 * Type/Class Declarations
 * --------------------------------------------------------------------------------------------- */

namespace irve::internal::decode {

/**
 * @brief RISC-V opcodes
 * @note If you change this, you must also change things on the Rust side
*/
enum class opcode_t : uint8_t{
    LOAD = 0b00000      , LOAD_FP = 0b00001   , CUSTOM_0 = 0b00010  , MISC_MEM = 0b00011  ,
    OP_IMM = 0b00100    , AUIPC = 0b00101     , OP_IMM_32 = 0b00110 , B48_0 = 0b00111     ,
    STORE = 0b01000     , STORE_FP = 0b01001  , CUSTOM_1 = 0b01010  , AMO = 0b01011       ,
    OP = 0b01100        , LUI = 0b01101       , OP_32 = 0b01110     , B64 = 0b01111       ,
    MADD = 0b10000      , MSUB = 0b10001      , NMSUB = 0b10010     , NMADD = 0b10011     ,
    OP_FP = 0b10100     , RESERVED_0 = 0b10101, CUSTOM_2 = 0b10110  , B48_1 = 0b10111     ,
    BRANCH = 0b11000    , JALR = 0b11001      , RESERVED_1 = 0b11010, JAL = 0b11011       ,
    SYSTEM = 0b11100    , RESERVED_3 = 0b11101, CUSTOM_3 = 0b11110  , BGE80 = 0b11111,
};

/**
 * @brief RISC-V instruction formats
 * @note If you change this, you must also change things on the Rust side
*/
enum class inst_format_t {
    R_TYPE = 0, I_TYPE = 1, S_TYPE = 2, B_TYPE = 3, U_TYPE = 4, J_TYPE = 5
};

/**
 * @brief holds the results from decoding a RISC-V instruction
 * @note We are NOT supporting compressed instructions
*/
class decoded_inst_t {
public:

    /**
     * @brief The constructor, decodes an instruction
     * @param instruction The instruction to decode
    */
    decoded_inst_t(word_t instruction);

    decoded_inst_t() = default;//FIXME remove this (needed for icache)

    decoded_inst_t(const decoded_inst_t& other) = default;
    decoded_inst_t& operator=(const decoded_inst_t& other) = default;

    /**
     * @brief TODO
     * @param indent TODO
     * @param inst_count TODO
    */
    void log(uint8_t indent, uint64_t inst_count) const;

    /**
     * @brief TODO
     * @return TODO
     * 
     * Check this before using the other getters. The getters will assert that the fields actually
     * exist in the format of the instruction
    */
    inst_format_t get_format() const;

    opcode_t get_opcode() const;
    uint8_t get_funct3() const;
    uint8_t get_funct5() const;
    uint8_t get_funct7() const;
    uint8_t get_rd() const;
    uint8_t get_rs1() const;//Also uimm//TODO how should we expose uimm?
    uint8_t get_rs2() const;
    word_t get_imm() const;

private:
    std::string disassemble() const;
    opcode_t m_opcode;//Bits [6:2]

    uint8_t m_funct3;
    uint8_t m_funct5;
    uint8_t m_funct7;
    uint8_t m_rd;
    uint8_t m_rs1;
    uint8_t m_rs2;
    word_t m_imm_I;
    word_t m_imm_S;
    word_t m_imm_B;
    word_t m_imm_U;
    word_t m_imm_J;

    inst_format_t m_format;
};

}

#endif//DECODE_H
