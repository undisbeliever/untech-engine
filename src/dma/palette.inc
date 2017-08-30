// dma/palette.inc
// ===============
//
// Manages the CGRAM buffer upload during VBlank.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace Palette {

allocate(buffer, wram7e, 256 * 2)

allocateTmpWord(dirty)
namespace dirty {
    // Values match TransferTable Index
    constant tiles = 0x02
    constant objects = 0x04
    constant all = 0x06
}

namespace vblank {
    allocate(cgadd, shadow, 1)
    allocate(buffer, shadow, 2)
    allocate(size, shadow, 2)
}


// Initialize the tables/variables
macro _Init() {
    assert16a()
    assert16i()

    stz.w   Dma.Palette.vblank.size
}


macro _TransferOnNextVBlank() {
    assert16a()
    assert16i()

    lda.w   Dma.Palette.dirty
    and.w   #$00FF
    beq     EndIf
        tax

        lda.l   Dma.Palette.TransferTable.Cgadd,x
        sta.w   Dma.Palette.vblank.cgadd

        lda.l   Dma.Palette.TransferTable.Buffer,x
        sta.w   Dma.Palette.vblank.buffer

        lda.l   Dma.Palette.TransferTable.Size,x
        sta.w   Dma.Palette.vblank.size

        stz.w   Dma.Palette.dirty
    EndIf:
}


// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro _VBlank() {
    assert16a()
    assert8i()

    lda.w   Dma.Palette.vblank.size
    beq     EndIf
        sta.b   DAS0

        lda.w   Dma.Palette.vblank.buffer
        sta.w   A1T0
        ldx.b   #Dma.Palette.buffer >> 16
        stx.b   A1B0

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
        sta.b   DMAP0       // Also set BBAD0

        ldy.w   Dma.Palette.vblank.cgadd
        sty.w   CGADD


        ldy.b   #MDMAEN.dma0
        sty.w   MDMAEN

        stz.w   Dma.Palette.vblank.size
    EndIf:
}


// Marks a fixed section of the buffer as dirty
//
// PARAM: variable in Dma.Palette.dirty struct
macro MarkDirty(pattern) {
    assert8a()

    lda.b   #Dma.Palette.dirty.{pattern}
    tsb.w   Dma.Palette.dirty

    bne     EndIf{#}
        // dirty bits were previously unset, now setting them

        assert(Dma.TRANSFERS_PER_PAL_FRAME <= 0x80)
        assert(Dma.TRANSFER_SCALE == 128)

        lda.w   Dma.transfersLeft

        if Dma.Palette.dirty.{pattern} == 0x02 || Dma.Palette.dirty.{pattern} == 0x04 {
            dec
            dec
        } else if Dma.Palette.dirty.{pattern} == 0x06 {
            sec
            sbc.b   #4
        } else {
            error "Unknown pattern"
        }
        bpl +
            lda.b   #0
        +
        sta.w   Dma.transfersLeft
    EndIf{#}:
}


rodata(rom0)
function TransferTable {
    Cgadd:
        dw  0
        dw  0
        dw  128
        dw  0

    Buffer:
        dw  0
        dw  buffer
        dw  buffer + 128 * 2
        dw  buffer

    Size:
        dw  0
        dw  256
        dw  256
        dw  512
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

