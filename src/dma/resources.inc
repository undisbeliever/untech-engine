// dma/resources.inc
// =================
//
// Upload resource module animated tile/palettes during VBlank.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace Resources {


// Upload animated tiles to VRAM on next vblank
// (byte - shadow)
allocate(uploadAnimatedTilesetOnZero, shadow, 1)

// Upload palette to CGRAM during next vblank
// (byte - shadow)
allocate(uploadPaletteOnZero, shadow, 1)


// Initialize the tables/variables
//
// REQUIRES: 16 bit A, DB access shadow
macro _Init() {
    assert8a()

    lda.b   #1
    sta.w   Dma.Resources.uploadAnimatedTilesetOnZero
    sta.w   Dma.Resources.uploadPaletteOnZero
}

// Calculates the dmaTime used by the resource
//
// This macro MUST BE called after writing to `animationFrameSize`
//
// REQUITES: 16 bit A, DB = 0x7e
macro CalcualateDmaTime(name) {
    assert16a()

    // dmaTime = (animationFrameSize + VRAM_CYCLES + TRANSFER_SCALE - 1) / TRANSFER_SCALE

    assert(Dma.TRANSFER_SCALE == 32)

    lda.w   Resources.{name}.animationFrameSize
    clc
    adc.w   #Dma.Resources.VBLANK_CYCLES_PER_RESOURCE + Dma.TRANSFER_SCALE - 1
    lsr
    lsr
    lsr
    lsr
    lsr
    sta.w   Resources.{name}.dmaTime
}


// Tests that there is enough DMA time to upload the animation frame
// during VBlank and either sets the upload flag if it is possible,
// or branches to NotEnoughDmaTime.
//
// REQUIRES: 16 bit A, DB = 0x7e
// BRANCH: to NotEnoughDmaTime if out of dma time
// KEEP: X, Y
macro TestDmaTimeAndSetUploadFlag(name, NotEnoughDmaTime) {
    assert16a()

    // if Dma.transfersLeft < dmaTime: goto NotEnoughDmaTime
    // Dma.transfersLeft -= dmaTime
    // uploadPaletteOnZero = 0

    lda.w   Dma.transfersLeft
    sec
    sbc.w   Resources.{name}.dmaTime
    bcc     {NotEnoughDmaTime}

    sta.w   Dma.transfersLeft

    sep     #$20
a8()
    stz.w   Dma.Resources.upload{name}OnZero

    rep     #$30
a16()
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
// REQUIRES: VMAIN = VMAIN.incrementMode.high | VMAIN.increment.by1
//
// MODIFIES: CGADD, VMADD, DMA Channel 0
macro _VBlank_dp4300() {
    assert16a()
    assert8i()

    ldx.w   Dma.Resources.uploadAnimatedTilesetOnZero
    bne     SkipAnimatedTilesetUpload
        // upload animation tiles to VRAM

        lda.w   Resources.AnimatedTileset.animationVramWaddr
        sta.w   VMADD

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
        sta.b   DMAP0       // Also set BBAD0

        lda.w   Resources.AnimatedTileset.animationBufferPos
        sta.b   A1T0
        ldx.b   #Resources.decompressionBuffer >> 16
        stx.b   A1B0

        lda.w   Resources.AnimatedTileset.animationFrameSize
        sta.b   DAS0

        ldx.b   #MDMAEN.dma0
        stx.w   MDMAEN

    SkipAnimatedTilesetUpload:


    ldx.w   Dma.Resources.uploadPaletteOnZero
    bne     SkipPaletteUpload
        // upload current palette animation frame to CGRAM

        // x = 0
        stx.w   CGADD

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
        sta.b   DMAP0       // Also set BBAD0

        lda.w   Resources.Palette.animationPos
        sta.b   A1T0
        ldx.b   #Resources.Palette.paletteBuffer >> 16
        stx.b   A1B0

        lda.w   Resources.Palette.animationFrameSize
        sta.b   DAS0

        ldx.b   #MDMAEN.dma0
        stx.w   MDMAEN

    SkipPaletteUpload:
}

// maximum number of cycles (excluding dma) per resource
constant VBLANK_CYCLES_PER_RESOURCE = 46


// Resets the position of the transfer queues in preparation for the next frame.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80, screen active
// INPUT: Y is non-zero
// KEEP: A, X, Y
macro _ResetQueue() {
    assert8i()

    // Y is non-zero
    sty.w   Dma.Resources.uploadAnimatedTilesetOnZero
    sty.w   Dma.Resources.uploadPaletteOnZero
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

