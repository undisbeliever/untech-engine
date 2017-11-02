// interrupts/nmi_handler.inc
// ==========================
//
// VBlank Interrupt Service Routine.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


au()
iu()
code()
function NmiHandler {
    // Store registers
    rep     #$30
a16()
i16()
    pha
    phx
    phy
    phd
    phb

    phk
    plb

    lda.w   #$4300
    tcd

    sep     #$10
i8()

    // Reset NMI Flag.
    ldy.w   RDNMI

    // Disable HDMA
    ldx.b   #0
    stx.w   HDMAEN


    Dma.VBlank()
    Hdma.VBlank()

    Warnings.VBlank()
    ScrollBuffer.VBlank()
    FrameCounter.VBlank()
    Controller.VBlank()

    InidispBuffer.VBlank()


    rep     #$30

    plb
    pld
    ply
    plx
    pla

    rti
}

// vim: ft=bass-65816 ts=4 sw=4 et:

