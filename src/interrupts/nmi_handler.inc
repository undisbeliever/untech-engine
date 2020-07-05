// interrupts/nmi_handler.inc
// ==========================
//
// NMI Interrupt Service Routine.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


au()
iu()
code(interrupt_code)
function NmiHandler {
    // ensure NmiHandler is accessible by interrupt vectors
    assert((pc() & 0x3fffff) >= 0x008000 && (pc() & 0x3fffff) < 0x00ffff)

// Number of NMIs processed since the counter was last reset
// (byte)
allocate(counter, shadow, 1)

    // Store registers
    sep     #$20
a8()
    pha

    lda.l   counter
    inc
    sta.l   counter

    lda.l   RDNMI

    pla
    rti
}

// vim: ft=bass-65816 ts=4 sw=4 et:

