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

// Load a Palette into CGRAM
//
// Will always load the palette to the beginning of CGRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, force blank
// DMA: Uses DMA channel 0
// MODIFIES: Disable HDMA, Disable Interrupts, Force Blank
//
// INPUT: A = palette Id
a16()
i16()
code()
function LoadPalette {
constant _palettePtr = LZ4.ptr

    cmp.w   #Resources.PaletteList.count
    bcc     +
        break(INVALID_PALETTE_ID)
    +
    sta.b   _palettePtr
    asl
    clc
    adc.b   _palettePtr
    tax

    lda.l   Resources.PaletteList + 1,x
    sta.b   _palettePtr + 1
    lda.l   Resources.PaletteList,x
    sta.b   _palettePtr


    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB == REGISTER_DB

    sep     #$20
a8()
i16()
    // Disable NMI/HDMA to prevent CGRAM corruption
    stz.w   NMITIMEN
    stz.w   HDMAEN

    lda.b   #INIDISP.force
    sta.w   INIDISP


    // Decompress lz4 block

    ldx.w   #paletteBuffer
    lda.b   #paletteBuffer >> 16
    ldy.w   #paletteBuffer.size

    jsr     LZ4.Decompress


    // Transfer palette to CGRAM

    rep     #$30
a16()
    // Y = end of decompressed buffer
    tya
    clc
    adc.w   #-paletteBuffer
    sta.w   DAS0

    sep     #$20
a8()

    ldx.w   #paletteBuffer
    stx.w   A1T0
    lda.b   #paletteBuffer >> 16
    sta.w   A1B0

    ldy.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
    sty.w   DMAP0   // also sets BBAD0

    stz.w   CGADD

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    plb
    // DB = 0x7e

    rep     #$30
a16()
i16()
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

