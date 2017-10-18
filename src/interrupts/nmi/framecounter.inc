// interrupts/nmi/framecounter.inc
// ===============================
//
// Frame counter, incremented every VBlank.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


allocate(frameCounter, shadow, 2)

// If non-zero then show the CPU usage meter on screen
// (byte flag - shadow)
allocate(showCpuUsageMeter, shadow, 1)


namespace NmiHandler {
// VBlank code, increase frameCounter
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro FrameCounter.VBlank() {
    assert16a()

    inc.w   frameCounter
}
}

// Wait until the start of the next frame.
//
// If in force blank then this routine does nothing.
//
// If `showCpuUsageMeter` is non-zero then the screen brightness is changed to
// show the user the scanline that this routine is called at.
//
// REQUIRES: DB access shadow
au()
iu()
code()
function WaitFrame {
    php
    sep     #$30
a8()
i8()

    // Don't wait for frame when in force-blank
    assert(INIDISP.force == 0x80)
    lda.w   inidispBuffer
    bmi     Return

    ldx.w   showCpuUsageMeter
    beq     +
        eor.b   #7
        sta.l   INIDISP
    +

    lda.w   frameCounter

    -
        wai
        cmp.w   frameCounter
        beq     -

Return:
    plp
    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

