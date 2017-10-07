// metatiles/render.inc
// ====================
//
// MetaTile map renderer.
//
// Assumes:
//    * Camera xPos/yPos always inside map
//    * only Renderer edits bg1 ScrollBuffer
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
namespace Render {

allocateTmpWord(tmp0)
allocateTmpWord(tmp1)
allocateTmpWord(tmp2)


// Map index of the top-left most visible cell
// (word index)
allocate(visibleMapIndex, wram7e, 2)


// Display offset of the tilemap
constant displayHoffset = ScrollBuffer.bg1.hOffset
constant displayVoffset = ScrollBuffer.bg1.vOffset



// Draw the visible map directly to the VRAM
//
// ASSUMES: Camera position inside map
//
// REQUIRES: Force Blank
// REQUIRES: Map loaded
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Force Blank
a16()
i16()
code()
function DrawFullScreen_ForceBlank {

constant _mapIndex = tmp0
constant _columnCount = tmp1
constant _rowCount = tmp2

    phd

    // set DP to registers
    lda.w   #0x2100
    tcd

    // Reset map position to top/left of tilemap
    //
    // visibleMapIndex = (Camera.xPos - map.left) / METATILE_SIZE_PX * 2
    //                   + (Camera.yPos - map.top) / METATILE_SIZE_PX * map.width * 2

    lda.w   Camera.yPos
    sec
    sbc.w   map.top
    lsr
    lsr
    lsr
    lsr

    sep     #$20
a8()
    sta.l   WRMPYA

    lda.w   map.width
    sta.l   WRMPYB

    rep     #$30
a16()

    lda.w   Camera.xPos
    sec
    sbc.w   map.left
    lsr
    lsr
    lsr
    lsr

    clc
    adc.l   RDMPY
    asl
    sta.w   visibleMapIndex


    // Draw Tilemap
    //
    // mapIndex = visibleMapIndex
    //
    // VMAIN = incrementMode.high | increment.bg1
    // VMADD = METATILES_MAP_WADDR
    //
    // for row = 0 to METATILE_DISPLAY_HEIGHT + 1:
    //      x = mapIndex
    //      for column = 0 to METATILE_DISPLAY_WIDTH:
    //          VMDATA = tileMap.topLeft[map.data[x]]
    //          VMDATA = tileMap.topRight[map.data[x]]
    //          x++
    //
    //      x = mapIndex
    //      for column = 0 to METATILE_DISPLAY_WIDTH:
    //          VMDATA = tileMap.bottomLeft[map.data[x]]
    //          VMDATA = tileMap.bottomRight[map.data[x]]
    //          x++
    //
    //      mapIndex += map.bytesPerRow


    // A = visibleMapIndex
    tax

    sep     #$20
a8()
    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.b   VMAIN

    assert(METATILE_DISPLAY_WIDTH * METATILE_SIZE == 32)

    rep     #$30
a16()

    lda.w   #METATILES_MAP_WADDR
    sta.b   VMADD

    lda.w   #METATILE_DISPLAY_HEIGHT + 1
    sta.w   _columnCount

    ColumnLoop:
        stx.w   _mapIndex

        lda.w   #METATILE_DISPLAY_WIDTH
        sta.w   _rowCount

        TopLineLoop:
            ldy.w   map.data,x

            lda.w   tileMap.topLeft,y
            sta.b   VMDATA

            lda.w   tileMap.topRight,y
            sta.b   VMDATA

            inx
            inx

            dec.w   _rowCount
            bne     TopLineLoop


        ldx.w   _mapIndex

        lda.w   #METATILE_DISPLAY_WIDTH
        sta.w   _rowCount

        BottomLineLoop:
            ldy.w   map.data,x

            lda.w   tileMap.bottomLeft,y
            sta.b   VMDATA

            lda.w   tileMap.bottomRight,y
            sta.b   VMDATA

            inx
            inx

            dec.w   _rowCount
            bne     BottomLineLoop

        lda.w   _mapIndex
        clc
        adc.w   map.bytesPerRow
        tax

        dec.w   _columnCount
        bne     ColumnLoop


    // Draw rightmost column
    //
    // VMAIN = incrementMode.high | increment.bg32
    //
    // VMADD = METATILES_MAP_WADDR + 32 * 32
    // x = visibleMapIndex + DISPLAY_WIDTH
    // for row = 0 to METATILE_DISPLAY_HEIGHT + 1:
    //      VMDATA = tileMap.topLeft[map.data[x]]
    //      VMDATA = tileMap.bottomLeft[map.data[x]]
    //      x += map.bytesPerRow
    //
    // VMADD = METATILES_MAP_WADDR + 32 * 32 + 1
    // x = visibleMapIndex + DISPLAY_WIDTH
    // for row = 0 to METATILE_DISPLAY_HEIGHT + 1:
    //      VMDATA = tileMap.topRight[map.data[x]]
    //      VMDATA = tileMap.bottomRight[map.data[x]]
    //      x += map.bytesPerRow
    //

    sep     #$20
a8()
    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by32
    sta.b   VMAIN

    rep     #$30
a16()

    lda.w   #METATILES_MAP_WADDR + 32 * 32
    sta.b   VMADD

    lda.w   #METATILE_DISPLAY_HEIGHT + 1
    sta.w   _columnCount

    lda.w   visibleMapIndex
    clc
    adc.w   #METATILE_DISPLAY_WIDTH * 2
    pha

    LeftLoop:
        tax

        ldy.w   map.data,x

        lda.w   tileMap.topLeft,y
        sta.b   VMDATA

        lda.w   tileMap.bottomLeft,y
        sta.b   VMDATA

        txa
        clc
        adc.w   map.bytesPerRow

        dec.w   _columnCount
        bne     LeftLoop


    lda.w   #METATILES_MAP_WADDR + 32 * 32 + 1
    sta.b   VMADD

    lda.w   #METATILE_DISPLAY_HEIGHT + 1
    sta.w   _columnCount

    pla
    RightLoop:
        tax

        ldy.w   map.data,x

        lda.w   tileMap.topRight,y
        sta.b   VMDATA

        lda.w   tileMap.bottomRight,y
        sta.b   VMDATA

        txa
        clc
        adc.w   map.bytesPerRow

        dec.w   _columnCount
        bne     RightLoop



    // Update Scroll Buffer
    //
    // displayHoffset = Camera.xPos & (METATILE_SIZE_PX - 1)
    // displayVoffset = Camera.yPos & (METATILE_SIZE_PX - 1) - 1

    lda.w   Camera.xPos
    and.w   #METATILE_SIZE_PX - 1
    sta.w   displayHoffset

    lda.w   Camera.yPos
    and.w   #METATILE_SIZE_PX - 1
    dec
    sta.w   displayVoffset

    pld

    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

