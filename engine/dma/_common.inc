// dma/_common.inc
// ===============
//
// Common Macros for the DMA module.
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

// Counter to manage the amount of VBlank time I have left.
//
// This is reset every NMI interrupt.
//
// Each value represents transferring TRANSFER_SCALE bytes during DMA.
allocate(transfersLeft, lowram, 2)


// Initialize the counters
//
// REQUIRES: 16 bit A, 16 bit Index, DB access lowram
macro ResetTransfersLeft() {
    assert16a()

    lda.w   STAT77
    and.w   #STAT78.pal << 8
    beq     +
        lda.w   #Dma.TRANSFERS_PER_PAL_FRAME
        bra     ++
    +
        lda.w   #Dma.TRANSFERS_PER_NTSC_FRAME
    +
    sta.w   Dma.transfersLeft
}


// Initializes the DMA module.
//
// This MUST be called before using the DMA module.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
a16()
i16()
code()
function Init {
    sep     #$20
a8()

    MetaTiles.ResetBufferState()

    ObjPalette._Init()
    Resources._Init()

    rep     #$30
a16()
i16()
    Tile16._Init()

    Dma.ResetTransfersLeft()

    rts
}


// Finalize the transfer queues for the current frame and prep them for
// VBlank.
//
// Called by WaitFrame, just before the `wai` instruction.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80
macro FinalizeFrame() {
    assert16a()
    assert8i()

    Dma.Tile16._FinalizeFrame()
}


// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank_dp4300() {
    assert16a()
    assert8i()

    ldx.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    stx.w   VMAIN

    Dma.MetaSprite._VBlank_dp4300()
    Dma.MetaTiles._VBlank_dp4300()
    Dma.ObjPalette._VBlank_dp4300()
    Dma.Resources._VBlank_dp4300()
    Dma.Tile16._VBlank_dp4300()

    // Text module is low priority.
    // Must be done last
    Dma.Text._VBlank_dp4300()
}


// Reset the transfer queues in preparation for the next frame.
//
// Called by WaitFrame, after the VBlank macros have completed
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80, screen active
macro ResetQueues() {
    assert16a()
    assert8i()

    ldx.b   #0
    ldy.b   #0xff

    Dma.MetaSprite._ResetQueue()
    Dma.MetaTiles._ResetQueue()
    Dma.ObjPalette._ResetQueue()
    Dma.Resources._ResetQueue()
    Dma.Tile16._ResetQueue()

    // Do not reset the Dma.Text queue as the text buffer might not have been uploaded.

    Dma.ResetTransfersLeft()
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

