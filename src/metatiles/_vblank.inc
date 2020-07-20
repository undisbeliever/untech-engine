// metatiles/_vblank.inc
// =====================
//
// MetaTile DrawMapCell VBlank macro
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
namespace Render {


// Transfer drawMapCellBuffer to VRAM during VBlank
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
//
// MODIFIES: VMAIN = VMAIN.incrementMode.high
macro DrawMapCell_VBlank_dp4300() {
    assert16a()
    assert8i()

    // MUST NOT use DMA in this macro

    ldx.w   MetaTiles.drawMapCellBufferIndex
    beq     Return
        lda.w   #0x2100
        tcd

        ldy.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
        sty.b   VMAIN

        Loop:
            lda.w   MetaTiles.drawMapCellBuffer.vramWaddr - 2,x
            sta.b   VMADD

            lda.w   MetaTiles.drawMapCellBuffer.topLeft - 2,x
            sta.b   VMDATA
            lda.w   MetaTiles.drawMapCellBuffer.topRight - 2,x
            sta.b   VMDATA

            lda.w   MetaTiles.drawMapCellBuffer.vramWaddr - 2,x
            ora.w   #32
            sta.b   VMADD

            lda.w   MetaTiles.drawMapCellBuffer.bottomLeft - 2,x
            sta.b   VMDATA
            lda.w   MetaTiles.drawMapCellBuffer.bottomRight - 2,x
            sta.b   VMDATA

            dex
            dex
            bne     Loop

        // X = 0
        stx.w   MetaTiles.drawMapCellBufferIndex

        lda.w   #0x4300
        tcd
Return:
}
constant DRAW_MAP_CELL_VBLANK_INIT_CYCLES = 26
constant DRAW_MAP_CELL_VBLANK_LOOP_CYCLES = 64

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

