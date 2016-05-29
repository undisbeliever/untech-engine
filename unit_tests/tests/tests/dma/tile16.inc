
scope DmaTest {

scope Tile16 {

constant VRAM_BLOCK(0x6000)


// ::DEBUG::
allocate(_tmp, shadow, 8)
markTmpWord(_tmp)
markTmpWord(_tmp + 2)
markTmpWord(_tmp + 4)

code()
a16()
i16()
//DB=0x7e
Test.add("Dma.Tile16.BufferOneTile")
scope BufferOneTile: {
    constant VRAM_TILE1(VRAM_BLOCK)
    constant VRAM_TILE2(VRAM_BLOCK + 16 * 4)

    jsr     Dma.Init

    lda.w   Dma.transfersLeft
    bne     +
        jsr     WaitFrame
    +

    sep     #$20
a8()
    ldx.w   #Tile1
    lda.b   #Tile1 >> 16
    ldy.w   #VRAM_TILE1

    jsr     Dma.Tile16.BufferOneTile
    bcc     Fail

    ldx.w   #Tile2
    lda.b   #Tile2 >> 16
    ldy.w   #VRAM_TILE2

    jsr     Dma.Tile16.BufferOneTile
    bcc     Fail

    rep     #$30
a16()
    jsr     Dma.TransferOnNextVBlank

    jsr     WaitFrame

    rep     #$30
a16()
i16()
    ldx.w   #Tile1
    ldy.w   #VRAM_TILE1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #Tile2
    ldy.w   #VRAM_TILE2
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts

rodata(rom0)
Tile1:
    fill 32, 0x11
    fill 32, 0x22
    fill 32, 0x33
    fill 32, 0x44

Tile2:
    fill 32, 0xcc
    fill 32, 0xdd
    fill 32, 0xee
    fill 32, 0xff
}


// Check that the tile is loaded into VRAM
//
// INPUT: X = Tile address in rom0
// INPUT: Y = VRAM address
code()
a16()
i16()
scope _CheckTile: {
    allocate(tmp, wram7e, 2)


    sep     #$20
a8()
    lda.b   #INIDISP.force
    sta.l   INIDISP


    rep     #$30
a16()

    sty.w   tmp
    tya
    sta.l   VMADD

    // dummy read
    lda.l   VMDATAREAD

    // Verify First tile

    ldy.w   #32
    -
        lda.l   VMDATAREAD
        cmp.l   TileBankOffset,x
        bne     Fail

        inx
        inx
        dey
        bne     -


    // Second Half
    lda.w   tmp
    clc
    adc.w   #16 * 16
    sta.l   VMADD

    // dummy read
    lda.l   VMDATAREAD

    ldy.w   #32
    -
        lda.l   VMDATAREAD
        cmp.l   TileBankOffset,x
        bne     Fail

        inx
        inx
        dey
        bne     -

    sec
    rts

Fail:
    clc
    rts


rodata(rom0)
    constant TileBankOffset(pc() & 0xff0000)
code()
}

}
}

// vim: ft=asm ts=4 sw=4 et:

