// tile16.inc
// ==========
//
// Manages a buffer to upload 16px tiles to VRAM.
//
// REQUIRES:
//    the data block DMA_Tile16Data to be defined.
//

scope Dma {
scope Tile16 {

rodata(DMA_Tile16Data)
    constant TileListOffset(pc() & 0xff0000)
code()

constant BUFFER_SIZE(30)
constant TABLE_ENTRIES(15)

// Transfers the tiles during VBlank.
allocate(processVBlankOnZero, shadow, 2)


allocateTmpWord(tmp0)
allocateTmpWord(tmp1)


// SOA table for handling transfer information.
// LOCATION: shadow WRAM, accessible by REGISTER_DB during VBlank.
//
// The table terminates when dmaBufferSize == 0
scope table {
    assert(TABLE_ENTRIES < (256/2))

    // Function to call in the _VBlank
    // Equal to `nTiles * 2`
    allocate(vblankFunctionAddr, shadow, TABLE_ENTRIES * 2)

    // This value is always 0. Prevent buffer overruns
    allocate(forceEndBuffer, shadow, 2)

    // The word VRAM address to send the tiles
    allocate(wordVramAddress, shadow, TABLE_ENTRIES * 2)
}
constant TABLE_INDEX_END(TABLE_ENTRIES * 2)
allocateTmpWord(tableIndex)


// SOA data to load into DMA
//
// LOCATION: shadow WRAM, accessible by REGISTER_DB during VBlank.
scope dmaBuffer {
    assert(BUFFER_SIZE < (256/2))

    allocate(tileAddress, shadow, BUFFER_SIZE * 2)
    allocate(tileBank, shadow, BUFFER_SIZE * 2)

    // constant variable - speeds up VBlank
    // always equal to TILE_HALF_SIZE
    constant tileHalfSize(tileBank + 1)

    // size of the top half of a 16x16 4bpp tile
    constant TILE_HALF_SIZE(32 * 2)
}
constant DMA_BUFFER_INDEX_END(BUFFER_SIZE * 2)
allocateTmpWord(dmaBufferIndex)


// Initialize the tables/variables
macro _Init() {
    assert16a()
    assert16i()

    scope Tile16: {
        stz.w   Dma.Tile16.table.forceEndBuffer - 1
        stz.w   Dma.Tile16.tableIndex

        lda.w   #Dma.Tile16.dmaBuffer.TILE_HALF_SIZE << 8
        ldx.w   #Dma.Tile16.DMA_BUFFER_INDEX_END - 2
        -
            sta.w   Dma.Tile16.dmaBuffer.tileHalfSize - 1,x
            dex
            dex
            bpl     -

        stz.w   Dma.Tile16.dmaBufferIndex

        lda.w   #1
        sta.w   Dma.Tile16.processVBlankOnZero
    }
}


macro _TransferOnNextVBlank() {
    assert16a()

    scope Tile16: {
        ldx.w   Dma.Tile16.tableIndex
        stz.w   Dma.Tile16.table.vblankFunctionAddr,x
        stz.w   Dma.Tile16.processVBlankOnZero

        stz.w   Dma.Tile16.tableIndex
        stz.w   Dma.Tile16.dmaBufferIndex
    }
}


// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
// REQUIRES: VMAIN = VMAIN.incrementMode.high
macro _VBlank() {
    assert16a()
    assert8i()

    scope Tile16: {
        ldy.w   Dma.Tile16.processVBlankOnZero
        bne     Return

        lda.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
        sta.b   DMAP0
        sta.b   DMAP1
        sta.b   DMAP2
        sta.b   DMAP3
        sta.b   DMAP4
        sta.b   DMAP5
        sta.b   DMAP6
        sta.b   DMAP7       // also sets BBAD

        // Y = 0
        sty.b   DAS0H
        sty.b   DAS1H
        sty.b   DAS2H
        sty.b   DAS3H
        sty.b   DAS4H
        sty.b   DAS5H
        sty.b   DAS6H
        sty.b   DAS7H

        // Y = 0
        sty.w   Dma.Tile16._ProcessDMA.bufferIndex

        ldx.w   Dma.Tile16.table.vblankFunctionAddr,y

        Loop:
            phy

            lda.w   Dma.Tile16.table.wordVramAddress,y
            sta.w   VMADD

            ldy.w   Dma.Tile16._ProcessDMA.bufferIndex

            jmp     (Dma.Tile16._ProcessDMA.FunctionTable,x)


        constant ::Dma.Tile16.ContinueLoop(Continue)
        Continue:
            ply

            iny
            iny

            ldx.w   Dma.Tile16.table.vblankFunctionAddr,y
            bne     Loop
            bra     EndLoop

    constant ::Dma.Tile16.EndLoopFunction(EndLoopFunction)
    EndLoopFunction:
        ply

    EndLoop:
        ldx.b   #0xff
        stx.w   Dma.Tile16.processVBlankOnZero

    Return:
    }
}

// A Table of functions to process the DMA buffers
//
// Uploads the tiles in the table/buffer to VRAM using DMA
//
// REGISTERS: 16 bit A, 16 bit Index, DB = $80, DP = $4300
// INPUT: DMAPx set to VRAM, VMAIN set
// INPUT: A - Word address of second half of the tiles
// INPUT: Y = _ProcessDMA.bufferIndex current location in the DMA buffer
// OUTPUT: _ProcessDMA.bufferIndex updated to new location
a16()
i16()
code()
scope _ProcessDMA {
    constant ContinueLoop(::Dma.Tile16.ContinueLoop)
    constant EndLoopFunction(::Dma.Tile16.EndLoopFunction)

scope FunctionTable: {
    dw  EndLoopFunction
    dw  Transfer1
    dw  Transfer2
    dw  Transfer3
    dw  Transfer4
    dw  Transfer5
    dw  Transfer6
    dw  Transfer7
    dw  Transfer8
}

    allocate(bufferIndex, shadow, 2)

    macro _AddrSize(variable n) {
        if n > 0 {
            n = n - 1

            lda.w   dmaBuffer.tileAddress + n * 2,y
            sta.b   A1T0 + n * 16
            lda.w   dmaBuffer.tileBank + n * 2,y
            sta.b   A1B0 + n * 16

            _AddrSize(n)
        }
    }

    macro _StxDAS0(variable n) {
        if n > 0 {
            n = n - 1

            stx.b   DAS0 + n * 16

            _StxDAS0(n)
        }
    }

    macro ProcessTiles(evaluate nTiles) {
        a16()
        i8()

        scope Transfer{nTiles}: {
            if {nTiles} < 8 {
                pha
            }

            _AddrSize({nTiles})

            if {nTiles} == 1 {
                iny
                iny
                sty.w   bufferIndex
            } else {
                tya
                clc
                adc.w   #{nTiles} * 2
                sta.w   bufferIndex
            }

            ldy.b   #(1 << {nTiles}) - 1
            sty.w   MDMAEN

            if {nTiles} < 8 {
                pla
                ora.w   #16 * 16
                sta.w   VMADD
            }

            ldx.b   #dmaBuffer.TILE_HALF_SIZE
            _StxDAS0({nTiles})

            sty.w   MDMAEN

            jmp     ContinueLoop
        }
    }

    ProcessTiles(1)
    ProcessTiles(2)
    ProcessTiles(3)
    ProcessTiles(4)
    ProcessTiles(5)
    ProcessTiles(6)
    ProcessTiles(7)
    ProcessTiles(8)
}


// Buffers one 16x16px tile transfer for VBlank
//
// REQUIRES: 8 bit A, 16 bit Index, DB = $7E
//
// INPUT: A:X the tile address
// INPUT: Y the VRAM word address
//
// OUTPUT: C set if successful
code()
a8()
i16()
scope BufferOneTile: {
    sty.w   tmp0

    ldy.w   Dma.transfersLeft
    beq     Fail8

    ldy.w   dmaBufferIndex
    cpy.w   #DMA_BUFFER_INDEX_END
    bcs     Fail8

    sta.w   dmaBuffer.tileBank,y

    rep     #$30
a16()
    txa
    sta.w   dmaBuffer.tileAddress + 0,y


    ldx.w   tableIndex
    cpx.w   #TABLE_INDEX_END
    bcs     Fail16

    lda.w   tmp0
    sta.w   table.wordVramAddress,x

    lda.w   #1 * 2
    sta.w   table.vblankFunctionAddr,x


    inx
    inx
    stx.w   tableIndex

    iny
    iny
    sty.w   dmaBufferIndex

    dec.w   Dma.transfersLeft

    sep     #$21
    // C set
a8()
    rts

Fail16:
    sep     #$20
Fail8:
    clc
a8()
    rts
}


// Buffers many 16x16px tiles for a transfer during VBlank
//
// Tile list format:
//  * byte: n tiles
//    * word: (tileAddress >> 7) * n tiles
//
// REQUIRES: 8 bit A, 16 bit Index, DB = $7E
//
// INPUT:
//      X - the address of the tile list data (Within `TileListBank`)
//      Y - the VRAM word address (MUST on an even VRAM tile row)
//
// OUTPUT: C set if successful
code()
a8()
i16()
scope BufferTileList: {
    lda.l   TileListOffset,x
    inx
End:
}




// Buffers many 16x16px tiles from a list of a given size
//
// REQUIRES: 8 bit A, 16 bit Index, DB = $7E
//
// INPUT:
//      X - the address of the tile list (within `TileListBank`)
//      Y - the VRAM word address (MUST on an even VRAM tile row)
//      A - the number of tiles to render
//
// Tile list is a word list, containing (tileAddresses >> 7)
//
// OUTPUT: C set if successful
a8()
i16()
scope BufferTileListGivenSize: {
constant tmpVram(tmp0)
constant tmpNtiles(tmp1)

    assert(pc() == BufferTileList.End)

    sty.w   tmpVram

    // assert 1 <= a <= 8
    dec
    and.b   #7
    inc
    sta.w   tmpNtiles

    asl
    // C clear
    adc.w   dmaBufferIndex
    cmp.b   #DMA_BUFFER_INDEX_END
    bcs     Fail8


    ldy.w   tableIndex
    cpy.w   #TABLE_INDEX_END
    bcs     Fail8


    lda.w   Dma.transfersLeft
    sec
    sbc.w   tmpNtiles
    bcc     Fail8
    sta.w   Dma.transfersLeft


    lda.w   tmpNtiles
    asl
    sta.w   table.vblankFunctionAddr,y

    rep     #$30
a16()
    lda.w   tmpVram
    sta.w   table.wordVramAddress,y

    sep     #$20
a8()

    iny
    iny
    sty     tableIndex

    ldy     dmaBufferIndex

    Loop:
        // tileBank = tileList << 7 >> 16
        // tileAddress = word(tileList << 7)

        lda.l   TileListOffset + 1,x
        lsr
        sta.w   dmaBuffer.tileBank,y

        lda.l   TileListOffset,x

        rep     #$30
a16()
        xba
        ror
        and.w   #0xff80
        sta.w   dmaBuffer.tileAddress,y

        sep     #$20
a8()

        inx
        inx

        iny
        iny

        dec.w   tmpNtiles
        bne     Loop


    sty.w   dmaBufferIndex

    sec
    rts

Fail8:
    clc
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

