// dma/metasprite.inc
// ==================
//
// Manages the MetaSprite buffer upload during VBlank.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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
namespace MetaSprite {

constant BYTES_TO_TRANSFER = 128 * 4 + 128 / 4

// Preform DMA transfer if zero
// (byte)
allocate(transferOnZero, lowram, 1)


// Decrement transfers left Counter
//
// NOTE: This macro will always succeed, even if `Dma.transfersLeft`
// does not contain enough time to upload the OAM buffer.
//
// REQUIRES: 16 bit A, DB access lowram
macro _InitLoop() {
    assert16a()

    lda.w   Dma.transfersLeft
    sec
    sbc.w   #Dma.CalcDmaTime(Dma.MetaSprite.BYTES_TO_TRANSFER, Dma.MetaSprite.VBLANK_CYCLES)
    bcs     NoUnderflow{#}
        lda.w   #0
NoUnderflow{#}:
    sta.w   Dma.transfersLeft
}


// EndLoop code, enable transfer of oamBuffer to OAM on next VBlank
//
// REQUIRES: 8 bit A, DB access lowram
macro _EndLoop() {
    assert8a()

    stz.w   Dma.MetaSprite.transferOnZero
}


// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300, DB = REGISTERS
macro _VBlank_dp4300() {
    assert16a()
    assert8i()

    ldx.w   Dma.MetaSprite.transferOnZero
    bne     EndIf
        stz.w   OAMADD

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (OAMDATA << 8)
        sta.b   DMAP0       // Also set BBAD0

        lda.w   #MetaSprite.Render.oamBuffer
        sta.b   A1T0
        ldx.b   #MetaSprite.Render.oamBuffer >> 16
        stx.b   A1B0

        lda.w   #Dma.MetaSprite.BYTES_TO_TRANSFER
        sta.b   DAS0

        ldy.b   #MDMAEN.dma0
        sty.w   MDMAEN
    EndIf:
}

// cpu of cycles (excluding dma) used in the _VBlank macro
constant VBLANK_CYCLES = 37


// Reset the transfer queue in preparation for the next frame.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80, screen active
// INPUT: X is zero
// INPUT: Y is non-zero
// KEEP: A, X, Y
macro _ResetQueue() {
    assert8i()

    // Y is non-zero
    sty.w   Dma.MetaSprite.transferOnZero
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

