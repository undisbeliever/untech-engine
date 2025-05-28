// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.

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
        jsl     WaitFrame__far
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


    jsl     WaitFrame__far

    // verify that colors 128 - 255 of CGRAM is 0x1f1f

    jsl     ForceBlank__far


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

    jsl     WaitFrame__far

    // Fill the entirety of the buffer with 0x7f
    lda.w   #0x7f7f
    ldx.w   #Dma.ObjPalette.buffer.size - 2
    -
        sta.w   Dma.ObjPalette.buffer,x
        dex
        dex
        bpl     -


    jsl     WaitFrame__far

    // verify that colors 128 - 255 of CGRAM are not 0x7f7f

    jsl     ForceBlank__far


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

