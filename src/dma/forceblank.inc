// dma/forceblank.inc
// ==================
//
// DMA transfers to call on force blank.
//

scope Dma {
scope ForceBlank {


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


// Clears a block of VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: X - vram address
// INPUT: Y - size (in bytes)
code()
a8()
i16()
scope ClearVram: {
    stx.w   VMADD
    sty.w   DAS0

    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.w   VMAIN

    ldy.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.two | (VMDATA << 8)
    sty.w   DMAP0       // also sets BBAD0

    ldx.w   #SnesHeader.EmptyBytes
    stx.w   A1T0
    stz.w   A1B0

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
code()
a8()
i16()
scope ClearCgram: {
    pha

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

    ldx.b   #MDMAEN.dma0
    stx.w   MDMAEN

    plp
    rts
}


// Transfers a block of memory to VRAM
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
//
// INPUT: A:X address
// INPUT: Y   size (in bytes)
code()
a8()
i16()
scope TransferToVram: {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    lda.b   #VMAIN.increment.by1 | VMAIN.incrementMode.high
    sta.w   VMAIN

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
    stx.w   DMAP0

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
code()
a8()
i16()
scope TransferToVramL: {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    lda.b   #VMAIN.increment.by1 | VMAIN.incrementMode.high
    sta.w   VMAIN

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (VMDATAL << 8)
    stx.w   DMAP0

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
code()
a8()
i16()
scope TransferToVramH: {
    stx.w   A1T0
    sta.w   A1B0
    sty.w   DAS0

    lda.b   #VMAIN.increment.by1 | VMAIN.incrementMode.high
    sta.w   VMAIN

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (VMDATAH << 8)
    stx.w   DMAP0

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

