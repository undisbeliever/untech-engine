// dma/forceblank.inc
// ==================
//
// Force Blank DMA Transfers.
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


RegisterBreakId(NOT_IN_FORCE_BLANK)

namespace Dma {
namespace ForceBlank {

// Breaks execution if not in force blank
//
// REQUIRES: DB access lowram
// KEEP: A, X, Y
macro AssertForceBlank() {
    if __CPU__.aSize == 8 {
        bit.w   inidispAfterWaitFrame
    } else if __CPU__.aSize == 16 {
        bit.w   inidispAfterWaitFrame - 1
    } else {
        error "Unknown A size"
    }
    bmi     InForceBlank{#}
        break(NOT_IN_FORCE_BLANK)
    InForceBlank{#}:
}


// Dirties a block of Work RAM, replacing every byte in the block with `0xba`.
//
// This macro requires force blank to prevent the HDMA/DMA crash on S-CPU
// version 1.
//
// The byte `0xba` was chosen because it is easily recognizable and has bit 7
// set.
//
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
macro DirtyWram(evaluate addr, evaluate size) {
    assert8a()
    assert16i()

    evaluate bank = {addr} >> 16
    assert({bank} == 0 || {bank} == 0x7e || {bank} == 0x7f)

    ldx.w   #{addr}
    lda.b   #{bank}
    ldy.w   #{size}
    jsr     Dma.ForceBlank.DirtyWram
}


// Transfers a block of memory to VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
macro ToVram(addr, size) {
    assert8a()
    assert16i()

    ldx.w   #{addr}
    lda.b   #({addr}) >> 16
    ldy.w   #{size}
    jsr     Dma.ForceBlank.TransferToVram
}
macro ToVram(addr) {
    Dma.ForceBlank.ToVram({addr}, {addr}.size)
}


// Transfers a block of memory to the low byte of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
macro ToVramL(addr, size) {
    assert8a()
    assert16i()

    ldx.w   #{addr}
    lda.b   #({addr}) >> 16
    ldy.w   #{size}
    jsr     Dma.ForceBlank.TransferToVramL
}
macro ToVramL(addr) {
    Dma.ForceBlank.ToVramL({addr}, {addr}.size)
}


// Transfers a block of memory to the high byte of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
macro ToVramH(addr, size) {
    assert8a()
    assert16i()

    ldx.w   #{addr}
    lda.b   #({addr}) >> 16
    ldy.w   #{size}
    jsr     Dma.ForceBlank.TransferToVramH
}
macro ToVramH(addr) {
    Dma.ForceBlank.ToVramH({addr}, {addr}.size)
}



// Dirties a block of Work RAM, replacing every byte in the block with `0xba`.
//
// This macro requires force blank to prevent the HDMA/DMA crash on S-CPU
// version 1.
//
// The byte `0xba` was chosen because it is easily recognizable and has bit 7
// set.
//
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: A:X - wram address
// INPUT: Y - size (in bytes)
a8()
i16()
code()
function DirtyWram {
    stx.w   WMADDL
    sta.w   WMADDH

    sty.w   DAS0

    ldy.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.one | (WMDATA << 8)
    sty.w   DMAP0       // also sets BBAD0

    ldx.w   #BaByte
    stx.w   A1T0
    lda.b   #BaByte >> 16
    sta.w   A1B0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts


rodata(rom0)
BaByte:
    db  0xba
}



// Clears a block of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: X - vram address
// INPUT: Y - size (in bytes)
a8()
i16()
code()
function ClearVram {
    stx.w   VMADD
    sty.w   DAS0

    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.w   VMAIN

    ldy.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.two | (VMDATA << 8)
    sty.w   DMAP0       // also sets BBAD0

    ldx.w   #SnesHeader.EmptyBytes
    stx.w   A1T0
    stz.w   A1B0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}


// Clear the high byte of a block of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: X - vram address
// INPUT: Y - size (in bytes to clear)
a8()
i16()
code()
function ClearVramH {
    stx.w   VMADD
    sty.w   DAS0

    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.w   VMAIN

    ldy.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.one | (VMDATAH << 8)
    sty.w   DMAP0       // also sets BBAD0

    ldx.w   #SnesHeader.EmptyBytes
    stx.w   A1T0
    stz.w   A1B0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}


// Clears a block of CGRAM
// DMA: Uses DMA channel 0
// REQUIRES: DB access registers
// REQUIRES: screen blank
//
// INPUT: X - starting color
// INPUT: Y - nColors
a8()
i16()
code()
function ClearCgram {
    php

    rep     #$30
    sep     #$10
a16()
i8()

    stx.w   CGADD

    tya
    asl
    sta.w   DAS0

    lda.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.one | (CGDATA << 8)
    sta.w   DMAP0       // also sets BBAD0

    stz.w   A1B0 - 1
    lda.w   #SnesHeader.EmptyBytes
    sta.w   A1T0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    ldx.b   #MDMAEN.dma0
    stx.w   MDMAEN

    plp
    rts
}


// Transfers a block of memory to CGRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: A:X address
// INPUT: Y   size (in bytes)
a8()
i16()
code()
function TransferToCgram {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
    stx.w   DMAP0       // also sets BBAD0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}


// Transfers a block of memory to VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: A:X address
// INPUT: Y   size (in bytes)
a8()
i16()
code()
function TransferToVram {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    lda.b   #VMAIN.increment.by1 | VMAIN.incrementMode.high
    sta.w   VMAIN

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
    stx.w   DMAP0       // also sets BBAD0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}


// Transfers a block of memory to the low byte of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: A:X address
// INPUT: Y   size (in bytes)
a8()
i16()
code()
function TransferToVramL {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    assert(VMAIN.incrementMode.low | VMAIN.increment.by1 == 0)
    stz.w   VMAIN

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (VMDATAL << 8)
    stx.w   DMAP0       // also sets BBAD0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}


// Transfers a block of memory to the high byte of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: A:X address
// INPUT: Y   size (in bytes)
a8()
i16()
code()
function TransferToVramH {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    lda.b   #VMAIN.increment.by1 | VMAIN.incrementMode.high
    sta.w   VMAIN

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (VMDATAH << 8)
    stx.w   DMAP0       // also sets BBAD0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}

// Fills a 32x32 VRAM tilemap, setting each tile to a given word
//
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: X = VRAM word address
// INPUT: Y = tile
a8()
i16()
code()
function FillTilemap {
allocateTmpWord(tmp)

    rep     #$30
a16()
    tya
    sta.l   tmp

    sep     #$20
a8()

    // Low byte
    phx
    stx.w   VMADD

    assert(VMAIN.incrementMode.low | VMAIN.increment.by1 == 0)
    stz.w   VMAIN

    ldy.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.one | (VMDATAL << 8)
    sty.w   DMAP0       // also sets BBAD0

    ldx.w   #tmp
    stx.w   A1T0
    lda.b   #tmp >> 16
    sta.w   A1B0

    ldy.w   #32 * 32
    sty.w   DAS0

    AssertForceBlank()

    // Disable HDMA (prevents a crash on model 1 consoles)
    stz.w   HDMAEN

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN


    // High byte
    plx
    stx.w   VMADD

    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.w   VMAIN

    lda.b   #VMDATAH
    sta.w   BBAD0

    ldx.w   #tmp + 1
    stx.w   A1T0

    ldy.w   #32 * 32
    sty.w   DAS0

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

