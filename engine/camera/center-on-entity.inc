// camera/center-on-entity.inc
// ===========================
//
// A simple camera that centers the camera on a given entity
//
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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

