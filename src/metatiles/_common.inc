// metatiles/_common.inc
// =====================
//
// Common MetaTile routines.
//
// Assumes:
//    * Camera xPos/yPos always inside map
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {

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

