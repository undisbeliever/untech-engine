// dma/metatiles.inc
// =================
//
// Manages the MetaTile buffer upload during VBlank.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace MetaTiles {


// Initialize the tables/variables
//
// REQUIRES: 16 bit A, DB access shadow
macro _Init() {
    assert16a()

    sep     #$20
a8()
    stz.w   MetaTiles.bufferState

    rep     #$20
a16()
}



// Queue the horizontal buffer for transfer to VRAM
//
// REQUIRES: 16 bit A, DB access shadow
macro QueueHorizontalBuffer() {
    assert16a()

    lda.w   #MetaTiles.bufferState.HORIZONTAL
    tsb.w   MetaTiles.bufferState

    lda.w   Dma.transfersLeft
    sec
    sbc.w   #MetaTiles.horizontalBuffer.buffer.size / Dma.TRANSFER_SCALE + 1
    sta.w   Dma.transfersLeft
}


// Queue the vertical buffer for transfer to VRAM
//
// REQUIRES: 16 bit A, DB access shadow
macro QueueVerticalBuffer() {
    assert16a()

    lda.w   #MetaTiles.bufferState.VERTICAL
    tsb.w   MetaTiles.bufferState

    lda.w   Dma.transfersLeft
    sec
    sbc.w   #MetaTiles.verticalBuffer.buffer.size / Dma.TRANSFER_SCALE + 1
    sta.w   Dma.transfersLeft
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
//
// MODIFIES: VMADD, DMA Channel 0 & DMA Channel 1
// MODIFIES: VMAIN = VMAIN.incrementMode.high
macro _VBlank() {
    assert16a()
    assert8i()

    ldx.w   MetaTiles.bufferState
    beq     Return
        assert(MetaTiles.bufferState.VERTICAL == 0x80)
        bpl     TransferHorizontalBuffer
            // Transfer Vertical Buffer to VRAM

            assert(MetaTiles.verticalBuffer.buffer.size == 32 * 2 * 2)

            ldy.b   #VMAIN.incrementMode.high | VMAIN.increment.by32
            sty.w   VMAIN

            lda.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
            sta.b   DMAP0       // Also set BBAD0

            lda.w   #MetaTiles.verticalBuffer.buffer
            sta.b   A1T0
            ldy.b   #MetaTiles.verticalBuffer.buffer >> 16
            sty.b   A1B0

            lda.w   #MetaTiles.verticalBuffer.buffer.size / 2
            sta.b   DAS0

            lda.w   MetaTiles.verticalBuffer.vramWaddr
            sta.w   VMADD

            ldx.b   #MDMAEN.dma0
            stx.w   MDMAEN

            inc
            sta.w   VMADD

            ldy.b   #MetaTiles.verticalBuffer.buffer.size / 2
            sty.b   DAS0L       // DAS0H is 0

            // x = MDMAEN.dma0
            stx.w   MDMAEN


            ldy.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
            sty.w   VMAIN


        // Skip horizontalBuffer if it not queued
        assert(MetaTiles.bufferState.HORIZONTAL & 0x01)
        lda.w   MetaTiles.bufferState
        lsr
        bcc     SkipHorizontalBuffer


        TransferHorizontalBuffer:
            // Transfer buffer to VRAM

            // Uses two DMA channels to split the continuous 64 * 2 tilemap
            // buffer into two 32 * 2 buffers required by the PPU.

            assert(MetaTiles.horizontalBuffer.buffer.size == 64 * 2 * 2)

            lda.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
            sta.b   DMAP0       // Also set BBAD0
            sta.b   DMAP1       // Also set BBAD0

            lda.w   #MetaTiles.horizontalBuffer.buffer
            sta.b   A1T0
            lda.w   #MetaTiles.horizontalBuffer.buffer + 64 * 2
            sta.b   A1T1

            ldx.b   #MetaTiles.horizontalBuffer.buffer >> 16
            stx.b   A1B0
            stx.b   A1B1

            lda.w   #32 * 2
            sta.b   DAS0
            sta.b   DAS1

            lda.w   MetaTiles.horizontalBuffer.vramWaddr
            sta.w   VMADD

            ldx.b   #MDMAEN.dma0 | MDMAEN.dma1
            stx.w   MDMAEN

            clc
            adc.w   #32 * 32
            sta.w   VMADD

            ldy.b   #32 * 2
            sty.b   DAS0L       // DAS0H is 0
            sty.b   DAS1L       // DAS1H is 1

            // x = MDMAEN.dma0 | MDMAEN.dma1
            stx.w   MDMAEN

    SkipHorizontalBuffer:

        stz.w   MetaTiles.bufferState

    Return:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

