// camera/center-on-entity.inc
// ===========================
//
// A simple camera that centers the camera on a given entity
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Camera {


// Centers the camera on the player entity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E, DP = 0
//
// INPUT: X = entity
a16()
i16()
code()
function CenterOnPlayer {
    lda.w   Entity.Player.xPos.px
    sec
    sbc.w   #DISPLAY_WIDTH / 2
    tax

    lda.w   Entity.Player.yPos.px
    sec
    sbc.w   #DISPLAY_HEIGHT / 2
    tay

    jmp     SetPosition_XY
}



// Centers the camera on a given entity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E, DP = 0
//
// INPUT: X = entity
a16()
i16()
code()
function CenterOnEntity_X {
    ldy.b   BaseEntity.yPos.px,x

    lda.b   BaseEntity.xPos.px,x
    sec
    sbc.w   #DISPLAY_WIDTH / 2
    tax

    tya
    sec
    sbc.w   #DISPLAY_HEIGHT / 2
    tay

    jmp     SetPosition_XY
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

