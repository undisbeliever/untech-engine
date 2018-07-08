// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Wait until the start of the next frame, counting the number
// of loops waited.
//
// REQUIRES: DB access shadow
//
// RETURN: Y = number of loops waited
au()
iu()
code()
function WaitFrame_Count {
    php

    sep     #$30
a8()
i8()
    ldy.b   #0

    // Don't wait for frame when in force-blank
    assert(INIDISP.force == 0x80)
    lda.w   inidispBuffer
    bmi     Return

    rep     #$20
a16()

    Dma.FinalizeFrame()

    rep     #$30
a16()
i16()

    ldy.w   #0
    ldx.w   frameCounter

    Loop:
        // Constant time loop
        iny

        cpx.w   frameCounter
        beq     Loop

Return:
    plp
    rts
}


// Dirties all the dpTmp variables
//
// REQUIRES: none
code()
function DirtyDpTmp {
    php
    phd

    rep     #$30
i16()
a16()

    lda.w   #0
    tcd

    lda.l   frameCounter
    eor.l   JOY1
    eor.l   JOY2
    eor.w   #0xffff

    ldx.w   dpTmp19 + 2

    ldx.w   #DP_TMP_COUNT * 2 - 2
    -
        dec
        sta.b   dpTmp0,x

        dex
        dex
        bpl     -

    pld
    plp
    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

