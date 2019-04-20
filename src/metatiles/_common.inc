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

a16()
i16()
// DP unknown
code()
namespace SetMapCell {
ReturnWithWarning:
    Warnings.Warn(SET_MAP_CELL_ERROR)

Return:
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
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = tile Id
// INPUT: X = map index
a16()
i16()
// DP unknown
code()
function SetMapCell {
    cpx.w   map.dataSize
    bcs     ReturnWithWarning

    asl
    cmp.w   #N_META_TILES
    bcs     ReturnWithWarning

    cmp.w   map.data,x
    beq     Return

    sta.w   map.data,x

    // skip draw if in force-blank
    lda.w   inidispBuffer - 1
    bmi     Return

    Render._DrawMapCell()

    rts
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
    // X = ((entity->yPos - map.top) / METATILE_SIZE_PX * map.width
    //      + (entity->xPos - map.left) / METATILE_SIZE_PX) * 2

    assert(METATILE_SIZE_PX == 16)

    lda.b   BaseEntity.yPos.px
    sec
    sbc.w   map.top
    lsr
    lsr
    lsr
    lsr
    sep     #$20
a8()
    xba
    lda.w   map.width
    rep     #$30
a16()
    sta.l   WRMPYA      // also sets WRMPYB


    lda.b   BaseEntity.xPos.px
    sec
    sbc.w   map.left
    lsr
    lsr
    lsr
    lsr
    clc

    adc.l   RDMPY
    asl
    tax

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

