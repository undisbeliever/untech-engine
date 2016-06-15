// metasprite.inc
// ==============
//
// Copes the MetaSprite buffer to OAM
//

scope Dma {
scope MetaSprite {

// Preform DMA transfer if zero
// (byte)
allocate(transferOnZero, shadow, 1)


// Decrement transfers left Counter
//
// Done at the start of the frame to ensure we never run out of DMA time
//
// REQUIRES: 16 bit A, DB access shadow
macro _InitLoop() {
    assert16a()

    scope Dma: {
        lda.w   Dma.transfersLeft
        sec
        sbc.w   #(128 * 4 + 128 / 4) / Dma.TRANSFER_SCALE
        sta.w   Dma.transfersLeft
    End:
    }
}


// EndLoop code, enable transfer of oamBuffer to OAM on next VBlank
//
// REQUIRES: 8 bit A, DB access shadow
macro _EndLoop() {
    assert8a()

    scope Dma: {
        stz.w   Dma.MetaSprite.transferOnZero
    EndIf:
    }
}


// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300, DB = REGISTERS
macro _VBlank() {
    assert16a()
    assert8i()

    scope MetaSprite: {
        ldx.w   Dma.MetaSprite.transferOnZero
        bne     EndIf
            stz.w   OAMADD

            lda.w   #DMAP.direction.toPpu | DMAP.transfer.one | (OAMDATA << 8)
            sta.b   DMAP0       // Also set BBAD0

            lda.w   #MetaSprite.Render.oamBuffer
            sta.w   A1T0
            ldx.b   #MetaSprite.Render.oamBuffer >> 16
            stx.b   A1B0

            lda.w   #128 * 4 + 128 / 4
            sta.b   DAS0

            ldy.b   #MDMAEN.dma0
            sty.w   MDMAEN

            sty.w   Dma.MetaSprite.transferOnZero
    EndIf:
    }
}

}
}

// vim: ft=asm ts=4 sw=4 et:
