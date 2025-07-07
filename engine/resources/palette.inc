// resources/palette.inc
// =====================
//
// Palette resource management.
//
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


RegisterBreakId(INVALID_PALETTE_ID)

namespace Resources {
namespace Palette {


// Load Palette resource.
//
// The palette will be loaded into WRAM and uploaded to CGRAM on the next VBlank.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = palette Id (only 8 bits read)
a16()
i16()
code()
function LoadPalette {
constant _ptr = LZ4.ptr
assertZeroPage(_ptr)
assert(Format.PaletteHeader.size == 3)

    and.w   #0xff
    cmp.w   #Resources.PaletteList.count
    bcc     +
        break(INVALID_PALETTE_ID)
    +
    sta.b   _ptr
    asl
    clc
    adc.b   _ptr
    tax

    lda.l   Resources.PaletteList + 1,x
    sta.b   _ptr + 1
    lda.l   Resources.PaletteList,x
    sta.b   _ptr
    tax


    assert(Format.PaletteHeader.nAnimations == 0)
    lda     [_ptr]              // nAnimations
    inx
    and.w   #0xff
    bne     HasAnimation
        // no animation

        ldy.w   #1
        lda.w   #0

        bra     EndIf

    HasAnimation:
        // palette has an animation

        tay

        assert(Format.PaletteHeader.animationDelay == 1)
        stx.b   _ptr
        lda     [_ptr]          // animationDelay
        inx
        inx
    EndIf:
    assert(Format.PaletteHeader.size == 3)

    // Y = nAnimations
    // A = animationDelay

    sta.w   animationDelay
    sta.w   animationCountdown
    stx.b   _ptr


    // animationFrameSize = *_ptr / nAnimations
    // paletteBufferEnd = LZ4.Decompress(_ptr, paletteBuffer, paletteBuffer.size)

    lda     [_ptr]              // lz4 decompressed size
    sta.l   WRDIV

    sep     #$30
a8()
i8()
    // Y = max(nAnimations, 1)
    tya
    sta.l   WRDIVB                  // wait 16 cycles

    rep     #$30                    // 3
a16()
i16()
    ldx.w   #paletteBuffer          // 3
    stx.w   animationPos            // 5
    ldy.w   #paletteBuffer.size     // 3

    lda.l   RDDIV                   // 2 from loading instruction
    sta.w   animationFrameSize
    Dma.Resources.CalcualateDmaTime(Palette)

    sep     #$20
a8()
    lda.b   #paletteBuffer >> 16

    jsr     LZ4.Decompress

    sty.w   paletteBufferEnd

    rep     #$30
a16()
    Dma.Resources.TestDmaTimeAndSetUploadFlag(Palette, NotEnoughDmaTime)

    rts

NotEnoughDmaTime:
    // upload on next frame
    stz.w   animationCountdown
    rts
}


// Disables the palette animation
// REQUIRES: 16 bit A, DB = 0x7e
macro DisableAnimation() {
    assert16a()

    stz.w   Resources.Palette.animationDelay
}



// Processes one display frame of the palette animation (if enabled).
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro _ProcessAnimation() {
    assert16a()
    assert16i()

    ldy.w   Palette.animationDelay
    beq     Return
        lda.w   Palette.animationCountdown
        sec
        sbc.w   animationSpeed
        bcs     AnimationUnchanged

            Dma.Resources.TestDmaTimeAndSetUploadFlag(Palette, NotEnoughDmaTime)

            lda.w   Palette.animationPos
            clc
            adc.w   Palette.animationFrameSize
            cmp.w   Palette.paletteBufferEnd
            bcc     +
                lda.w   #Palette.paletteBuffer
            +
            sta.w   Palette.animationPos

            // Y = animationDelay
            tya

    AnimationUnchanged:
        sta.w   Palette.animationCountdown

    NotEnoughDmaTime:
Return:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

