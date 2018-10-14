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


// Wait until the start of the next frame, printing the number
// of loops waited on the top-right of the screen.
//
// REQUIRES: DB access shadow
//
// RETURN: Y = number of loops waited
code()
function WaitFrame_Print {
    Text.Console.SetCursor(Text.Console.TEXT_WIDTH - 5, 0)

    // Using hex as it is constant time

    jsr     WaitFrame_Count
    jmp     Text.Console.PrintHex16Y
}

// vim: ft=bass-65816 ts=4 sw=4 et:

