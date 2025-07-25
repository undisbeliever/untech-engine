// dma/tile16.inc
// ==============
//
// Manages a transfer queue for uploading 16px tiles to VRAM during VBlank.
//
// REQUIRES:
//    the data block DMA_Tile16Data to be defined.
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
namespace Tile16 {

constant TileListData = Project.DMA_Tile16Data

    // Tile list format:
    //    byte: number of tiles
    //    word: (tileAddress >> 7) * nTiles
    //
    // NOTE: This data format is to be accessed with
    //       a `Index+1` index.
    //       (see `src/metasprite/_dataformat.inc`)
    namespace TileListData {
        assert(TileListData & 0xffff != 0)
        struct(TileListData - 1)
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
// LOCATION: Low-RAM, accessible by REGISTER_DB during VBlank.
//
// The table terminates when vblankFunctionAddr == 0
namespace transferQueue {
    assert(TRANSFER_QUEUE_SIZE < (256/2))

    // Function to call in the _VBlank
    // Equal to `nTiles * 2`
    allocate(vblankFunctionAddr, lowram, TRANSFER_QUEUE_SIZE * 2)

    // This value is always 0. Prevents overruns
    allocate(forceEndTransfer, lowram, 2)

    // The word VRAM address to send the tiles
    allocate(wordVramAddress, lowram, TRANSFER_QUEUE_SIZE * 2)
}
constant TRANSFER_QUEUE_INDEX_END = TRANSFER_QUEUE_SIZE * 2
allocate(transferQueueIndex, lowram, 2)


// List of tiles to upload to VRAM
//
// LOCATION: Low-RAM, accessible by REGISTER_DB during VBlank.
namespace tileQueue {
    assert(TILE_QUEUE_SIZE < (256/2))

    // tile location in ROM
    allocate(address, lowram, TILE_QUEUE_SIZE * 2)

    // Tile bank in ROM
    //
    // The bank byte will always point to the upper-half of the address
    // space (banks 0x80-0xff) in order for the queue to access tiles at
    // the end of a 4 MiB ROM.
    //
    // Second byte contains the size of the top half of a 16x16 4bpp tile
    // (used to speedup the VBlank code).
    //
    allocate(bank, lowram, TILE_QUEUE_SIZE * 2)

    // constant variable - speeds up VBlank
    // always equal to TILE_HALF_SIZE
    constant tileHalfSize = bank + 1

    // size of the top half of a 16x16 4bpp tile
    constant TILE_HALF_SIZE = 32 * 2
}
constant TILE_QUEUE_INDEX_END = TILE_QUEUE_SIZE * 2
allocate(tileQueueIndex, lowram, 2)



// Transfers the tiles during VBlank.
allocate(processVBlankOnZero, lowram, 1)


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
}



// Finalize the transfer queue for the current frame and reset it for the next one.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80
macro _FinalizeFrame() {
    assert16a()
    assert8i()

    assert(Dma.Tile16.TRANSFER_QUEUE_INDEX_END < 0xff)
    ldx.w   Dma.Tile16.transferQueueIndex
    beq     +
        // set last vblankFunctionAddr to EndLoopFunction
        stz.w   Dma.Tile16.transferQueue.vblankFunctionAddr,x
        ldx.b   #0xff
    +
    inx

    // X = 0 if transferQueueIndex is non-zero zero, or
    //     1 if transferQueueIndex is zero.
    stx.w   Dma.Tile16.processVBlankOnZero
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
// REQUIRES: VMAIN = VMAIN.incrementMode.high
macro _VBlank_dp4300() {
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
            bra     Return

    EndLoopFunction:
        ply

    Return:
}


// Reset the transfer queues in preparation for the next frame.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x80, screen active
// INPUT: X is zero
// INPUT: Y is non-zero
// KEEP: X, Y
macro _ResetQueue() {
    assert16a()

    stz.w   Dma.Tile16.transferQueueIndex
    stz.w   Dma.Tile16.tileQueueIndex
}


// A Table of functions to process the transfer queues
//
// Uploads the tiles in the tileQueue to VRAM using DMA
//
// REGISTERS: 16 bit A, 16 bit Index, DB = $80, DP = $4300
// INPUT: DMAPx/BBADx set to VMDATA (2 registers), VMAIN set
// INPUT: A - Word address of second half of the tiles
// INPUT: Y = _ProcessDMA.tileQueuePos = current location in the tile Queue
// OUTPUT: _ProcessDMA.tileQueuePos updated to new location
a16()
i16()
farCode()
namespace _ProcessDMA {
    FunctionTable: {
        dw  WaitFrame__far.Dma.VBlank_dp4300.Dma.Tile16._VBlank_dp4300.EndLoopFunction
        dw  Transfer1
        dw  Transfer2
        dw  Transfer3
        dw  Transfer4
        dw  Transfer5
        dw  Transfer6
        dw  Transfer7
        dw  Transfer8
    }

    allocate(tileQueuePos, lowram, 2)

    inline ProcessTiles(evaluate nTiles) {
        // REMEMBER to update DmaTransferTimeTable if you change this routine

        a16()
        i8()
        farCode()
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

            jmp     WaitFrame__far.Dma.VBlank_dp4300.Dma.Tile16._VBlank_dp4300.ContinueLoop
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
rodata()
DmaTransferTimeTable:
namespace DmaTransferTimeTable {

expression _DmaTime_Single(n) = Dma.CalcDmaTime(n * 128, n * 21 + 55 + (n == 1 ? 8 : 12) + (n < 8 ? 17 : 0))
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


    lda.w   Dma.transfersLeft
    sec
    sbc.b   #DmaTransferTimeTable.ONE_TILE
    bcc     Fail
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


    ldy.w   tileQueueIndex

    // bank = (tileList.addr << 7) >> 16 | 0x80
    // address = word(tileList.addr << 7)

    lda.l   TileListData.addr,x
    xba

    sep     #$21
a8()
    // carry set
    ror
    sta.w   tileQueue.bank,y

    rep     #$30
a16()
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


    ldy.w   tileQueueIndex

    // bank = (tileList.addr << 7) >> 16 | 0x80
    // address = word(tileList.addr << 7)

    // First tile
    lda.l   TileListData.addr,x
    xba

    sep     #$21
a8()
    // carry set
    ror
    sta.w   tileQueue.bank,y

    rep     #$30
a16()
    ror
    and.w   #0xff80
    sta.w   tileQueue.address,y



    // Second tile
    lda.l   TileListData.addr + 2,x
    xba

    sep     #$21
a8()
    // carry set
    ror
    sta.w   tileQueue.bank + 2,y

    rep     #$30
a16()
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
    ldx.w   tileListAddr
    ldy.w   tileQueueIndex

    lda.l   TileListData.addr,x

    sep     #$21
    // carry set
a8()
    Loop:
        // Starting the loop with an 8 bit A is faster when processing more than 4 tiles.

        // A = tileListData.addr,x

        // bank = (tileList.addr << 7) >> 16 | 0x80
        // address = word(tileList.addr << 7)
        xba

        // carry set
        ror
        sta.w   tileQueue.bank,y

        rep     #$30
a16()
        ror
        and.w   #0xff80
        sta.w   tileQueue.address,y

        lda.l   TileListData.addr + 2,x

        sep     #$21
a8()
        // carry set

        // MUST NOT MODIFY CARRY

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

