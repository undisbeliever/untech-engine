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
// (byte - shadow, project editable)
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


// Enables the display, NMI (VBlank) ISR and autoJoy
//
// Does not immediately turn on the display. The screen will be turned
// on during the next VBlank to prevent screen tearing.
//
// The brightness of the display will be unchanged from the previous
// value of `inidispBuffer`.
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


    lda.b   #NMITIMEN.vBlank | NMITIMEN.autoJoy
    sta.l   NMITIMEN

    plp
    rts
}


// Enables the display at full brightness, NMI (VBlank) ISR and autoJoy
//
// Does not immediately turn on the display. The screen will be turned
// on during the next VBlank to prevent screen tearing.
au()
iu()
code()
function EnableDisplay_Full {
    php
    sep     #$20
a8()

    lda.b   #15
    sta.l   inidispBuffer


    lda.b   #NMITIMEN.vBlank | NMITIMEN.autoJoy
    sta.l   NMITIMEN

    plp
    rts
}


// Force blank the display and disables interrupts and HDMA.
//
// The screen is immediately blanked. To prevent screen tearing this
// routine should called after a `WaitFrame` call.
//
// Interrupts and HDMA are disabled to prevent to VRAM/CGRAM/OAM registers from
// unexpectedly changing in the middle of an upload routine.
//
// This routine should be the only way to force blank the display or to disable
// interrupts.
au()
iu()
code()
function ForceBlank {
    php

    sep     #$20
a8()

    lda.b   #0
    sta.l   NMITIMEN
    sta.l   HDMAEN


    lda.l   inidispBuffer
    ora.b   #INIDISP.force
    sta.l   inidispBuffer
    sta.l   INIDISP

    plp
    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

