/**
 * @file    emulator.cpp
 * @brief   The main emulator class used to instantiate an instance of irve
 * 
 * @copyright Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 * 
 * TODO longer description
 *
*/

/* ------------------------------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------------------------- */

#include "emulator.h"

#include <cassert>
#include <cstdint>

#include "common.h"
#include "cpu_state.h"
#include "CSR.h"
#include "decode.h"
#include "gdbserver.h"
#include "execute.h"
#include "memory.h"
#include "rvexception.h"

#define INST_COUNT this->m_CSR.implicit_read(CSR::address::MINSTRET).u
#include "logging.h"

using namespace irve::internal;

/* ------------------------------------------------------------------------------------------------
 * Function Implementations
 * --------------------------------------------------------------------------------------------- */

emulator::emulator_t::emulator_t(int imagec, const char* const* imagev):
        m_CSR(),
        m_memory(imagec, imagev, m_CSR),
        m_cpu_state(m_CSR),
        m_intercept_breakpoints(false) {
    irvelog(0, "Created new emulator instance");
}

bool emulator::emulator_t::tick() {
    this->m_CSR.implicit_write(CSR::address::MINSTRET, this->m_CSR.implicit_read(CSR::address::MINSTRET) + 1);
    this->m_CSR.implicit_write(CSR::address::MCYCLE,   this->m_CSR.implicit_read(CSR::address::MCYCLE  ) + 1);
    irvelog(0, "Tick %lu begins", this->get_inst_count());

    //Any of these could lead to exceptions (ex. faults, illegal instructions, etc.)
    try {
        decode::decoded_inst_t decoded_inst = this->fetch_and_decode();
        this->execute(decoded_inst);
    } catch (const rvexception::rvexception_t& e) {
        uint32_t raw_cause = (uint32_t)e.cause();
        assert((raw_cause < 32) && "Unsuppored cause value!");//Makes it simpler since this means we must check medeleg always
        irvelog(1, "Handling exception: Cause: %u", raw_cause);
        this->handle_trap(e.cause());
    } catch (const rvexception::irve_exit_request_t&) {
        irvelog(0, "Recieved exit request from emulated guest");
        return false;
    }

    this->m_CSR.update_timer();

    //TODO also add a function to memory to update peripherals (they will set the relevant pending bit if needed, or we could have them return a bool indicating if we should set the external interrupt pending bit here)

    this->check_and_handle_interrupts();

    irvelog(0, "Tick %lu ends", this->get_inst_count());
    return true;
}

void emulator::emulator_t::run_until(uint64_t inst_count) {
    if (inst_count) {
        //Run until the given instruction count is reached or an exit request is made
        while ((this->get_inst_count() < inst_count) && this->tick());
    }
    else {
        //The only exit criteria is an exit request
        while (this->tick());
    }
}

void emulator::emulator_t::run_gdbserver(uint16_t port) {
    this->m_intercept_breakpoints = true;
    this->m_encountered_breakpoint = false;
    gdbserver::start(*this, this->m_cpu_state, this->m_memory, port);
}

uint64_t emulator::emulator_t::get_inst_count() const {
    return INST_COUNT;
}

bool emulator::emulator_t::test_and_clear_breakpoint_encountered_flag() {
    bool breakpoint_encountered = this->m_encountered_breakpoint;
    this->m_encountered_breakpoint = false;
    return breakpoint_encountered;
}

void emulator::emulator_t::flush_icache() {
    this->m_icache.clear();
}

decode::decoded_inst_t emulator::emulator_t::fetch_and_decode() {
    word_t pc = this->m_cpu_state.get_pc();
    irvelog(1, "Fetching from 0x%08x", pc);

    //Note: Using exceptions instead to catch misses is (very slightly) faster when using the same few instructions over and over again
    //(ex in nouveau_stress_test). But it tanks perf in other scenarios. So we do compare-and-branch instead
    if (this->m_icache.contains(pc.u)) {
        irvelog(1, "Cache hit");
        return this->m_icache.at(pc.u);
    } else {
        irvelog(1, "Cache miss");

        //Read a word from memory at the PC
        //NOTE: It may throw an exception for various reasons
        word_t inst = this->m_memory.instruction(pc);

        //Log what we fetched and return it
        irvelog(1, "Fetched 0x%08x from 0x%08x", inst, pc);

        irvelog(1, "Decoding instruction 0x%08X", inst);
        decode::decoded_inst_t decoded_inst(inst);
        decoded_inst.log(2, this->get_inst_count());

        //TODO be more fine-grained about this
        //Note the icache is cleared on exceptions and interrupts, so that is already handled
        if (decoded_inst.get_opcode() == decode::opcode_t::MISC_MEM) {//To catch FENCE.i
            this->flush_icache();
        } else if (decoded_inst.get_opcode() == decode::opcode_t::SYSTEM) {//To catch satp changes, SFENCE.VMA
            this->flush_icache();
        } else {//There is no need to clear the cache
            this->m_icache.emplace(pc.u, decoded_inst);
        }

        return decoded_inst;
    }
}

//TODO move this to a separate file maybe?
void emulator::emulator_t::execute(const decode::decoded_inst_t &decoded_inst) {
    irvelog(1, "Executing instruction");

    //We can assume the opcode exists since the instruction is valid
    switch (decoded_inst.get_opcode()) {
        case decode::opcode_t::LOAD:
            assert((decoded_inst.get_format() == decode::inst_format_t::I_TYPE) && "Instruction with LOAD opcode had a non-I format!");
            execute::load(decoded_inst, this->m_cpu_state, this->m_memory, this->m_CSR);
            break;
        case decode::opcode_t::CUSTOM_0:
            assert((decoded_inst.get_format() == decode::inst_format_t::R_TYPE) && "Instruction with CUSTOM_0 opcode had a non-R format!");
            execute::custom_0(decoded_inst, this->m_cpu_state, this->m_memory, this->m_CSR);
            break;
        case decode::opcode_t::MISC_MEM:
            assert((decoded_inst.get_format() == decode::inst_format_t::I_TYPE) && "Instruction with MISC_MEM opcode had a non-I format!");
            execute::misc_mem(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::OP_IMM:
            assert((decoded_inst.get_format() == decode::inst_format_t::I_TYPE) && "Instruction with OP_IMM opcode had a non-I format!");
            execute::op_imm(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::AUIPC:
            assert((decoded_inst.get_format() == decode::inst_format_t::U_TYPE) && "Instruction with AUIPC opcode had a non-U format!");
            execute::auipc(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::STORE:
            assert((decoded_inst.get_format() == decode::inst_format_t::S_TYPE) && "Instruction with STORE opcode had a non-S format!");
            execute::store(decoded_inst, this->m_cpu_state, this->m_memory, this->m_CSR);
            break;
        case decode::opcode_t::AMO:
            //TODO assertion
            execute::amo(decoded_inst, this->m_cpu_state, this->m_memory, this->m_CSR);
            break;
        case decode::opcode_t::OP:
            assert((decoded_inst.get_format() == decode::inst_format_t::R_TYPE) && "Instruction with OP opcode had a non-R format!");
            execute::op(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::LUI:
            assert((decoded_inst.get_format() == decode::inst_format_t::U_TYPE) && "Instruction with LUI opcode had a non-U format!");
            execute::lui(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::BRANCH:
            assert((decoded_inst.get_format() == decode::inst_format_t::B_TYPE) && "Instruction with BRANCH opcode had a non-B format!");
            execute::branch(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::JALR:
            assert((decoded_inst.get_format() == decode::inst_format_t::I_TYPE) && "Instruction with JALR opcode had a non-I format!");
            execute::jalr(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::JAL:
            assert((decoded_inst.get_format() == decode::inst_format_t::J_TYPE) && "Instruction with JAL opcode had a non-J format!");
            execute::jal(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        case decode::opcode_t::SYSTEM:
            assert((decoded_inst.get_format() == decode::inst_format_t::I_TYPE) && "Instruction with SYSTEM opcode had a non-I format!");
            execute::system(decoded_inst, this->m_cpu_state, this->m_CSR);
            break;
        default:
            assert(false && "Instruction with either invalid opcode, or that is implemented in decode but not in execute yet!");
            break;
    }
}

void emulator::emulator_t::check_and_handle_interrupts() {
    //Section 3.1.6.1 describes when interrupts are globally enabled
    //We will never be interrupted to a lower privilege level if we are currently executing in a higher privilege level
    //Ex. if we're in M-mode, interrupts delegated to S-mode will never be triggered (will just stay pending)
    //However it does work the other way around: if we're in S-mode or U-mode, interrupts delegated to M-mode will be triggered
    //
    //Now let's consider MIE and SIE.
    //Critically these ONLY TAKE AFFECT IF WE'RE IN THE CORRESPONDING PRIVILEGE MODE
    //So if we're in M-mode and MIE is clear, we will never be interrupted
    //But if we're in S-mode and MIE is clear, we will still be interrupted to M-mode
    //This is similar for SIE: if we're in S-mode and SIE is clear, we will never be interrupted, but if we're in U-mode and SIE is clear, we will still be interrupted to S-mode
    //
    //Finally, an interupts only occurs if it is pending (mip/sip) and enabled (mie/sie)
    //
    //Note: 4.1.3 may also be useful

    irvelog(1, "Checking for interrupts...");

    reg_t mstatus = this->m_CSR.implicit_read(CSR::address::MSTATUS);
    bool in_m_mode = this->m_CSR.get_privilege_mode() == CSR::privilege_mode_t::MACHINE_MODE;
    bool in_s_mode = this->m_CSR.get_privilege_mode() == CSR::privilege_mode_t::SUPERVISOR_MODE;
    
    if (in_m_mode) {
        if (!mstatus.bit(3)) {//mstatus.MIE is not set, so since we're in M-mode, this means interrupts are disabled
            irvelog(0, "Interrupts are currently disabled in M-mode");
            return;
        }
    } else if (in_s_mode) {
        if (!mstatus.bit(1)) {//mstatus.SIE is not set, so since we're in S-mode, this means interrupts are disabled
            irvelog(0, "Interrupts are currently disabled in S-mode");
            return;
        }
    }
    //If we reach this point, interrupts aren't globally disabled, so we need to check for them

    //
    //Check if any interrupts are "interrupting", and choose the one with the highest priority
    //

    //Get mip and sie. NOTE: We don't need to read sip and sie, since those are just shadows for S-mode code to use
    reg_t mip = this->m_CSR.implicit_read(CSR::address::MIP);
    reg_t mie = this->m_CSR.implicit_read(CSR::address::MIE);

    //Also mideleg will be useful
    reg_t mideleg = this->m_CSR.implicit_read(CSR::address::MIDELEG);

    //Helper lambda. Returns true if the given bit is "interrupting". Aka, that...
    //1. The interrupt is pending (mip/sip)
    //2. The interrupt is enabled (mie/sie)
    //3. The interrupt is delegated to the current or higher privilege level (mideleg/sideleg)
    //Also sets `delegated_to_smode` properly for later use
    auto is_interrupting = [&](uint8_t bit) {
        assert((bit < 32) && "Invalid is_interrupting() bit!");
        bool pending = mip.bit(bit) == 1;
        bool enabled = mie.bit(bit) == 1;

        //The only time we'd ignore an interrupt is if we're in M-mode and it's delegated to S-mode
        bool delegated_to_smode = mideleg.bit(bit) == 1;
        bool ignored_in_current_mode = (in_m_mode && delegated_to_smode);

        bool interrupting = pending && enabled && !ignored_in_current_mode;
        return interrupting;
    };

    //According to the spec, the order of priority is: MEI, MSI, MTI, SEI, SSI, STI
    rvexception::cause_t cause;
    if        (is_interrupting(11)) {//MEI
        cause = rvexception::cause_t::MACHINE_EXTERNAL_INTERRUPT;
    } else if (is_interrupting( 3)) {//MSI
        cause = rvexception::cause_t::MACHINE_SOFTWARE_INTERRUPT;
    } else if (is_interrupting( 7)) {//MTI
        cause = rvexception::cause_t::MACHINE_TIMER_INTERRUPT;
    } else if (is_interrupting( 9)) {//SEI
        cause = rvexception::cause_t::SUPERVISOR_EXTERNAL_INTERRUPT;
    } else if (is_interrupting( 1)) {//SSI
        cause = rvexception::cause_t::SUPERVISOR_SOFTWARE_INTERRUPT;
    } else if (is_interrupting( 5)) {//STI
        cause = rvexception::cause_t::SUPERVISOR_TIMER_INTERRUPT;
    } else {
        irvelog(1, "No interrupts \"interrupting\" at this time.");
        return;
    }
    //If we make it here, we have an interrupt to handle (specifically the one in `cause)

    irvelog(1, "Handling interrupt: Cause: %u", (uint32_t)cause);
    this->handle_trap(cause);
}

void emulator::emulator_t::handle_trap(rvexception::cause_t cause) {
    this->flush_icache();

    //TODO better logging

    //We do this to support debugging with irvegdb. This will cause issues if anything other than GDB puts an EBREAK instruction in memory
    if (this->m_intercept_breakpoints && (cause == rvexception::cause_t::BREAKPOINT_EXCEPTION) && (this->m_CSR.get_privilege_mode() != CSR::privilege_mode_t::USER_MODE)) {
        irvelog(1, "Breakpoint intercepted");
        this->m_encountered_breakpoint = true;
        return;
    }

    this->m_cpu_state.invalidate_reservation_set();//Could have interrupted an LR/SC sequence

    word_t raw_cause = (uint32_t)cause;
    assert((raw_cause.bits(30, 0).u < 32) && "Unsupported cause!");

    bool is_interrupt = raw_cause.bit(31) == 1;

    bool handle_in_m_mode;
    if (is_interrupt) {//The cause was an interrupt
        //With interrupts, they don't "come" from a particular mode
        //We assume check_and_handle_interrupts() has already checked that the interrupt is "interrupting", so this should be enough
        bool interrupt_delegated_to_machine_mode = this->m_CSR.implicit_read(CSR::address::MIDELEG).bit(raw_cause.bits(30, 0)) == 0;
        handle_in_m_mode = interrupt_delegated_to_machine_mode;
    } else {//The cause was an exception
        bool exception_from_machine_mode = this->m_CSR.get_privilege_mode() == CSR::privilege_mode_t::MACHINE_MODE;
        bool exception_delegated_to_machine_mode = this->m_CSR.implicit_read(CSR::address::MEDELEG).bit(raw_cause) == 0;
        handle_in_m_mode = exception_from_machine_mode || exception_delegated_to_machine_mode;
    }

    if (handle_in_m_mode) {//Exception should be handled in machine mode
        //Manage the privilege stack
        word_t mstatus = this->m_CSR.implicit_read(CSR::address::MSTATUS);
        word_t mie = mstatus.bit(3);
        mstatus &= 0b11111111111111111110011101110111;//Clear the MPP, MPIE, and MIE bits
        mstatus |= ((uint32_t)this->m_CSR.get_privilege_mode()) << 11;//Set the MPP bits to the current privilege mode
        mstatus |= mie << 7;//Set MPIE to MIE
        //MIE is set to 0
        this->m_CSR.implicit_write(CSR::address::MSTATUS, mstatus);//Write changes back to the CSR
        this->m_CSR.set_privilege_mode(CSR::privilege_mode_t::MACHINE_MODE);

        //Write other CSRs to indicate information about the exception
        this->m_CSR.implicit_write(CSR::address::MCAUSE, (uint32_t) cause);
        this->m_CSR.implicit_write(CSR::address::MEPC, this->m_cpu_state.get_pc());
        this->m_CSR.implicit_write(CSR::address::MTVAL, 0);

        //Jump to the exception handler
        word_t mtvec = this->m_CSR.implicit_read(CSR::address::MTVEC);
        bool vectored = mtvec.bits(1, 0) == 0b01;
        if (vectored && is_interrupt) {
            this->m_cpu_state.set_pc(mtvec.bits(31, 2) + (raw_cause.bits(30, 0).u * 4));
        } else {
            this->m_cpu_state.set_pc(mtvec.bits(31, 2));
        }
    } else {//Exception should be handled by supervisor mode
        //Manage the privilege stack
        word_t sstatus = this->m_CSR.implicit_read(CSR::address::SSTATUS);
        word_t sie = sstatus.bit(1);
        sstatus &= 0b11111111111111111111111011011101;//Clear the SPP, SPIE, and SIE bits
        sstatus |= ((this->m_CSR.get_privilege_mode() == CSR::privilege_mode_t::SUPERVISOR_MODE) ? 0b1 : 0b0) << 8;//Set the SPP bit appropriately
        sstatus |= sie << 5;//Set SPIE to SIE
        //SIE is set to 0
        this->m_CSR.implicit_write(CSR::address::SSTATUS, sstatus);//Write changes back to the CSR
        this->m_CSR.set_privilege_mode(CSR::privilege_mode_t::SUPERVISOR_MODE);

        //Write other CSRs to indicate information about the exception
        this->m_CSR.implicit_write(CSR::address::SCAUSE, (uint32_t) cause);
        this->m_CSR.implicit_write(CSR::address::SEPC, this->m_cpu_state.get_pc());
        this->m_CSR.implicit_write(CSR::address::STVAL, 0);

        //Jump to the exception handler
        word_t stvec = this->m_CSR.implicit_read(CSR::address::STVEC);
        bool vectored = stvec.bits(1, 0) == 0b01;
        if (vectored && is_interrupt) {
            this->m_cpu_state.set_pc(stvec.bits(31, 2) + (raw_cause.bits(30, 0).u * 4));
        } else {
            this->m_cpu_state.set_pc(stvec.bits(31, 2));
        }
    }
}
