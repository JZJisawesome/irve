/**
 * @file    uart.h
 * @brief   16550 UART implementation
 * 
 * @copyright Copyright (C) 2023 John Jekel and Nick Chan
 * See the LICENSE file at the root of the project for licensing info.
 * 
 * TODO longer description
 *
*/

#ifndef UART_H
#define UART_H

/* ------------------------------------------------------------------------------------------------
 * Includes
 * --------------------------------------------------------------------------------------------- */

#include <cstdint>
#include <string>

/**
 * @brief The namespace containing the IRVE's UART implementation
*/
namespace irve::internal::uart {

/* ------------------------------------------------------------------------------------------------
 * Constants/Defines
 * --------------------------------------------------------------------------------------------- */

    namespace address {
        const uint8_t RHR = 0b000;//Receive Holding Register
        const uint8_t THR = 0b000;//Transmit Holding Register
        const uint8_t IER = 0b001;//Interrupt Enable Register
        const uint8_t ISR = 0b010;//Interrupt Status Register
        const uint8_t FCR = 0b010;//FIFO Control Register
        const uint8_t LCR = 0b011;//Line Control Register
        const uint8_t MCR = 0b100;//Modem Control Register
        const uint8_t LSR = 0b101;//Line Status Register
        const uint8_t MSR = 0b110;//Modem Status Register
        const uint8_t SPR = 0b111;//Scratch Pad Register

        //When LCR.DLAP == 1
        const uint8_t DLL = 0b000;//Divisor Latch LSB
        const uint8_t DLM = 0b001;//Divisor Latch MSB
        const uint8_t PSD = 0b101;//Prescaler Division
    }

/* ------------------------------------------------------------------------------------------------
 * Type/Class Declarations
 * --------------------------------------------------------------------------------------------- */

    /**
     * @brief The IRVE 16550 UART
    */
    class uart_t {
    public:
        uart_t();
        ~uart_t();
        
        //3-bit addresses, 8-bit data
        uint8_t read(uint8_t register_address);//NOT const since it could pop from the FIFO
        void write(uint8_t register_address, uint8_t data);

        bool interrupt_pending() const;//More convenient than reading ISR and checking bits
    private:
        bool dlab() const;

        //No need for rhr and thr since they just go directly to stdin/stdout
        //uint8_t m_ier;//Interrupt Enable Register
        uint8_t m_isr;//Interrupt Status Register
        //uint8_t m_fcr;//FIFO Control Register
        uint8_t m_lcr;//Line Control Register
        //uint8_t m_mcr;//Modem Control Register
        //uint8_t m_lsr;//Line Status Register
        //uint8_t m_msr;//Modem Status Register
        uint8_t m_spr;//Scratch Pad Register

        //Note: We expose these registers, but we completely ignore their contents
        //since the serial output is the terminal and there are no real "wires" to
        //run at a particular baud rate
        uint8_t m_dll;//Divisor Latch LSB
        uint8_t m_dlm;//Divisor Latch MSB
        uint8_t m_psd;//Prescaler Division

        std::string m_output_line_buffer;
    };
}

#endif//UART_H
