/* decode.c
 * Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 *
 * Code to decode RISC-V instructions
 *
 * Based on code from rv32esim
*/

//TODO transition to using word_t and reg_t

/* Includes */

#include "decode.h"

#include <cassert>
#include <cstdint>
#include <string>

#include "common.h"

#include "rvexception.h"

#define INST_COUNT inst_count
#include "logging.h"

/* Macros */

//TODO do this more cleanly (the word_t way)
#define SIGN_EXTEND_TO_32(data, num_bits) (((uint32_t)(((int32_t)((((uint32_t)data) << (32 - num_bits)) & (1ull << 31))) >> (31 - num_bits))) | ((uint32_t)data))

/* Function Implementations */

decoded_inst_t::decoded_inst_t(word_t instruction) :
    //TODO do this more cleanly (the word_t way)
    m_opcode((opcode_t) ((instruction.u >> 2) & 0b11111)),
    m_funct3((instruction.u >> 12) & 0b111),
    m_funct7((instruction.u >> 25) & 0b1111111),
    m_rd((instruction.u >> 7) & 0b11111),
    m_rs1((instruction.u >> 15) & 0b11111),
    m_rs2((instruction.u >> 20) & 0b11111),
    m_imm_I(SIGN_EXTEND_TO_32(instruction.u >> 20, 12)),
    m_imm_S(SIGN_EXTEND_TO_32(((instruction.u >> 20) & 0b111111100000) | ((instruction.u >> 7) & 0b11111), 12)),
    m_imm_B(SIGN_EXTEND_TO_32(((instruction.u >> 19) & 0b1000000000000) | ((instruction.u << 4) & 0b100000000000) | ((instruction.u >> 20) & 0b11111100000) | ((instruction.u >> 7) & 0b11110), 13)),
    m_imm_U(instruction.u & 0b11111111111111111111000000000000),
    m_imm_J(SIGN_EXTEND_TO_32(((instruction.u >> 11) & 0b100000000000000000000) | (instruction.u & 0b11111111000000000000) | ((instruction.u >> 9) & 0b100000000000) | ((instruction.u >> 20) & 0b11111111110), 21))
{
    //These are defined invalid RISC-V instructions
    //In addition, we don't support compressed instructions
    if (!instruction.u || (instruction.u == 0xFFFFFFFF) || ((instruction.u & 0b11) != 0b11)) {
        throw rvexception_t(false, ILLEGAL_INSTRUCTION_EXCEPTION);
    }

    switch (this->m_opcode) {
        //R-type
        case OP:
            this->m_format = R_TYPE;
            break;
        //I-type
        case LOAD:
        case OP_IMM:
        case JALR:
        case SYSTEM:
        case MISC_MEM:
            this->m_format = I_TYPE;
            break;
        //S-type
        case STORE:
            this->m_format = S_TYPE;
            break;
        //B-type
        case BRANCH:
            this->m_format = B_TYPE;
            break;
        //U-type
        case LUI:
        case AUIPC:
            this->m_format = U_TYPE;
            break;
        //J-type
        case JAL:
            this->m_format = J_TYPE;
            break;
        default:
            throw rvexception_t(false, ILLEGAL_INSTRUCTION_EXCEPTION);
            break;
    }
}

void decoded_inst_t::log(uint8_t indent, uint64_t inst_count) const {
    switch (this->get_format()) {
        case R_TYPE:
            irvelog(indent, "pretty = %s", this->disassemble().c_str());
            irvelog(indent, "type   = R");
            irvelog(indent, "opcode = 0x%X", this->get_opcode());
            irvelog(indent, "funct3 = 0x%X", this->get_funct3());
            irvelog(indent, "funct7 = 0x%X", this->get_funct7());
            irvelog(indent, "rd     = x%u", this->get_rd());
            irvelog(indent, "rs1    = x%u", this->get_rs1());
            irvelog(indent, "rs2    = x%u", this->get_rs2());
            break;
        case I_TYPE:
            irvelog(indent, "pretty = %s", this->disassemble().c_str());
            irvelog(indent, "type   = I");
            irvelog(indent, "opcode = 0x%X", this->get_opcode());
            irvelog(indent, "funct3 = 0x%X", this->get_funct3());
            irvelog(indent, "rd     = x%u", this->get_rd());
            irvelog(indent, "rs1    = x%u", this->get_rs1());
            irvelog(indent, "imm    = 0x%X", this->get_imm());
            break;
        case S_TYPE:
            irvelog(indent, "pretty = %s", this->disassemble().c_str());
            irvelog(indent, "type   = S");
            irvelog(indent, "opcode = 0x%X", this->get_opcode());
            irvelog(indent, "funct3 = 0x%X", this->get_funct3());
            irvelog(indent, "rs1    = x%u", this->get_rs1());
            irvelog(indent, "rs2    = x%u", this->get_rs2());
            irvelog(indent, "imm    = 0x%X", this->get_imm());
            break;
        case B_TYPE:
            irvelog(indent, "pretty = %s", this->disassemble().c_str());
            irvelog(indent, "type   = B");
            irvelog(indent, "opcode = 0x%X", this->get_opcode());
            irvelog(indent, "funct3 = 0x%X", this->get_funct3());
            irvelog(indent, "rs1    = x%u", this->get_rs1());
            irvelog(indent, "rs2    = x%u", this->get_rs2());
            irvelog(indent, "imm    = 0x%X", this->get_imm());
            break;
        case U_TYPE:
            irvelog(indent, "pretty = %s", this->disassemble().c_str());
            irvelog(indent, "type   = U");
            irvelog(indent, "opcode = 0x%X", this->get_opcode());
            irvelog(indent, "rd     = x%u", this->get_rd());
            irvelog(indent, "imm    = 0x%X", this->get_imm());
            break;
        case J_TYPE:
            irvelog(indent, "pretty = %s", this->disassemble().c_str());
            irvelog(indent, "type   = J");
            irvelog(indent, "opcode = 0x%X", this->get_opcode());
            irvelog(indent, "rd     = x%u", this->get_rd());
            irvelog(indent, "imm    = 0x%X", this->get_imm());
            break;
        default:
            assert(false && "We should never get here");
            break;
    }
}

inst_format_t decoded_inst_t::get_format() const {
    return this->m_format;
}

opcode_t decoded_inst_t::get_opcode() const {
    return this->m_opcode;
}

uint8_t decoded_inst_t::get_funct3() const {
    assert((this->get_format() != U_TYPE) && "Attempt to get funct3 of U-type instruction!");
    assert((this->get_format() != J_TYPE) && "Attempt to get funct3 of J-type instruction!");
    return this->m_funct3;
}

uint8_t decoded_inst_t::get_funct7() const {
    assert((this->get_format() != S_TYPE) && "Attempt to get funct7 of S-type instruction!");
    assert((this->get_format() != B_TYPE) && "Attempt to get funct7 of B-type instruction!");
    assert((this->get_format() != U_TYPE) && "Attempt to get funct7 of U-type instruction!");
    assert((this->get_format() != J_TYPE) && "Attempt to get funct7 of J-type instruction!");
    //We allow I-type in addition to R-type because it is useful for the immediate shift instructions
    return this->m_funct7;
}

uint8_t decoded_inst_t::get_rd() const {
    assert((this->get_format() != S_TYPE) && "Attempt to get rd of S-type instruction!");
    assert((this->get_format() != B_TYPE) && "Attempt to get rd of B-type instruction!");
    return this->m_rd;
}

uint8_t decoded_inst_t::get_rs1() const {
    assert((this->get_format() != U_TYPE) && "Attempt to get rs1 of U-type instruction!");
    assert((this->get_format() != J_TYPE) && "Attempt to get rs1 of J-type instruction!");
    return this->m_rs1;
}

uint8_t decoded_inst_t::get_rs2() const {
    assert((this->get_format() != I_TYPE) && "Attempt to get rs2 of I-type instruction!");
    assert((this->get_format() != U_TYPE) && "Attempt to get rs2 of U-type instruction!");
    assert((this->get_format() != J_TYPE) && "Attempt to get rs2 of J-type instruction!");
    return this->m_rs2;
}

word_t decoded_inst_t::get_imm() const {
    switch (this->get_format()) {
        case R_TYPE:
            assert(false && "Attempt to get imm of R-type instruction!");
            break;
        case I_TYPE:
            return this->m_imm_I;
            break;
        case S_TYPE:
            return this->m_imm_S;
            break;
        case B_TYPE:
            return this->m_imm_B;
            break;
        case U_TYPE:
            return this->m_imm_U;
            break;
        case J_TYPE:
            return this->m_imm_J;
            break;
        default:
            assert(false && "We should never get here");
            break;
    }
    return this->m_imm_I;
}

std::string decoded_inst_t::disassemble() const {
    return std::string("TODO disassembly of instruction here (Save this for XRVE actually)");//TODO
}
