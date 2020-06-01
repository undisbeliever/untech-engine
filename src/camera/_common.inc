// camera/_common.inc
// ==================
//
// Common routines for the Camera Module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Camera {

// Initializes the Camera module
//
// This MUST be called before using this module
//
// ASSUMES: Room loaded
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E, DP = 0
a16()
i16()
code()
function Init {
    // Reset the camera to map origin

    lda.w   #MetaTiles.map.LEFT
    sta.w   xPos

    assert(MetaTiles.map.TOP == MetaTiles.map.LEFT)
    sta.w   yPos


    // Calculate camera limits
    lda.w   MetaTiles.map.right
    sec
    sbc.w   #DISPLAY_WIDTH
    sta.w   maxXPos

    lda.w   MetaTiles.map.bottom
    sec
    sbc.w   #DISPLAY_HEIGHT
    sta.w   maxYPos

    rts
}


// Sets the camera position.
//
// This routine will ensure the camera position is inside the map boundary.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E, DP = 0
//
// INPUT: X = X position
//        Y = Y position
a16()
i16()
code()
function SetPosition_XY {
    cpx.w   #MetaTiles.map.LEFT
    bcs     +
        ldx.w   #MetaTiles.map.LEFT
        bra     ++
    +
    cpx.w   maxXPos
    bcc     +
        ldx.w   maxXPos
    +
    stx.w   xPos


    cpy.w   #MetaTiles.map.TOP
    bcs     +
        ldy.w   #MetaTiles.map.TOP
        bra     ++
    +
    cpy.w   maxYPos
    bcc     +
        ldy.w   maxYPos
    +
    sty.w   yPos

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

