
scope Dma {

// Counter to manage the amount of VBlank time I have left.
//
// This is reset every NMI interrupt.
//
// SCALE: each value represents transferring 128 bytes during DMA.
allocate(transfersLeft, shadow, 2)

constant TRANSFER_SCALE(128)

// ::TODO get proper numbers::
constant TRANSFERS_PER_NTSC_FRAME(4000 / TRANSFER_SCALE)
constant TRANSFERS_PER_PAL_FRAME(5000 / TRANSFER_SCALE)


// Initialize the counters
//
// REQUIRES: 16 bit A, 16 bit Index, DB access shadow
macro ResetTransfersLeft() {
    assert16a()

    lda.w   STAT77
    and     #STAT78.pal << 8
    beq     +
        lda.w   #Dma.TRANSFERS_PER_PAL_FRAME
        bra     ++
    +
        lda.w   #Dma.TRANSFERS_PER_NTSC_FRAME
    +
    sta.w   Dma.transfersLeft
}


// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank() {
    assert16a()
    assert8i()

    scope {
        ldx.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
        stx.w   VMAIN

        Dma.MetaSprite._VBlank()
        Dma.Palette._VBlank()
        Dma.Tile16._VBlank()

        // Text module is low priority.
        // Must be done last
        Dma.Text._VBlank()

        Dma.ResetTransfersLeft()
    }
}


// Initializes the tables.
//
// This MUST be called before using this module.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
code()
a16()
i16()
scope Init: {
    Palette._Init()
    Tile16._Init()

    rts
}


// Transfer the buffer on the next VBlank Interrupt.
//
// To prevent a glitch no uploads are to be processed until VBlank completed
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
code()
a16()
i16()
scope TransferOnNextVBlank: {
    Palette._TransferOnNextVBlank()
    Tile16._TransferOnNextVBlank()

    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

