// dma/obj-palette.inc
// ===================
//
// Manages the object palette buffer.
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
namespace ObjPalette {

// Memory buffer to hold object palette data
// Will be transferred to CGRAM during VBlank when `dirty` is non-zero
constant buffer.size = 128 * 2
allocate(buffer, wram7e, buffer.size)

// If zero then the buffer will be transferred to CGRAM on the next VBlank.
// (lowram word)
allocate(transferOnZero, lowram, 1)


// Initialize the tables/variables
macro _Init() {
    assert8a()

    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero
}


// Upload Object Palette to CGRAM if dirty
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
// MODIFIES: CGADD, Dma Channel 0
macro _VBlank_dp4300() {
    assert16a()
    assert8i()

    ldy.w   Dma.ObjPalette.transferOnZero
    bne     EndIf
        ldy.b   #128
        sty.w   CGADD

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
        sta.b   DMAP0       // Also set BBAD0

        lda.w   #Dma.ObjPalette.buffer.size
        sta.b   DAS0

        lda.w   #Dma.ObjPalette.buffer
        sta.b   A1T0
        ldx.b   #Dma.ObjPalette.buffer >> 16
        stx.b   A1B0

        ldy.b   #MDMAEN.dma0
        sty.w   MDMAEN

    EndIf:
}

// cpu of cycles (excluding dma) used in the _VBlank macro
constant VBLANK_CYCLES = 38


// Reset the transfer queue in preparation for the next frame.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80, screen active
// INPUT: X is zero
// INPUT: Y is non-zero
// KEEP: A, X, Y
macro _ResetQueue() {
    assert8i()

    // Y is non-zero
    sty.w   Dma.ObjPalette.transferOnZero
}


// Marks the object palette buffer as dirty
//
// NOTE: This macro will always mark the buffer as dirty, even if
// `Dma.transfersLeft` does not contain enough time to upload the ObjPalette
// buffer.
//
// REQUIRES: 8 bit A.
macro MarkDirty() {
    assert8a()

    lda.w   Dma.ObjPalette.transferOnZero
    beq     AlreadyMarkedDirty{#}

        lda.w   Dma.transfersLeft
        sec
        sbc.b   #Dma.CalcDmaTime(Dma.ObjPalette.buffer.size, Dma.ObjPalette.VBLANK_CYCLES)
        bcs     NoUnderflow{#}
            lda.b   #0
    NoUnderflow{#}:
        sta.w   Dma.transfersLeft

        stz.w   Dma.ObjPalette.transferOnZero

AlreadyMarkedDirty{#}:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

