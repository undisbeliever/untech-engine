// interrupts/nmi/inidisp-buffer.inc
// =================================
//
// VBlank buffer of the INIDISP register.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Buffer of the INIDISP register
// The MSB of this register should only be set by ForceBlank
// (byte - shadow)
allocate(inidispBuffer, shadow, 1)

namespace NmiHandler {
namespace InidispBuffer {

// Initialize the inidisp buffer
// MUST be called when resetting registers
// REQUITES: 8 bit A, DB unknown
macro Init() {
    assert8a()

    lda.b   #0x0f | INIDISP.force
    sta.l   inidispBuffer
}


// VBlank code, sets INIDISP register
//
// SHOULD BE called at the end of the NMI ISR.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank() {
    assert16a()
    assert8i()

    ldx.w   inidispBuffer
    stx.w   INIDISP
}
}
}


// Enables the display.
//
// The brightness of the display is set to the brightness bits
// of `inidispBuffer`
au()
iu()
code()
function EnableDisplay {
    php

    sep     #$20
a8()

    lda.l   inidispBuffer
    and.b   #INIDISP.brightness.mask
    sta.l   inidispBuffer
    sta.l   INIDISP

    plp
    rts
}


// Enables the display at full brightness.
au()
iu()
code()
function EnableDisplay_Full {
    php
    sep     #$20
a8()

    lda.b   #15
    sta.l   inidispBuffer
    sta.l   INIDISP

    plp
    rts
}


// Force blank the display.
//
// This routine should be the only way to force blank the display.
au()
iu()
code()
function ForceBlank {
    php

    sep     #$20
a8()

    lda.l   inidispBuffer
    ora.b   #INIDISP.force
    sta.l   inidispBuffer
    sta.l   INIDISP

    plp
    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

