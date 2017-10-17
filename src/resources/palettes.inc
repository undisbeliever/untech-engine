// resources/palettes.inc
// ======================
//
// Palette resource management.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_PALETTE_ID)

namespace Resources {
namespace Palettes {


// Load Palette resource.
//
// The palette will be loaded into WRAM and uploaded to CGRAM on the next VBlank.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = palette Id
a16()
i16()
code()
function LoadPalette {
constant _ptr = LZ4.ptr
assert(Format.PaletteHeader.size == 3)

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

        stx.b   _ptr
        lda     [_ptr]          // animationDelay
        inx
        inx
    EndIf:

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
    sta.l   animationFrameSize

    sep     #$20
a8()
    lda.b   #paletteBuffer >> 16

    jsr     LZ4.Decompress

    sty.w   paletteBufferEnd

    rep     #$30
a16()
    Dma.Resources.TestDmaTimeAndSetUploadFlag(Palettes, NotEnoughDmaTime)

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

    stz.w   Resources.Palettes.animationDelay
}



// Processes one display frame of the palette animation (if enabled).
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro _ProcessAnimation() {
    assert16a()
    assert16i()

    ldy.w   Palettes.animationDelay
    beq     Return
        lda.w   Palettes.animationCountdown
        sec
        sbc.w   animationSpeed
        bcs     AnimationUnchanged

            Dma.Resources.TestDmaTimeAndSetUploadFlag(Palettes, NotEnoughDmaTime)

            lda.w   Palettes.animationPos
            clc
            adc.w   Palettes.animationFrameSize
            cmp.w   Palettes.paletteBufferEnd
            bcc     +
                lda.w   #Palettes.paletteBuffer
            +
            sta.w   Palettes.animationPos

            // Y = animationDelay
            tya

    AnimationUnchanged:
        sta.w   Palettes.animationCountdown

    NotEnoughDmaTime:
Return:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

