// dma/obj-palette.inc
// ===================
//
// Manages the object palette buffer.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace ObjPalette {

// Memory buffer to hold object palette data
// Will be transferred to CGRAM during VBlank when `dirty` is non-zero
constant buffer.size = 128 * 2
allocate(buffer, wram7e, buffer.size)

// If zero then the buffer will be transferred to CGRAM on the next VBlank.
// (shadow word)
allocate(transferOnZero, shadow, 1)


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

