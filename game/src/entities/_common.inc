// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Entities {

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: Y = animation id
a16()
i16()
code()
function SetAnimation_Facing_Y {
    assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)

    lda.b   BaseEntity.movementState
    lsr
    bcc     +
        iny
    +
    jmp     MetaSprite.Animation.SetAnimationId_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: Y = animation id
a16()
i16()
code()
function SetAnimation_FacingAndGravity_Y {
constant _tmp = zpTmp0

    assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)
    assert(Entity.MovementState.UP_GRAVITY_FLAG == 2)

    sty.w   _tmp

    lda.b   BaseEntity.movementState
    and.w   #3
    ora.w   _tmp
    jmp     MetaSprite.Animation.SetAnimationId
}


}

