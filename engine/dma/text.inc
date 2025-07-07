// dma/text.inc
// ============
//
// Manages the text buffer upload during VBlank.
//
// The text buffer is only uploaded to the low byte of each VRAM word IF there
// is enough VBlank time to transfer the entire buffer to VRAM.
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
namespace Text {

// VBlank code.
//
// NOTE: changes VMAIN mode
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300, DB = REGISTERS
macro _VBlank_dp4300() {
    assert16a()
    assert8i()

    ldx.w   Text.bufferDirty
    beq     EndIf

        // Test if there is enough VBlank time available
        ldx.w   Dma.transfersLeft
        cpx.b   #Dma.CalcDmaTime(Text.buffer.size, Dma.Text.VBLANK_CYCLES) + 1
        bcc     EndIf

            // Buffer is dirty, and we have the time available to
            // copy to vram.


            ldx.b   #VMAIN.incrementMode.low | VMAIN.increment.by1
            stx.w   VMAIN

            assertLowRam(Text.tilemapVramWaddr)
            lda.w   Text.tilemapVramWaddr
            sta.w   VMADD

            lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (VMDATAL << 8)
            sta.b   DMAP0       // Also set BBAD0

            lda.w   #Text.buffer
            sta.b   A1T0
            ldx.b   #Text.buffer >> 16
            stx.b   A1B0

            lda.w   #Text.buffer.size
            sta.b   DAS0

            ldy.b   #MDMAEN.dma0
            sty.w   MDMAEN


            // Buffer now clean
            ldy.b   #0
            sty.w   Text.bufferDirty
    EndIf:
}

// cpu of cycles (excluding dma) used in the _VBlank macro
constant VBLANK_CYCLES = 54

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

