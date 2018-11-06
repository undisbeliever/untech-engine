// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace DmaTest {
namespace ObjPalette {

a16()
i16()
code()
Test.add("Dma.ObjPalette.MarkDirty")
function MarkDirtyTest {
    jsr     Dma.Init

    lda.w   Dma.transfersLeft
    cmp.w   #5
    bcs     +
        jsr     WaitFrame
    +

    // Fill the entirety of the buffer with 0x1f
    lda.w   #0x1f1f
    ldx.w   #Dma.ObjPalette.buffer.size - 2
    -
        sta.w   Dma.ObjPalette.buffer,x
        dex
        dex
        bpl     -

    sep     #$20
a8()

    lda.w   Dma.transfersLeft
    pha
        Dma.ObjPalette.MarkDirty()
    pla

    // Ensure transfersLeft decreased
    cmp.w   Dma.transfersLeft
    beq     Fail
    bcc     Fail


    // Ensure marking dirty again doesn't decrease transfersLeft
    lda.w   Dma.transfersLeft
    pha
        Dma.ObjPalette.MarkDirty()
        Dma.ObjPalette.MarkDirty()
        Dma.ObjPalette.MarkDirty()
    pla
    cmp.w   Dma.transfersLeft
    bne     Fail


    jsr     WaitFrame

    // verify that colors 128 - 255 of CGRAM is 0x1f1f

    jsr     ForceBlank


    lda.b   #128
    sta.l   CGADD

    ldx.w   #128 * 2
    -
        lda.l   CGDATAREAD
        and.b   #0x7f       // filter out open bus
        cmp.b   #0x1f
        bne     Fail

        dex
        bne     -

    jsr     Test.ResetPpuState

    sec
    rts

Fail:
    clc
    rts
}


// Tests that the ObjPalette buffer is only transferred when
// `Dma.ObjPalette.MarkDirty` is invoked.
//
// There used to be a bug that would transfer the ObjPalette
// buffer on every frame, but it has been fixed.
a16()
i16()
code()
Test.add("Dma.ObjPalette.MarkDirty (not called)")
function MarkDirty_NotCalled {
    // No need to invoke Dma.Init as it was already called by the
    // previous test.

    // Assumes that either the previous test or the Reset code
    // has set colours 127-255 of CGRAM to black or 0x1f1f (yellow)

    sep     #$20
a8()

    jsr     WaitFrame

    // Fill the entirety of the buffer with 0x7f
    lda.w   #0x7f7f
    ldx.w   #Dma.ObjPalette.buffer.size - 2
    -
        sta.w   Dma.ObjPalette.buffer,x
        dex
        dex
        bpl     -


    jsr     WaitFrame

    // verify that colors 128 - 255 of CGRAM are not 0x7f7f

    jsr     ForceBlank


    lda.b   #128
    sta.l   CGADD

    ldx.w   #128 * 2
    -
        lda.l   CGDATAREAD
        and.b   #0x7f       // filter out open bus
        cmp.b   #0x7f
        beq     Fail

        dex
        bne     -

    jsr     Test.ResetPpuState

    sec
    rts

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

