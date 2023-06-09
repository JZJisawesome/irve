/**
 * @file    CSR.h
 * @brief   Contains (mostly) CSRs for IRVE as well as methods to read and write them.
 * 
 * @copyright Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 * 
 * TODO longer description
 *
*/

#ifndef CSR_H
#define CSR_H

/* ------------------------------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------------------------- */

#ifdef private//Unit tests define this but this dosn't play nicely with chrono
#undef private
#include <chrono>
#define private public
#else
#include <chrono>
#endif

#include <cstdint>

#include "common.h"
#include "rvexception.h"

/* ------------------------------------------------------------------------------------------------
 * Constants/Defines
 * --------------------------------------------------------------------------------------------- */

namespace irve::internal::CSR {

/**
 * @brief CSR addresses implemented by IRVE
*/
namespace address {
    const uint16_t SSTATUS              = 0x100;
    const uint16_t SIE                  = 0x104;
    const uint16_t STVEC                = 0x105;
    const uint16_t SCOUNTEREN           = 0x106;
    const uint16_t SENVCFG              = 0x10A;
    const uint16_t SSCRATCH             = 0x140;
    const uint16_t SEPC                 = 0x141;
    const uint16_t SCAUSE               = 0x142;
    const uint16_t STVAL                = 0x143;
    const uint16_t SIP                  = 0x144;
    const uint16_t SATP                 = 0x180;
    const uint16_t MSTATUS              = 0x300;
    const uint16_t MISA                 = 0x301;
    const uint16_t MEDELEG              = 0x302;
    const uint16_t MIDELEG              = 0x303;
    const uint16_t MIE                  = 0x304;
    const uint16_t MTVEC                = 0x305;
    const uint16_t MCOUNTEREN           = 0x306;
    const uint16_t MENVCFG              = 0x30A;
    const uint16_t MSTATUSH             = 0x310;
    const uint16_t MENVCFGH             = 0x31A;
    const uint16_t MCOUNTINHIBIT        = 0x320;

    const uint16_t MHPMEVENT_START      = 0x323;//Inclusive
    const uint16_t MHPMEVENT_END        = 0x33F;//Inclusive

    const uint16_t MSCRATCH             = 0x340;
    const uint16_t MEPC                 = 0x341;
    const uint16_t MCAUSE               = 0x342;
    const uint16_t MTVAL                = 0x343;
    const uint16_t MIP                  = 0x344;
    const uint16_t MTINST               = 0x34A;
    const uint16_t MTVAL2               = 0x34B;
    
    const uint16_t PMPCFG_START         = 0x3A0;//Inclusive
    const uint16_t PMPCFG_END           = 0x3AF;//Inclusive
    const uint16_t PMPADDR_START        = 0x3B0;//Inclusive
    const uint16_t PMPADDR_END          = 0x3EF;//Inclusive
    
    const uint16_t MCYCLE               = 0xB00;
    const uint16_t MINSTRET             = 0xB02;

    const uint16_t MHPMCOUNTER_START    = 0xB03;//Inclusive
    const uint16_t MHPMCOUNTER_END      = 0xB1F;//Inclusive

    const uint16_t MCYCLEH              = 0xB80;
    const uint16_t MINSTRETH            = 0xB82;

    const uint16_t MHPMCOUNTERH_START   = 0xB83;//Inclusive
    const uint16_t MHPMCOUNTERH_END     = 0xB9F;//Inclusive

    const uint16_t MTIME                = 0xBC0;//Custom
    const uint16_t MTIMEH               = 0xBC4;//Custom
    const uint16_t MTIMECMP             = 0xBD0;//Custom
    const uint16_t MTIMECMPH            = 0xBD4;//Custom

    const uint16_t CYCLE                = 0xC00;
    const uint16_t TIME                 = 0xC01;
    const uint16_t INSTRET              = 0xC02;

    const uint16_t HPMCOUNTER_START     = 0xC03;//Inclusive
    const uint16_t HPMCOUNTER_END       = 0xC1F;//Inclusive

    const uint16_t CYCLEH               = 0xC80;
    const uint16_t TIMEH                = 0xC81;
    const uint16_t INSTRETH             = 0xC82;

    const uint16_t HPMCOUNTERH_START    = 0xC83;//Inclusive
    const uint16_t HPMCOUNTERH_END      = 0xC9F;//Inclusive

    const uint16_t MVENDORID            = 0xF11;
    const uint16_t MARCHID              = 0xF12;
    const uint16_t MIMPID               = 0xF13;
    const uint16_t MHARTID              = 0xF14;
    const uint16_t MCONFIGPTR           = 0xF15;
};

/* ------------------------------------------------------------------------------------------------
 * Type/Class Declarations
 * --------------------------------------------------------------------------------------------- */

/**
 * @brief Privilege modes
*/
enum class privilege_mode_t : uint8_t {
    USER_MODE       = 0b00,
    SUPERVISOR_MODE = 0b01,
    MACHINE_MODE    = 0b11
};

/**
 * @brief Class containing RISC-V CSRs
*/
class CSR_t {
public:
    /**
     * @brief Default CSR_t constructor. Only guaranteed to initialize CSRs that must be
     * according to the RISC-V spec.
    */
    CSR_t();

    /**
     * @brief Reads a CSR explictly (checking for adequate privilege and readablity)
     * @param csr The CSR to read
     * @return The value of the CSR
    */
    reg_t explicit_read(uint16_t csr) const;//Performs privilege checks

    /**
     * @brief Writes a CSR explictly (checking for adequate privilege and writability)
     * @param csr The CSR to write
     * @param data The data to write to the CSR
    */
    void explicit_write(uint16_t csr, word_t data);//Performs privilege checks

    /**
     * @brief Reads a CSR implicitly (without checking privilege; still checks readablity)
     * @param csr The CSR to read
     * @return The value of the CSR
    */
    reg_t implicit_read(uint16_t csr) const;//Does not perform privilege checks

    /**
     * @brief Writes a CSR implicitly (without checking privilege; still checks writability)
     * @param csr The CSR to write
     * @param data The data to write to the CSR
    */
    void implicit_write(uint16_t csr, word_t data);//Does not perform privilege checks

    /**
     * @brief Sets the privilege mode of the RISC-V CPU
     * @param new_privilege_mode The new privilege mode to use
    */
    void set_privilege_mode(privilege_mode_t new_privilege_mode);

    /**
     * @brief Gets the privilege mode of the RISC-V CPU
     * @return The current privilege mode
    */
    privilege_mode_t get_privilege_mode() const;

    /**
     * @brief Updates the RISC-V CPU's mtime timer; may also set a timer interrupt as pending
     * in the mip CSR
    */
    void update_timer();
private:
    /**
     * @brief Checks if the current privilege mode can read a CSR
     * @param csr The CSR to check
     * @return True if the current privilege mode can read the CSR, false otherwise
    */
    bool current_privilege_mode_can_explicitly_read(uint16_t csr) const;

    /**
     * @brief Checks if the current privilege mode can write a CSR
     * @param csr The CSR to check
     * @return True if the current privilege mode can write the CSR, false otherwise
    */
    bool current_privilege_mode_can_explicitly_write(uint16_t csr) const;

    reg_t sie;
    reg_t stvec;
    reg_t scounteren;
    reg_t senvcfg;
    reg_t sscratch;
    reg_t sepc;
    reg_t scause;
    //stval is NOT here
    reg_t sip;
    reg_t satp;
    reg_t mstatus;
    //misa is NOT here
    reg_t medeleg;
    reg_t mideleg;
    reg_t mie;
    reg_t mtvec;
    reg_t menvcfg;
    //mstatush is NOT here
    //menvcfgh is NOT here
    reg_t mscratch;
    reg_t mepc;
    reg_t mcause;
    //mtval is NOT here
    reg_t mip;

    reg_t pmpcfg[64];
    reg_t pmpaddr[64];

    uint64_t minstret;//Handles both minstret and minstreth
    uint64_t mcycle;//Handles both mcycle and mcycleh

    //NOTE: According to the spec, mtime and mtimecmp must be in memory, not in CSRs
    //However, that would mean CSR_t needs a reference to memory, which is not ideal
    //So instead we keep them here, and memory will have to redirect writes to their addresses into implicit writes to these CSRs
    uint64_t mtime;//Handles both time and timeh
    uint64_t mtimecmp;//Handles both time and timeh
    std::chrono::time_point<std::chrono::steady_clock> m_last_time_update;
    uint16_t m_delay_update_counter;//Don't check how much time has passed each tick() (much too slow)

    privilege_mode_t m_privilege_mode;//Not a CSR, but it is a register we need to access to determine if we can access a CSR (and it is also used in other places)
};

}

#endif
