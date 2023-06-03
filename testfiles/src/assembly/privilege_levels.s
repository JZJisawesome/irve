# privilege_levels.s
# Copyright (C) 2023 John Jekel and Nick Chan
# See the LICENSE file at the root of the project for licensing info.
#
# Testing out moving between privilege levels in assembly
#

.section .vector_table, "ax"
.global _start
_start:
    j mmode_entry
    j mmode_ecall_handler
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit

.text
mmode_entry:
    la a0, mmode_string0
    call print

    #Set mstatus so that when we execute mret, we will go to S-Mode
    li t0, 0b00000000000000000001100000000000
    li t1, 0b00000000000000000000100000000000
    csrrc zero, mstatus, t0#Clear MPP
    csrrs zero, mstatus, t1#Set MPP to 0b01 (S-Mode)

    #Set mepc to the address of the smode_entry label
    la t0, smode_entry
    csrrw zero, mepc, t0

    #Go to S-Mode
    mret

mmode_ecall_handler:
    la a0, mmode_string1
    call print
    .insn r CUSTOM_0, 0, 0, zero, zero, zero#Exit

smode_entry:
    la a0, smode_string
    call print
    ecall#Go back to M-Mode

print:#Expect address in a0
    lbu t0, 0(a0)
    beqz t0, 1f
    sb t0, -1(zero)
    addi a0, a0, 1
    j print
1:
    ret

mmode_string0:
.string "Hello from M-Mode!\n"
    
mmode_string1:
.string "Hello from M-Mode again!\n"

smode_string:
.string "Hello from S-Mode!\n"
