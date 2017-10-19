// dma/tile16.inc
// ==============
//
// Manages a transfer queue for uploading 16px tiles to VRAM during VBlank.
//
// REQUIRES:
//    the data block DMA_Tile16Data to be defined.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Dma {
namespace Tile16 {

rodata(DMA_Tile16Data)

    // Tile list format:
    //    byte: number of tiles
    //    word: (tileAddress >> 7) * nTiles

    namespace TileListData {
        struct(pc() & 0xff0000)
            field(count, 1)
            field(addr, 2)
            constant addr.h = addr + 1
        endstruct()
    }
code()



constant TRANSFER_QUEUE_SIZE = 15
constant TILE_QUEUE_SIZE = 30

// List of transfers to process during VBlank
//
// LOCATION: shadow WRAM, accessible by REGISTER_DB during VBlank.
//
// The table terminates when vblankFunctionAddr == 0
namespace transferQueue {
    assert(TRANSFER_QUEUE_SIZE < (256/2))

    // Function to call in the _VBlank
    // Equal to `nTiles * 2`
    allocate(vblankFunctionAddr, shadow, TRANSFER_QUEUE_SIZE * 2)

    // This value is always 0. Prevents overruns
    allocate(forceEndTransfer, shadow, 2)

    // The word VRAM address to send the tiles
    allocate(wordVramAddress, shadow, TRANSFER_QUEUE_SIZE * 2)
}
constant TRANSFER_QUEUE_INDEX_END = TRANSFER_QUEUE_SIZE * 2
allocateTmpWord(transferQueueIndex)


// List of tiles to upload to VRAM
//
// LOCATION: shadow WRAM, accessible by REGISTER_DB during VBlank.
namespace tileQueue {
    assert(TILE_QUEUE_SIZE < (256/2))

    // tile location in ROM
    allocate(address, shadow, TILE_QUEUE_SIZE * 2)
    allocate(bank, shadow, TILE_QUEUE_SIZE * 2)

    // constant variable - speeds up VBlank
    // always equal to TILE_HALF_SIZE
    constant tileHalfSize = bank + 1

    // size of the top half of a 16x16 4bpp tile
    constant TILE_HALF_SIZE = 32 * 2
}
constant TILE_QUEUE_INDEX_END = TILE_QUEUE_SIZE * 2
allocateTmpWord(tileQueueIndex)



// Transfers the tiles during VBlank.
allocate(processVBlankOnZero, shadow, 2)


allocateTmpWord(tmp0)

// Parameters of the queue transfer routines
// (word)
allocateTmpWord(tileListAddr)
allocateTmpWord(vramWaddr)
allocateTmpWord(vramWaddr2)



// Initialize the tables/variables
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro _Init() {
    assert16a()
    assert16i()

    stz.w   Dma.Tile16.transferQueue.forceEndTransfer - 1
    stz.w   Dma.Tile16.transferQueueIndex

    lda.w   #Dma.Tile16.tileQueue.TILE_HALF_SIZE << 8
    ldx.w   #Dma.Tile16.TILE_QUEUE_INDEX_END - 2
    -
        sta.w   Dma.Tile16.tileQueue.tileHalfSize - 1,x
        dex
        dex
        bpl     -

    stz.w   Dma.Tile16.tileQueueIndex

    lda.w   #1
    sta.w   Dma.Tile16.processVBlankOnZero
}



// Enable transfer queue on next VBlank.
//
// REQUIRES: 16 bit A, DB = 0x7e
macro _TransferOnNextVBlank() {
    assert16a()

    ldx.w   Dma.Tile16.transferQueueIndex
    stz.w   Dma.Tile16.transferQueue.vblankFunctionAddr,x
    stz.w   Dma.Tile16.processVBlankOnZero

    stz.w   Dma.Tile16.transferQueueIndex
    stz.w   Dma.Tile16.tileQueueIndex
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
// REQUIRES: VMAIN = VMAIN.incrementMode.high
macro _VBlank() {
    assert16a()
    assert8i()
        // REMEMBER to update DmaTransferTime if you change this routine

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
        sty.w   Dma.Tile16._ProcessDMA.tileQueuePos

        ldx.w   Dma.Tile16.transferQueue.vblankFunctionAddr,y

        Loop:
            phy

            lda.w   Dma.Tile16.transferQueue.wordVramAddress,y
            sta.w   VMADD

            ldy.w   Dma.Tile16._ProcessDMA.tileQueuePos

            jmp     (Dma.Tile16._ProcessDMA.FunctionTable,x)


        ContinueLoop:
            ply

            iny
            iny

            ldx.w   Dma.Tile16.transferQueue.vblankFunctionAddr,y
            bne     Loop
            bra     EndLoop

    EndLoopFunction:
        ply

    EndLoop:
        ldx.b   #0xff
        stx.w   Dma.Tile16.processVBlankOnZero

    Return:
}


// A Table of functions to process the transfer queues
//
// Uploads the tiles in the tileQueue to VRAM using DMA
//
// REGISTERS: 16 bit A, 16 bit Index, DB = $80, DP = $4300
// INPUT: DMAPx set to VRAM, VMAIN set
// INPUT: A - Word address of second half of the tiles
// INPUT: Y = _ProcessDMA.tileQueuePos = current location in the tile Queue
// OUTPUT: _ProcessDMA.tileQueuePos updated to new location
a16()
i16()
code()
namespace _ProcessDMA {
    constant ContinueLoop = NmiHandler.Dma.VBlank.Dma.Tile16._VBlank.ContinueLoop
    constant EndLoopFunction = NmiHandler.Dma.VBlank.Dma.Tile16._VBlank.EndLoopFunction

    FunctionTable: {
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

    allocate(tileQueuePos, shadow, 2)

    inline ProcessTiles(evaluate nTiles) {
        // REMEMBER to update DmaTransferTimeTable if you change this routine

        a16()
        i8()
        code()
        function Transfer{nTiles} {
            if {nTiles} < 8 {
                pha
            }

            variable n = {nTiles}
            while n > 0 {
                n = n - 1

                lda.w   tileQueue.address + n * 2,y
                sta.b   A1T0 + n * 16
                lda.w   tileQueue.bank + n * 2,y
                sta.b   A1B0 + n * 16
            }

            if {nTiles} == 1 {
                iny
                iny
                sty.w   tileQueuePos
            } else {
                tya
                clc
                adc.w   #{nTiles} * 2
                sta.w   tileQueuePos
            }

            ldy.b   #(1 << {nTiles}) - 1
            sty.w   MDMAEN

            if {nTiles} < 8 {
                pla
                ora.w   #16 * 16
                sta.w   VMADD
            }

            ldx.b   #tileQueue.TILE_HALF_SIZE

            variable n = {nTiles}
            while n > 0 {
                n = n - 1

                stx.b   DAS0 + n * 16
            }

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


// DMA Time required for each tile count
rodata(rom0)
DmaTransferTimeTable:
namespace DmaTransferTimeTable {

expression _DmaTime_Single(n) = CalcDmaTime(n * 128, n * 21 + 55 + (n == 1 ? 8 : 12) + (n < 8 ? 17 : 0))
expression _DmaTime_Dual(n) = _DmaTime_Single(8) + _DmaTime_Single(n - 8)
expression _DmaTime(n) = n <= 8 ? _DmaTime_Single(n) : _DmaTime_Dual(n)

constant ONE_TILE = _DmaTime(1)
constant TWO_TILES = _DmaTime(2)

    db  0
    variable n = 1
    while n <= 16 {
        db  _DmaTime(n)
        n = n + 1
    }

assert(pc() - DmaTransferTimeTable == 16 + 1)
}


// Queues one 16x16px tile from a tile list
//
// REQUIRES: DB = $7E
//
// ASSUMES: The number of tiles in the list is 1
//          Will always transfer one tile
//
// INPUT:
//      tileListAddr - the address of the tile list (within `TileListBank`)
//      vramWaddr - the VRAM word address (MUST on an even VRAM tile row)
//
// OUTPUT: Changes register size
//
// OUTPUT: C set if successful
// OUTPUT: Changes register size to 8 bit A, 16 bit Index
au()
iu()
code()
function QueueOneTileFromList {
    rep     #$30
    sep     #$20
a8()
i16()

    ldx.w   tileListAddr

AfterLoadTileset:

    // Check if we can upload 1 tile

    lda.w   tileQueueIndex
    cmp.b   #TILE_QUEUE_INDEX_END
    bcs     Fail

    ldy.w   transferQueueIndex
    cpy.w   #TRANSFER_QUEUE_INDEX_END
    bcs     Fail


    assert(DmaTransferTimeTable.ONE_TILE == 2)
    lda.w   Dma.transfersLeft
    dec
    dec
    bmi     Fail
    sta.w   Dma.transfersLeft


    // Y = transferQueueIndex

    lda.b   #1 << 1
    sta.w   transferQueue.vblankFunctionAddr,y

    rep     #$30
a16()
    lda.w   vramWaddr
    sta.w   transferQueue.wordVramAddress,y

    iny
    iny
    sty.w   transferQueueIndex


    sep     #$20
a8()
    ldy     tileQueueIndex

    // bank = (tileList.addr << 7) >> 16
    // address = word(tileList.addr << 7)
    lda.l   TileListData.addr.h,x
    lsr
    sta.w   tileQueue.bank,y

    lda.l   TileListData.addr,x

    rep     #$30
a16()
    xba
    ror
    and.w   #0xff80
    sta.w   tileQueue.address,y



    sep     #$21
a8()
    iny
    iny
    sty.w   tileQueueIndex

    // carry set (iny does not affect carry)
    rts

Fail:
    clc
    rts

}


// Queues two 16x16px tiles from a tile list to two different vram Addresses
//
// NOTE: Will queue a MAXIMUM of 2 tiles.
//
// REQUIRES: DB = $7E
//
// INPUT:
//      tileListAddr - the address of the tile list (within `TileListBank`)
//      vramWaddr - the VRAM word address (MUST on an even VRAM tile row)
//      vramWaddr2 - the second VRAM word address (MUST on an even VRAM tile row)
//
// OUTPUT: C set if successful
// OUTPUT: Changes register size to 8 bit A, 16 bit Index
au()
iu()
code()
function QueueTwoTilesFromList {
constant Fail = QueueOneTileFromList.Fail
    rep     #$30
    sep     #$20
a8()
i16()

    ldx.w   tileListAddr
    lda.l   TileListData.count,x
    dec
    beq     QueueOneTileFromList.AfterLoadTileset


    // Check if we can upload 2 tiles
    lda.w   tileQueueIndex
    cmp.b   #TILE_QUEUE_INDEX_END - 2
    bcs     Fail

    ldy.w   transferQueueIndex
    cpy.w   #TRANSFER_QUEUE_INDEX_END - 2
    bcs     Fail


    lda.w   Dma.transfersLeft
    sec
    sbc.b   #DmaTransferTimeTable.TWO_TILES
    bcc     Fail
    sta.w   Dma.transfersLeft


    // Y = transferQueueIndex
    // X = tileListAddr

    lda.b   #1 << 1
    sta.w   transferQueue.vblankFunctionAddr,y
    sta.w   transferQueue.vblankFunctionAddr + 2,y

    rep     #$31
a16()
    lda.w   vramWaddr
    sta.w   transferQueue.wordVramAddress,y
    lda.w   vramWaddr2
    sta.w   transferQueue.wordVramAddress + 2,y

    tya
    // carry clear
    adc.w   #4
    sta.w   transferQueueIndex

    sep     #$20
a8()
    ldy     tileQueueIndex

    // bank = (tileList.addr << 7) >> 16
    // address = word(tileList.addr << 7)

    // First tile

    lda.l   TileListData.addr.h,x
    lsr
    sta.w   tileQueue.bank,y

    lda.l   TileListData.addr,x

    rep     #$30
a16()
    xba
    ror
    and.w   #0xff80
    sta.w   tileQueue.address,y



    // Second tile
    sep     #$20
a8()
    lda.l   TileListData.addr + 2 + 1,x
    lsr
    sta.w   tileQueue.bank + 2,y

    lda.l   TileListData.addr + 2,x

    rep     #$30
a16()
    xba
    ror
    and.w   #0xff80
    sta.w   tileQueue.address + 2,y



    tya
    clc
    adc.w   #4
    sta.w   tileQueueIndex

    sep     #$21
a8()
    // c set
    rts
}


// Queues a list of 16x16px tiles at the end of a vram row.
//
// NOTE: Will queue a MAXIMUM of 8 tiles
//
// REQUIRES: DB = $7e
//
// INPUT:
//      tileListAddr - the address of the tile list (within `TileListBank`)
//      vramWaddr - the VRAM word address (MUST on an even VRAM tile row)
//
// OUTPUT: C set if successful
// OUTPUT: Changes register size to 8 bit A, 16 bit Index
au()
iu()
code()
function QueueListAtEndOfRow {
    rep     #$30
    sep     #$20
a8()
i16()

    ldx.w   tileListAddr

    // ensure 1 <= nTiles <= 8
    lda.l   TileListData.count,x
    dec
    and.b   #7
    inc

NTilesA:
    tay

    // vramWaddr |= (8 - nTiles) * 32

    eor.b   #7
    inc

    asl
    asl
    asl
    asl
    asl
    ora.w   vramWaddr
    sta.w   vramWaddr

    tya
    bra     QueueOneRowFromList.NTilesA
}


// Queues a single row of 16x16px tiles from a tile list
//
// NOTE: Will queue a MAXIMUM of 8 tiles
//
// REQUIRES: DB = $7E
//
// INPUT:
//      tileListAddr - the address of the tile list (within `TileListBank`)
//      vramWaddr - the VRAM word address (MUST on an even VRAM tile row)
//
// OUTPUT: C set if successful
// OUTPUT: Changes register size to 8 bit A, 16 bit Index
au()
iu()
code()
function QueueOneRowFromList {
constant tmp_nTiles = tmp0
    rep     #$30
    sep     #$20
a8()
i16()

    ldx.w   tileListAddr

    // ensure 1 <= nTiles <= 8
    lda.l   TileListData.count,x
    dec
    and.b   #7
    inc

NTilesA:
    sta.w   tmp_nTiles
    tax

    asl
    // C clear
    adc.w   tileQueueIndex
    cmp.b   #TILE_QUEUE_INDEX_END
    bcs     Fail


    sep     #$30
i8()
    // X = tmp_nTiles
    lda.w   Dma.transfersLeft
    sec
    sbc.l   DmaTransferTimeTable,x
    bcc     Fail
    sta.w   Dma.transfersLeft

    rep     #$10
i16()


    ldy.w   transferQueueIndex
    cpy.w   #TRANSFER_QUEUE_INDEX_END
    bcs     Fail


    lda.w   tmp_nTiles
    asl
    sta.w   transferQueue.vblankFunctionAddr,y


    rep     #$30
a16()
    lda.w   vramWaddr
    sta.w   transferQueue.wordVramAddress,y

    iny
    iny
    sty.w   transferQueueIndex


BuildDmaQueue:
    sep     #$20
a8()

    ldx.w   tileListAddr
    ldy.w   tileQueueIndex

    Loop:
        // bank = (tileList.addr << 7) >> 16
        // address = word(tileList.addr << 7)

        lda.l   TileListData.addr.h,x
        lsr
        sta.w   tileQueue.bank,y

        lda.l   TileListData.addr,x

        rep     #$30
a16()
        xba
        ror
        and.w   #0xff80
        sta.w   tileQueue.address,y

        sep     #$20
a8()
        inx
        inx

        iny
        iny

        dec.w   tmp_nTiles
        bne     Loop


    sty.w   tileQueueIndex
    sec
    rts


Fail:
    rep     #$11
    // c clear
    rts
}


// Queues two rows of 16x16px tiles from a tile list
//
// NOTE: Will queue a MAXIMUM of 16 tiles
//
// REQUITES: DB = $7E
//
// INPUT:
//      tileListAddr - the address of the tile list (within `TileListBank`)
//      vramWaddr - the VRAM word address (MUST on an even VRAM tile row)
//      vramWaddr2 - the VRAM word address of the second row (MUST on an even VRAM tile row)
//
// OUTPUT: C set if successful
// OUTPUT: Changes register size to 8 bit A, 16 bit Index
au()
iu()
code()
function QueueTwoRowsFromList {
constant tmp_nTiles = QueueOneRowFromList.tmp_nTiles
constant Fail = QueueOneRowFromList.Fail

    rep     #$30
    sep     #$20
a8()
i16()

    ldx.w   tileListAddr

    lda.l   TileListData.count,x
    cmp.b   #8 + 1
    bcc     QueueOneRowFromList.NTilesA


    // ensure 8 <= nTiles <= 16
    dec
    and.b   #15
    inc
    sta.w   tmp_nTiles
    tax

    asl
    // C clear
    adc.w   tileQueueIndex
    cmp.b   #TILE_QUEUE_INDEX_END
    bcs     Fail


    sep     #$30
i8()
    // X = tmp_nTiles

    lda.w   Dma.transfersLeft
    sec
    sbc.l   DmaTransferTimeTable,x
    bcc     Fail
    sta.w   Dma.transfersLeft

    rep     #$10
i16()


    ldy.w   transferQueueIndex
    cpy.w   #TRANSFER_QUEUE_INDEX_END - 2
    bcs     Fail


    // Y = transferQueueIndex

    lda.b   #8 << 1
    sta.w   transferQueue.vblankFunctionAddr,y

    lda.w   tmp_nTiles
    sec
    sbc.b   #8
    asl
    sta.w   transferQueue.vblankFunctionAddr + 2,y


    rep     #$31
a16()
    lda.w   vramWaddr
    sta.w   transferQueue.wordVramAddress,y
    lda.w   vramWaddr2
    sta.w   transferQueue.wordVramAddress + 2,y

    tya
    // carry clear
    adc.w   #4
    sta.w   transferQueueIndex

    jmp     QueueOneRowFromList.BuildDmaQueue
}


// Queues a list of 16x16px tiles at the end of two vram rows.
//
// NOTE: Will queue a MAXIMUM of 8 tiles
//
// REQUIRES: DB = $7e
//
// INPUT:
//      tileListAddr - the address of the tile list (within `TileListBank`)
//      vramWaddr - the VRAM word address (MUST on an even VRAM tile row)
//      vramWaddr2 - the VRAM word address of the second row (MUST on an even VRAM tile row)
//
// OUTPUT: C set if successful
// OUTPUT: Changes register size to 8 bit A, 16 bit Index
au()
iu()
code()
function QueueListAtEndOfTwoRows {
constant tmp_nTiles = QueueOneRowFromList.tmp_nTiles

    rep     #$30
    sep     #$20
a8()
i16()

    ldx.w   tileListAddr

    lda.l   TileListData.count,x
    cmp.b   #8 + 1
    bcs     +
        // nTiles <= 8, load tiles at end of the second row
        ldy.w   vramWaddr2
        sty.w   vramWaddr
        jmp     QueueListAtEndOfRow.NTilesA
    +

    // ensure 1 <= nTiles <= 15
    dec
    and.b   #15
    inc
    sta.w   tmp_nTiles
    tax

    asl
    // C clear
    adc.w   tileQueueIndex
    cmp.b   #TILE_QUEUE_INDEX_END
    bcs     Fail


    sep     #$30
i8()
    // X = tmp_nTiles

    lda.w   Dma.transfersLeft
    sec
    sbc.l   DmaTransferTimeTable,x
    bcc     Fail
    sta.w   Dma.transfersLeft

    rep     #$10
i16()


    ldy.w   transferQueueIndex
    cpy.w   #TRANSFER_QUEUE_INDEX_END - 2
    bcs     Fail


    // Y = transferQueueIndex

    lda.w   tmp_nTiles
    sec
    sbc.b   #8
    asl
    sta.w   transferQueue.vblankFunctionAddr,y

    lda.b   #8 << 1
    sta.w   transferQueue.vblankFunctionAddr + 2,y


    // transferQueue.wordAddress[Y] = vramWaddr | (16 - nTiles) * 32
    // transferQueue.wordAddress[Y + 1] = vramWaddr2

    lda.w   tmp_nTiles
    eor.b   #15

    rep     #$31
a16()
    and.w   #7
    inc
    xba
    lsr
    lsr
    lsr
    ora.w   vramWaddr
    sta.w   transferQueue.wordVramAddress,y

    lda.w   vramWaddr2
    sta.w   transferQueue.wordVramAddress + 2,y

    tya
    // carry clear
    adc.w   #4
    sta.w   transferQueueIndex

    jmp     QueueOneRowFromList.BuildDmaQueue

Fail:
    rep     #$11
    // c clear
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

