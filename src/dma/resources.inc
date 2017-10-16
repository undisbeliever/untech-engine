// dma/resources.inc
// =================
//
// Upload resource module animated tile/palettes during VBlank.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace Resources {


// Upload palette to CGRAM during next vblank
// (byte - shadow)
allocate(uploadPaletteOnZero, shadow, 1)


// Initialize the tables/variables
//
// REQUIRES: 16 bit A, DB access shadow
macro _Init() {
    assert16a()

    sep     #$20
a8()
    lda.b   #1
    sta.w   Dma.Resources.uploadPaletteOnZero

    rep     #$20
a16()
}



// Queue the current position of the resource palette buffer to CGRAM
//
// REQUIRES: 16 bit A, DB access shadow
// BRANCH: to NotEnoughDmaTime if out of dma time
// KEEP: X, Y
macro QueuePaletteUpload(NotEnoughDmaTime) {
    assert16a()
    assert(Dma.TRANSFER_SCALE == 128)

    // time = (animationFrameSize + TRANSFER_SCALE/2) / TRANSFER_SCALE + 1
    // if Dma.transfersLeft < time: goto NotEnoughDmaTimeBranch
    // Dma.transfersLeft -= time
    // uploadPaletteOnZero = 0

    lda.w   Resources.Palettes.animationFrameSize
    clc
    adc.w   #Dma.TRANSFER_SCALE + Dma.TRANSFER_SCALE / 2
    asl
    xba
    and.w   #0xff
    eor.w   #0xffff
    sec
    adc.w   Dma.transfersLeft
    bcc     {NotEnoughDmaTime}

    sta.w   Dma.transfersLeft

    lda.w   #0xff
    trb.w   Dma.Resources.uploadPaletteOnZero
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
//
// MODIFIES: CGADD, DMA Channel 0
macro _VBlank() {
    assert16a()
    assert8i()

    ldx.w   Dma.Resources.uploadPaletteOnZero
    bne     SkipPaletteUpload
        // upload current palette animation frame to CGRAM

        // x = 0
        stx.w   CGADD

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
        sta.b   DMAP0       // Also set BBAD0

        lda.w   Resources.Palettes.animationPos
        sta.b   A1T0
        ldx.b   #Resources.Palettes.paletteBuffer >> 16
        stx.b   A1B0

        lda.w   Resources.Palettes.animationFrameSize
        sta.b   DAS0

        ldx.b   #MDMAEN.dma0
        stx.w   MDMAEN

        stx.w   Dma.Resources.uploadPaletteOnZero

    SkipPaletteUpload:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

