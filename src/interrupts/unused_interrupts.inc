// interrupts/unused_interrupts.inc
// ================================
//
// Interrupt Service Routines for the unused intrrupts.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// IRQ Interrupt ISR (does nothing)
au()
iu()
code(interrupt_code)
function IrqHandler {
    // Ensure code is accessible by interrupt vectors
    assert((pc() & 0x3fffff) >= 0x008000 && (pc() & 0x3fffff) < 0x00ffff)

    rti
}


// COP Interrupt ISR (does nothing)
au()
iu()
code(interrupt_code)
function CopHandler {
    // Ensure code is accessible by interrupt vectors
    assert((pc() & 0x3fffff) >= 0x008000 && (pc() & 0x3fffff) < 0x00ffff)

    rti
}


// Empty Interrupt ISR (does nothing)
au()
iu()
code(interrupt_code)
function EmptyHandler {
    // Ensure code is accessible by interrupt vectors
    assert((pc() & 0x3fffff) >= 0x008000 && (pc() & 0x3fffff) < 0x00ffff)

    rti
}

// vim: ft=bass-65816 ts=4 sw=4 et:

