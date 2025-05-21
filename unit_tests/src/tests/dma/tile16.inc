// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace DmaTest {
namespace Tile16 {

constant VRAM_BLOCK = 0x6000


a16()
i16()
code()
Test.add("Dma.Tile16.QueueOneTileFromList")
function QueueOneTileFromList {
    constant VRAM_1 = VRAM_BLOCK + 32 * 2

    jsr     _Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList2
    stx.w   Dma.Tile16.tileListAddr

    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr

    jsr     Dma.Tile16.QueueOneTileFromList
    bcc     Fail
au()

    rep     #$30
a16()

    jsl     _ProcessDma16Queue__far

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_1 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Dma.Tile16.QueueTwoTilesFromList")
function QueueTwoTilesFromList {
    constant VRAM_1 = VRAM_BLOCK + 32 * 7
    constant VRAM_2 = VRAM_BLOCK + 32 * 6

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList10
    stx.w   Dma.Tile16.tileListAddr

    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr

    ldy.w   #VRAM_2
    sty.w   Dma.Tile16.vramWaddr2

    jsr     Dma.Tile16.QueueTwoTilesFromList
    bcc     Fail
au()

    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far


    ldx.w   #TileData.Tile7
    ldy.w   #VRAM_1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile6
    ldy.w   #VRAM_2
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


//DB=0x7e
a16()
i16()
code()
Test.add("Dma.Tile16.QueueOneRowFromList")
function QueueOneRowFromList {
    constant VRAM_1 = VRAM_BLOCK

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList3
    stx.w   Dma.Tile16.tileListAddr
    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr

    jsr     Dma.Tile16.QueueOneRowFromList
    bcc     Fail
au()

    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far

    ldx.w   #TileData.Tile4
    ldy.w   #VRAM_1 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile5
    ldy.w   #VRAM_1 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile6
    ldy.w   #VRAM_1 + 32 * 2
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Dma.Tile16.QueueTwoRowsFromList (one row)")
function QueueTwoRowsFromList_one {
    constant VRAM_1 = VRAM_BLOCK + 32 * 4

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList2
    stx.w   Dma.Tile16.tileListAddr

    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr
    ldy.w   #VRAM_1 + 32 * 16
    sty.w   Dma.Tile16.vramWaddr2

    jsr     Dma.Tile16.QueueTwoRowsFromList
    bcc     Fail
au()


    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_1 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_1 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Dma.Tile16.QueueTwoRowsFromList (two rows)")
function QueueTwoRowsFromList_two {
    constant VRAM_1 = VRAM_BLOCK
    constant VRAM_2 = VRAM_BLOCK + 3 * 32 * 16

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList10
    stx.w   Dma.Tile16.tileListAddr

    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr
    ldy.w   #VRAM_2
    sty.w   Dma.Tile16.vramWaddr2

    jsr     Dma.Tile16.QueueTwoRowsFromList
    bcc     Fail
au()


    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far

    rep     #$30
a16()
i16()
    ldx.w   #TileData.Tile7
    ldy.w   #VRAM_1 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile6
    ldy.w   #VRAM_1 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile5
    ldy.w   #VRAM_1 + 32 * 2
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile4
    ldy.w   #VRAM_1 + 32 * 3
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_1 + 32 * 4
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_1 + 32 * 5
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_1 + 32 * 6
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile0
    ldy.w   #VRAM_1 + 32 * 7
    jsr     _CheckTile
    bcc     Fail

    // Check Second Row

    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_2 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_2 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


//DB=0x7e
a16()
i16()
code()
Test.add("Dma.Tile16.QueueListAtEndOfRow")
function QueueListAtEndOfRow {
    constant VRAM_1 = VRAM_BLOCK + 10 * 32 * 16

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList5
    stx.w   Dma.Tile16.tileListAddr
    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr

    jsr     Dma.Tile16.QueueListAtEndOfRow
    bcc     Fail
au()


    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far

    ldx.w   #TileData.Tile4
    ldy.w   #VRAM_1 + 32 * 7
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_1 + 32 * 6
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_1 + 32 * 5
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_1 + 32 * 4
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile0
    ldy.w   #VRAM_1 + 32 * 3
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}




a16()
i16()
code()
Test.add("Dma.Tile16.QueueListAtEndOfTwoRows (one row)")
function QueueListAtEndOfTwoRows_one {
    constant VRAM_1 = VRAM_BLOCK +  8 * 32 * 16
    constant VRAM_2 = VRAM_BLOCK + 14 * 32 * 16

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far     // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList2
    stx.w   Dma.Tile16.tileListAddr

    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr
    ldy.w   #VRAM_2
    sty.w   Dma.Tile16.vramWaddr2

    jsr     Dma.Tile16.QueueListAtEndOfTwoRows
    bcc     Fail
au()


    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_2 + 32 * 6
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_2 + 32 * 7
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Dma.Tile16.QueueListAtEndOfTwoRows (two rows)")
function QueueListAtEndOfTwoRows_two {
    constant VRAM_1 = VRAM_BLOCK + 12 * 32 * 16
    constant VRAM_2 = VRAM_BLOCK +  6 * 32 * 16

    jsr     Dma.Init
    jsl     _ProcessDma16Queue__far      // Reset Dma.transfersLeft

    ldx.w   #TileData.TileList10
    stx.w   Dma.Tile16.tileListAddr

    ldy.w   #VRAM_1
    sty.w   Dma.Tile16.vramWaddr
    ldy.w   #VRAM_2
    sty.w   Dma.Tile16.vramWaddr2

    jsr     Dma.Tile16.QueueListAtEndOfTwoRows
    bcc     Fail
au()


    rep     #$30
a16()
    jsl     _ProcessDma16Queue__far

    ldx.w   #TileData.Tile7
    ldy.w   #VRAM_1 + 32 * 6
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile6
    ldy.w   #VRAM_1 + 32 * 7
    jsr     _CheckTile
    bcc     Fail

    // Check Second Row

    ldx.w   #TileData.Tile5
    ldy.w   #VRAM_2 + 32 * 0
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile4
    ldy.w   #VRAM_2 + 32 * 1
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile3
    ldy.w   #VRAM_2 + 32 * 2
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_2 + 32 * 3
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_2 + 32 * 4
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile0
    ldy.w   #VRAM_2 + 32 * 5
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile1
    ldy.w   #VRAM_2 + 32 * 6
    jsr     _CheckTile
    bcc     Fail

    ldx.w   #TileData.Tile2
    ldy.w   #VRAM_2 + 32 * 7
    jsr     _CheckTile
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


// Check that the tile is loaded into VRAM
//
// INPUT: X = Tile address in rom0
// INPUT: Y = VRAM address
a16()
i16()
code()
function _CheckTile {
    allocate(tmp, wram7e, 2)

    // Using INIDISP instead of ForceBlank as we do not want to process the
    // normal Dma16 queue.
    sep     #$20
a8()
    lda.b   #INIDISP.force
    sta.l   INIDISP

    rep     #$20
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

    jsr     EnableDisplay_Full

    sec
    rts

Fail:
    clc
    rts


code()
}


namespace TileData {

rodata(rom0)
// padding
if pc() & 0x7f != 0 {
    fill 0x80 - (pc() & 0x7f)
}
assert(pc() & 0x7f == 0)


    constant TileBankOffset = pc() & 0xff0000


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


rodata(rom0)
DMA_Tile16Data:
constant TileList2 = pc() - DMA_Tile16Data + 1
    db  2
    dw  Tile2 >> 7
    dw  Tile3 >> 7


constant TileList3 = pc() - DMA_Tile16Data + 1
    db  3
    dw  Tile4 >> 7
    dw  Tile5 >> 7
    dw  Tile6 >> 7


constant TileList5 = pc() - DMA_Tile16Data + 1
    db  5
    dw  Tile0 >> 7
    dw  Tile1 >> 7
    dw  Tile2 >> 7
    dw  Tile3 >> 7
    dw  Tile4 >> 7


constant TileList10 = pc() - DMA_Tile16Data + 1
    db  10
    dw  Tile7 >> 7
    dw  Tile6 >> 7
    dw  Tile5 >> 7
    dw  Tile4 >> 7
    dw  Tile3 >> 7
    dw  Tile2 >> 7
    dw  Tile1 >> 7
    dw  Tile0 >> 7
    dw  Tile1 >> 7
    dw  Tile2 >> 7
}


// The following is an ugly series of hacks to get the Tile16 code to use the test data
// ------------------------------------------------------------------------------------

constant Project.DMA_Tile16Data = TileData.DMA_Tile16Data
include "../../../../src/dma/tile16.inc"


// Initialize the DmaTest.Tile16.Dma.Tile16 module
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function _Init {
    Dma.Tile16._Init()
    rts
}


// Verify the bank word of the tile queue is valid
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro _CheckTileQueueBankWords() {
    assert16a()
    assert16i()

    ldy.w   Dma.Tile16.tileQueueIndex
    beq     SkipLoop

    dey
    dey

    Loop:
        // Dma.Tile16.tileQueue.bank is valid if its MSB is set
        // Dma.Tile16.tileQueue.tileHalfSize is valid if it is equal to TILE_HALF_SIZE

        assert  (Dma.Tile16.tileQueue.bank + 1 == Dma.Tile16.tileQueue.tileHalfSize)

        lda.w   Dma.Tile16.tileQueue.bank,y
        and.w   #0xff80
        cmp.w   #0x80 | (Dma.Tile16.tileQueue.TILE_HALF_SIZE << 8)
        beq     +
            jmp     Test.Fail
        +

        dey
        dey
        bpl     Loop

SkipLoop:
}


// Transfer data in DmaTest.Tile16.Dma.Tile16 queue to VRAM and reset the queue.
//
// REQUIRES: 16 bit A, 16 bit Index
a16()
i16()
farCode()
_ProcessDma16Queue__far:
namespace WaitFrame__far {
    // ::ANNOY namespace statements are required to make Dma.Tile16._VBlank_dp4300() work::

    _CheckTileQueueBankWords()

    php
    phb
    phd

    sep     #$30
    rep     #$20
a16()
i8()
    ldx.b   #0x80
    phx
    plb
// DB = 0x80

    lda.w   #0x4300
    tcd
// DP = 0x4300

    Dma.Tile16._FinalizeFrame()

    ldx.b   #INIDISP.force
    stx.w   INIDISP

    namespace Dma.VBlank_dp4300 {
        Dma.Tile16._VBlank_dp4300()
    }

    ldx.b   #0xf
    stx.w   INIDISP

    Dma.Tile16._ResetQueue()

    pld
    plb
    plp

    rtl
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

