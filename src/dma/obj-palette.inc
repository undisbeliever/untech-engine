// dma/obj-palette.inc
// ===================
//
// Manages the object palette buffer.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace ObjPalette {

// Memory buffer to hold object palette data
// Will be transferred to CGRAM during VBlank when `dirty` is non-zero
constant buffer.size = 128 * 2
allocate(buffer, wram7e, buffer.size)

allocateTmpWord(dirty)


// Initialize the tables/variables
macro _Init() {
    assert16a()
    assert16i()

    stz.w   Dma.ObjPalette.dirty
}


// Upload Object Palette to CGRAM if dirty
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
// MODIFIES: CGADD, Dma Channel 0
macro _VBlank() {
    assert16a()
    assert8i()

    ldy.w   Dma.ObjPalette.dirty
    beq     EndIf
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

        stz.w   Dma.ObjPalette.dirty
    EndIf:
}


// Marks the object palette buffer as dirty
// REQUIRES: 8 bit A.
macro MarkDirty() {
    assert8a()

    lda.w   Dma.ObjPalette.dirty
    bne     AlreadyDirty{#}

        lda.w   Dma.transfersLeft
        sec
        sbc.b   #Dma.ObjPalette.buffer.size / Dma.TRANSFER_SCALE + 1
        sta.w   Dma.transfersLeft

        lda.b   #1
        sta.w   Dma.ObjPalette.dirty

AlreadyDirty{#}:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

