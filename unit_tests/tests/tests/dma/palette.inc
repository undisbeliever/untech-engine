
scope DmaTest {

scope Palette {

code()
a16()
i16()
Test.add("Dma.Palette.MarkDirty(all)")
scope MarkDirtyTest_all: {
    jsr     Dma.Init

    lda.w   Dma.transfersLeft
    cmp.w   #5
    bcs     +
        jsr     WaitFrame
    +

    // Fill the entirety of the buffer with 0x1f
    lda.w   #0x1f1f
    ldx.w   #256 * 2 - 2
    -
        sta.w   Dma.Palette.buffer,x
        dex
        dex
        bpl     -

    sep     #$20
a8()

    lda.w   Dma.transfersLeft
    pha
        Dma.Palette.MarkDirty(all)
    pla

    // Ensure transfersLeft decreased by 4
    sec
    sbc.b   #4
    cmp.w   Dma.transfersLeft
    bne     Fail


    // Ensure marking dirty again doesn't decrease transfersLeft
    pha
        Dma.Palette.MarkDirty(all)
        Dma.Palette.MarkDirty(tiles)
        Dma.Palette.MarkDirty(objects)
    pla
    cmp.w   Dma.transfersLeft
    bne     Fail


    rep     #$30
a16()
        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame
    sep     #$20
a8()

    lda.b   #0
    ldx.w   #256 * 2
    ldy.w   #0x1f
    jsr     _CheckCgram
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("Dma.Palette.MarkDirty(tiles)")
scope MarkDirtyTest_tiles: {
    jsr     Dma.Init

    lda.w   Dma.transfersLeft
    cmp.w   #5
    bcs     +
        jsr     WaitFrame
    +

    // Fill the first half of the buffer with 0x38
    lda.w   #0x3838
    ldx.w   #128 * 2 - 2
    -
        sta.w   Dma.Palette.buffer,x
        dex
        dex
        bpl     -

    sep     #$20
a8()

    lda.w   Dma.transfersLeft
    pha
        Dma.Palette.MarkDirty(tiles)
    pla

    // Ensure transfersLeft decreased by 2
    dec
    dec
    cmp.w   Dma.transfersLeft
    bne     Fail

    // Ensure marking dirty again doesn't decrease transfersLeft
    pha
        Dma.Palette.MarkDirty(tiles)
    pla
    cmp.w   Dma.transfersLeft
    bne     Fail


    rep     #$30
a16()
        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame
    sep     #$20
a8()

    lda.b   #0
    ldx.w   #128 * 2
    ldy.w   #0x38
    jsr     _CheckCgram
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}



code()
a16()
i16()
Test.add("Dma.Palette.MarkDirty(objects)")
scope MarkDirtyTest_objects: {
    jsr     Dma.Init

    lda.w   Dma.transfersLeft
    cmp.w   #5
    bcs     +
        jsr     WaitFrame
    +

    // Fill the second half of the buffer with 0x42
    lda.w   #0x4242
    ldx.w   #128 * 2 - 2
    -
        sta.w   Dma.Palette.buffer + 128 * 2,x
        dex
        dex
        bpl     -

    sep     #$20
a8()

    lda.w   Dma.transfersLeft
    pha
        Dma.Palette.MarkDirty(objects)
    pla

    // Ensure transfersLeft decreased by 2
    dec
    dec
    cmp.w   Dma.transfersLeft
    bne     Fail

    // Ensure marking dirty again doesn't decrease transfersLeft again
    pha
        Dma.Palette.MarkDirty(objects)
    pla
    cmp.w   Dma.transfersLeft
    bne     Fail


    rep     #$30
a16()
        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame
    sep     #$20
a8()

    lda.b   #128
    ldx.w   #128 * 2
    ldy.w   #0x42
    jsr     _CheckCgram
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


// Verify the buffer of 0x1f is transferred to CGRAM
//
// REQUIRES: 8 bit A, 16 bit Index, DB anywhere
//
// PARAM: A - color Id
// PARAM: X - number of bytes to check
// PARAM: Y - the byte the CGRAM was filled to
//
// RETURN: Carry set if CGRAM OK
a8()
i16()
scope _CheckCgram: {
    allocateTmpWord(tmp)

    sty.w   tmp

    pha
        lda.b   #INIDISP.force
        sta.l   INIDISP
    pla

    sta.l   CGADD

    lda.b   #0x1f
    -
        lda.l   CGDATAREAD
        and.b   #0x7f       // filter out open bus
        cmp.w   tmp
        bne     Fail

        dex
        bne     -


    // Reset color back to testing color
    lda.b   #0
    sta.l   CGADD

    lda.b   #Test.TESTING_COLOR
    sta.l   CGDATA
    lda.b   #Test.TESTING_COLOR >> 8
    sta.l   CGDATA

    sec
    rts

Fail:
    clc
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

