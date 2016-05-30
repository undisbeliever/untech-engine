
scope DmaTest {

scope Tile16 {

constant VRAM_BLOCK(0x6000)


// ::DEBUG::
allocate(_tmp, shadow, 8)
markTmpWord(_tmp)
markTmpWord(_tmp + 2)
markTmpWord(_tmp + 4)
markTmpWord(_tmp + 6)

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
    ldx.w   #TileData.Tile1
    lda.b   #TileData.Tile1 >> 16
    ldy.w   #VRAM_TILE1

    jsr     Dma.Tile16.BufferOneTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    lda.b   #TileData.Tile2 >> 16
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
    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_TILE1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_TILE2
    jsr     _CheckTile
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
//DB=0x7e
Test.add("Dma.Tile16.BufferTileList")
scope BufferTileList: {
    constant VRAM_1(VRAM_BLOCK)
    constant VRAM_2(VRAM_BLOCK + 3 * 32 * 16)

    jsr     Dma.Init

    sep     #$20
a8()
    ldx.w   #TileData.TileList
    ldy.w   #VRAM_1
    jsr     Dma.Tile16.BufferTileList
    bcc     Fail

    ldx.w   #TileData.TileList + 1 + 2 * 3
    ldy.w   #VRAM_2
    lda.b   #3

    jsr     Dma.Tile16.BufferTileListGivenSize
    bcc     Fail


    rep     #$30
a16()
    jsr     Dma.TransferOnNextVBlank

    jsr     WaitFrame

    rep     #$30
a16()
i16()
    ldx.w   #TileData.Tile0
    ldy.w   #VRAM_1 + 32 * 0
    jsr     _CheckTile
    bcs     +
Fail:
        clc
        rts
    +

    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_1 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_1 + 32 * 2
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_1 + 32 * 3
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile4
    ldy.w   #VRAM_1 + 32 * 4
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile5
    ldy.w   #VRAM_1 + 32 * 5
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile6
    ldy.w   #VRAM_1 + 32 * 6
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile7
    ldy.w   #VRAM_1 + 32 * 7
    jsr     _CheckTile
    bcc     Fail

    // Check Second Transfer

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_2 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile4
    ldy.w   #VRAM_2 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile5
    ldy.w   #VRAM_2 + 32 * 2
    jsr     _CheckTile
    bcc     Fail

    sec
    rts
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
        cmp.l   TileData.TileBankOffset,x
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
        cmp.l   TileData.TileBankOffset,x
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


code()
}


scope TileData {

rodata(rom0)
// padding
if pc() & 0x7f != 0 {
    fill 0x80 - (pc() & 0x7f)
}
assert(pc() & 0x7f == 0)


    constant TileBankOffset(pc() & 0xff0000)


Tile0:
    fill 32, 0x11
    fill 32, 0x22
    fill 32, 0x33
    fill 32, 0x44

Tile1:
    fill 32, 0x55
    fill 32, 0x66
    fill 32, 0x77
    fill 32, 0x88

Tile2:
    fill 32, 0x99
    fill 32, 0xaa
    fill 32, 0xbb
    fill 32, 0xcc

Tile3:
    fill 32, 0xdd
    fill 32, 0xee
    fill 32, 0xff
    fill 32, 0x01

Tile4:
    fill 32, 0x01
    fill 32, 0xff
    fill 32, 0xee
    fill 32, 0xdd

Tile5:
    fill 32, 0xcc
    fill 32, 0xbb
    fill 32, 0xaa
    fill 32, 0x99

Tile6:
    fill 32, 0x88
    fill 32, 0x77
    fill 32, 0x66
    fill 32, 0x55

Tile7:
    fill 32, 0x44
    fill 32, 0x33
    fill 32, 0x22
    fill 32, 0x11


rodata(DMA_Tile16Data)
TileList:
    db  8
    dw  Tile0 >> 7
    dw  Tile1 >> 7
    dw  Tile2 >> 7
    dw  Tile3 >> 7
    dw  Tile4 >> 7
    dw  Tile5 >> 7
    dw  Tile6 >> 7
    dw  Tile7 >> 7
}

}
}

// vim: ft=asm ts=4 sw=4 et:

