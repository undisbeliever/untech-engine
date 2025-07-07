// metatiles/_vblank.inc
// =====================
//
// MetaTile DrawMapCell VBlank macro
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

