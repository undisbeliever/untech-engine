// dma/metatiles.inc
// =================
//
// Manages the MetaTile buffer upload during VBlank.
//
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


namespace Dma {
namespace MetaTiles {


// Disables any pending horizontalBuffer or verticalBuffer updates
//
// REQUIRES: 8 bit A, DB access lowram
macro ResetBufferState() {
    assert8a()

    stz.w   MetaTiles.bufferState
}



// Queue the horizontal buffer for transfer to VRAM
//
// NOTE: This macro will always queue the transfer, even if `Dma.transfersLeft`
// does not contain enough time to upload the horizontal buffer.
//
// REQUIRES: 16 bit A, DB access lowram
macro QueueHorizontalBuffer() {
    assert16a()

    constant VC = Dma.MetaTiles.VBLANK_CYCLES_HORIZONTAL

    lda.w   #MetaTiles.bufferState.HORIZONTAL
    tsb.w   MetaTiles.bufferState

    lda.w   Dma.transfersLeft
    sec
    sbc.w   #Dma.CalcDmaTime(MetaTiles.horizontalBuffer.buffer.size, VC)
    bcs     NoUnderflow{#}
        lda.w   #0
NoUnderflow{#}:
    sta.w   Dma.transfersLeft
}


// Queue the vertical buffer for transfer to VRAM
//
// NOTE: This macro will always queue the transfer, even if `Dma.transfersLeft`
// does not contain enough time to upload the vertical buffer.
//
// REQUIRES: 16 bit A, DB access lowram
macro QueueVerticalBuffer() {
    assert16a()

    constant VC = Dma.MetaTiles.VBLANK_CYCLES_VERTICAL

    lda.w   #MetaTiles.bufferState.VERTICAL
    tsb.w   MetaTiles.bufferState

    lda.w   Dma.transfersLeft
    sec
    sbc.w   #Dma.CalcDmaTime(MetaTiles.verticalBuffer.buffer.size, VC)
    bcs     NoUnderflow{#}
        lda.w   #0
NoUnderflow{#}:
    sta.w   Dma.transfersLeft
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
//
// MODIFIES: VMADD, DMA Channel 0 & DMA Channel 1
// MODIFIES: VMAIN = VMAIN.incrementMode.high
macro _VBlank_dp4300() {
    assert16a()
    assert8i()

    ldx.w   MetaTiles.bufferState
    beq     Return
        assert(MetaTiles.bufferState.VERTICAL == 0x80)
        assert(MetaTiles.bufferState.VERTICAL | MetaTiles.bufferState.HORIZONTAL == 0xff)
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
        assert16a()
        assert(MetaTiles.bufferState.HORIZONTAL & 0x40)
        bit.w   MetaTiles.bufferState - 1
        bvc     SkipHorizontalBuffer


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

    Return:
}

// cpu of cycles (excluding dma) used in the _VBlank macro
constant VBLANK_CYCLES_VERTICAL = 83
constant VBLANK_CYCLES_HORIZONTAL = 88


// Reset the transfer queue in preparation for the next frame.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80, screen active
// INPUT: X is zero
// INPUT: Y is non-zero
// KEEP: A, X, Y
macro _ResetQueue() {
    assert8i()

    // X = 0
    stx.w   MetaTiles.bufferState
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

