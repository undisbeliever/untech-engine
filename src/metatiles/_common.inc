// metatiles/_common.inc
// =====================
//
// Common MetaTile routines.
//
// Assumes:
//    * Camera xPos/yPos always inside map
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


Warnings.Register(SET_MAP_CELL_QUEUE_FULL, 24, 0, 12)


namespace MetaTiles {


// Setup the room variables
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: room loaded into memory
macro _LoadMap() {
    assert16a()
    assert16i()

    // Use a lookup table to set the map variables whose value depends on the height of the room
    lda.w   Room.Format.Header.height
    and.w   #0xff
    cmp.w   #MetaTiles.MAP_HEIGHT_SMALL + 1
    bcc     +
        ldx.w   #MetaTiles.MapVariablesTable.Large
        bra     ++
    +
        ldx.w   #MetaTiles.MapVariablesTable.Small
    +
    ldy.w   #MetaTiles.MapVariablesTable.firstVariable
    lda.w   #MetaTiles.MapVariablesTable.size - 1
    mvn     0x7e=(MetaTiles.MapVariablesTable.Small >> 16)


    // RDMPY = map.width * map.bytesPerColumn
    assert(MetaTiles.MAP_HEIGHT_LARGE < 256)
    lda.w   MetaTiles.map.width - 1
    and.w   #0xff00
    ora.w   MetaTiles.map.bytesPerColumn
    sta.l   WRMPYA                      // also set WRMPYB


    // Set map boundary
    assert(1 << 4 == MetaTiles.METATILE_SIZE_PX)
    lda.w   MetaTiles.map.width
    and.w   #0xff
    asl
    asl
    asl
    asl
    // Carry clear
    adc.w   #MetaTiles.map.LEFT
    sta.w   MetaTiles.map.right

    lda.w   Room.Format.Header.height
    and.w   #0xff
    asl
    asl
    asl
    asl
    // Carry clear
    adc.w   #MetaTiles.map.TOP
    sta.w   MetaTiles.map.bottom


    lda.l   RDMPY
    sta.w   MetaTiles.map.dataSize


    jsr     MetaTiles._ResetQueues
}


// Table of map variables whose value depends on the height of the room
namespace MapVariablesTable {
constant firstVariable = map.bytesPerColumn

evaluate n = 0
inline word(name, evaluate v) {
    assert(map.{name} - firstVariable == {n})
    dw  {v}
    evaluate n = {n} + 2
}

rodata(rom0)
Small:
    word(bytesPerColumn,          MAP_HEIGHT_SMALL)
    word(dataIndexScreenOffset,   MAP_HEIGHT_SMALL * METATILE_DISPLAY_WIDTH + METATILE_DISPLAY_HEIGHT)
    word(dataIndexXmask,          0xff * MAP_HEIGHT_SMALL)
    word(dataIndexYmask,          MAP_HEIGHT_SMALL - 1)
    word(yPositionMask,           Y_POSITION_MASK_SMALL)
assertPowerOfTwo(MAP_HEIGHT_SMALL)

rodata(rom0)
Large:
evaluate n = 0
    word(bytesPerColumn,          MAP_HEIGHT_LARGE)
    word(dataIndexScreenOffset,   MAP_HEIGHT_LARGE * METATILE_DISPLAY_WIDTH + METATILE_DISPLAY_HEIGHT)
    word(dataIndexXmask,          0xff * MAP_HEIGHT_LARGE)
    word(dataIndexYmask,          MAP_HEIGHT_LARGE - 1)
    word(yPositionMask,           Y_POSITION_MASK_LARGE)
assertPowerOfTwo(MAP_HEIGHT_LARGE)

constant size  = Large - Small
assert(pc() - Large == size)
}



// Clear the MetaTile queues
//
// This routine is called when a room or tileset is loaded
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function _ResetQueues {
    sep     #$20
a8()
    Dma.MetaTiles.ResetBufferState()

    stz.w   MetaTiles.SetMapCellQueue.currentIndex
    stz.w   MetaTiles.drawMapCellBufferIndex

    rep     #$30
a16()

    CrumblingTiles._ResetQueues()

    rts
}



// Sets a map cell to a given value.
//
// This routine will set the tile within `map.data` but it will not draw
// the tile to the screen.  Instead it will add the tile to a queue and
// the tile will be drawn at a later time by `MetaTiles.Process_far`.
//
// If the queue is full then the map cell will still be changed, no draw
// call will be made and a warning will be emitted.
//
// This routine will silently fail if the index is out of bounds.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: A = tile Id
// INPUT: Y = map index
a16()
i16()
// DP unknown
code()
function SetMapCell {
    cpy.w   map.dataSize
    bcs     IndexOutOfBounds

    sep     #$20
a8()
    cmp.w   map.data,y
    beq     DataUnchanged

    sta.w   map.data,y

    rep     #$30
a16()
    tya

    sep     #$10
i8()
    ldy.w   SetMapCellQueue.currentIndex
    cpy.b   #SetMapCellQueue.LAST_INDEX + 1
    bcs     QueueFull

    sta.w    SetMapCellQueue.mapDataIndex,y

    iny
    iny
    sty.w   SetMapCellQueue.currentIndex

IndexOutOfBounds:
DataUnchanged:
    rep     #$30
a16()
i16()
    rts


a16()
i8()
QueueFull:
    Warnings.Warn(SET_MAP_CELL_QUEUE_FULL)

    rep     #$30
a16()
i16()
    rts

}



// Draws pending SetMapCellQueue tiles to the drawMapCellBuffer
//
// You MUST NOT call this function if the drawMapCellBuffer is full.
//
// ASSUMES: There is enough DMA time in `dma.transfersLeft` to fill `drawMapCellBuffer`.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
// REQUIRES: drawMapCellBuffer not full
// BRANCH: DrawMapCellBufferFull if the drawMapCellBuffer buffer is full
macro _ProcessSetMapCellQueue() {
    assert16a()
    assert8i()

    ldy.w   SetMapCellQueue.currentIndex
    beq     QueueEmpty

    i8()
    Loop:
        dey
        dey
        sty.w   SetMapCellQueue.currentIndex

        rep     #$30
    i16()
        ldx.w   SetMapCellQueue.mapDataIndex,y
        lda.w   map.data,x

        jsr     Render._DrawMapCell
        bcs     DrawMapCellBufferFull

        sep     #$10
    i8()
        ldy.w   SetMapCellQueue.currentIndex
        bne     Loop
    EndLoop:

QueueEmpty:
}



// Process MetaTiles
//
// This function will:
//    * Update the BG map offset
//    * Draw newly visible tiles to the horizontal and vertical buffers
//    * Draw any pending tiles in the SetMapCellQueue
//    * Process crumbling tiles
//
// It is recommended to call this function after updating the camera but before
// processing entities.
//
// This routine SHOULD BE called early in the game loop (ie, before
// `Entities.ProcessEntityList` and `Resources.ProcessAnimations`) to prevent a
// VBlank overrun.
//
// The camera SHOULD NOT be changed after calling this function.
//
//
// NOTE: This function may change the camera position to ensure ensure only
// 1 MetaTile row and 1 MetaTile column is processed per frame.
//
//
// ASSUMES: The screen is active
// ASSUMES: MetaTiles.Render.DrawFullScreen_ForceBlank__far has been called to setup map state
// ASSUMES: Camera position inside map
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
farCode()
function Process__far {
constant _mapPtr = utDpTmp0
constant _mapPtrRend = utDpTmp2

    BranchIfMapIsSmall_Long(SmallMapHeight)

    namespace LargeMapHeight {
        Render.__Update(LARGE)

        jmp     EndIf
    }

a16()
i16()
SmallMapHeight:
    namespace SmallMapHeight {
        Render.__Update(SMALL)
    }
EndIf:

    sep     #$10
i8()
    CrumblingTiles._IncrementTime()

    // Do not process queues if drawMapCellBuffer is full
    ldx.w   drawMapCellBufferIndex
    cpx.b   #drawMapCellBufferIndex.END
    bcs     DrawMapCellBufferFull


    // NOTE: branches to DrawMapCellBufferFull if drawMapCellBuffer is full
    _ProcessSetMapCellQueue()
assert16a()
assert8i()


    // drawMapCellBuffer is not full

    // Alternate between doing chainA and chainB first.
    // It should make it more fair to the system
    lda.w   CrumblingTiles.time
    lsr
    bcs     OddFrame
        assert(CrumblingTiles.N_CRUMBLING_BLOCK_CHAINS == 2)
        jsr     CrumblingTiles._Process_ChainA

        // Do not process crumbling tiles if the drawMapCellBuffer is full
        ldx.w   drawMapCellBufferIndex
        cpx.b   #drawMapCellBufferIndex.END
        bcs     DrawMapCellBufferFull

        jsr     CrumblingTiles._Process_ChainB

        rep     #$30
    a16()
    i16()
        rtl


    a16()
    i8()
    OddFrame:
        jsr     CrumblingTiles._Process_ChainB

        // Do not process crumbling tiles if the drawMapCellBuffer is full
        ldx.w   drawMapCellBufferIndex
        cpx.b   #drawMapCellBufferIndex.END
        bcs     DrawMapCellBufferFull

        jsr     CrumblingTiles._Process_ChainA

au()
iu()
DrawMapCellBufferFull:
    rep     #$30
a16()
i16()
    rtl
}



// Converts an entity's position into a map index
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// ASSUMES: Entity inside map
//
// INPUT: DP = entity
// OUTPUT: Y = map index
a16()
i16()
code()
function EntityPositionToMapIndex {
allocateTmpWord(_tileIndexTmp)

    assert(METATILE_SIZE_PX == 16)

    inline __PositionToMapIndex(mapHeight) {
        assert16a()
        assert16i()

        constant MAP_HEIGHT = MAP_HEIGHT_{mapHeight}
        constant Y_POSITION_MASK = Y_POSITION_MASK_{mapHeight}

        // X = (entity->yPos & Y_POSITION_MASK) / 16
        //     + ((entity.xPos.px & X_POSITION_MASK) / 16 * MAP_HEIGHT)

        lda.b   BaseEntity.yPos.px
        and.w   #Y_POSITION_MASK
        lsr
        lsr
        lsr
        lsr
        sta.w   _tileIndexTmp

        lda.b   BaseEntity.xPos.px
        and.w   #X_POSITION_MASK
        if MAP_HEIGHT / METATILE_SIZE_PX == 4 {
            asl
            asl
        } else if MAP_HEIGHT / METATILE_SIZE_PX == 8 {
            asl
            asl
            asl
        } else {
            error "Invalid MAP_HEIGHT"
        }
        // carry clear
        adc.w   _tileIndexTmp
        tay
    }

    BranchIfMapIsSmall(SmallMapHeight)

    LargeMapHeight:
    namespace LargeMapHeight {
        __PositionToMapIndex(LARGE)
        rts
    }

    SmallMapHeight:
    namespace SmallMapHeight {
        __PositionToMapIndex(SMALL)
        rts
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

