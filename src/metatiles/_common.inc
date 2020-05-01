// metatiles/_common.inc
// =====================
//
// Common MetaTile routines.
//
// Assumes:
//    * Camera xPos/yPos always inside map
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


Warnings.Register(SET_MAP_CELL_ERROR, 24, 0, 12)


namespace MetaTiles {


// Setup the room variables
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: room loaded into memory
macro _LoadMap() {
    assert16a()
    assert16i()

    // map.bytesPerColumn = room height <= MAP_HEIGHT_SMALL ? MAP_HEIGHT_SMALL : MAP_HEIGHT_LARGE
    lda.w   Room.Format.Header.height
    and.w   #0xff
    cmp.w   #MetaTiles.MAP_HEIGHT_SMALL + 1
    bcc     +
        lda.w   #MetaTiles.MAP_HEIGHT_LARGE
        bra     ++
    +
        lda.w   #MetaTiles.MAP_HEIGHT_SMALL
    +
    sta.w   MetaTiles.map.bytesPerColumn


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
}



namespace SetMapCell {
// DP unknown
// P on stack
au()
i16()
code()
ReturnWithWarning:
    Warnings.Warn(SET_MAP_CELL_ERROR)

Return:
    plp
    rts
}



// Sets a map cell to a given value.
//
// If the screen is enabled, then the cell will then be drawn to VRAM during
// VBlank.
//
// This routine will silently fail and emit a warning if the tile/index is out
// of bounds.
//
// If the VBlank buffer is full then the map cell will be set but not drawn and
// a warning will be emitted.
//
// ASSUMES: Map is on screen if the screen is enabled.
// ASSUMES: DrawFullScreen_ForceBlank has been called if screen is enabled
// REQUIRES: DB = 0x7e
//
// INPUT: A = tile Id
// INPUT: X = map index
au()
iu()
// DP unknown
code()
function SetMapCell {
    php

    rep     #$10
i16()
    cpx.w   map.dataSize
    bcs     ReturnWithWarning

    sep     #$20
a8()
    cmp.w   map.data,x
    beq     Return

    sta.w   map.data,x

    // skip draw if in force-blank
    assert(INIDISP.force == 0x80)
    bit.w   inidispBuffer
    bmi     Return

    BranchIfMapIsSmall_Long(SmallMapHeight)

    LargeMapHeight:
    namespace LargeMapHeight {
        rep     #$30
    a16()

        Render._DrawMapCell(LARGE)

        plp
        rts
    }

    namespace SmallMapHeight {
    // DP unknown
    // P on stack
    au()
    i16()
    code()
    ReturnWithWarning:
        Warnings.Warn(SET_MAP_CELL_ERROR)

    Return:
        plp
        rts
    }

    SmallMapHeight:
    namespace SmallMapHeight {
        rep     #$30
    a16()

        Render._DrawMapCell(SMALL)

        plp
        rts
    }
}


// Converts an entity's position into a map index
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// ASSUMES: Entity inside map
//
// INPUT: DP = entity
// OUTPUT: X = map index
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
        tax
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

