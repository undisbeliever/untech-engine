// camera/_common.inc
// ==================
//
// Common routines for the Camera Module.
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

