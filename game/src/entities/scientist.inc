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
namespace Scientist {

// 8:8 fixed point
constant PROJECTILE_MOMENTUM = 0x0400

namespace State {
    createEnum(0, 2)
        enum(NORMAL)
        enum(CROUCHED)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__Normal
    dw  Process__Crouched


buildFunctionTable(BaseEntityFunctionTable, Scientist)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        field(tmp, 2)
    endstruct()
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//         A = facing direction
// OUTPUT: C set = always activate entity
a16()
i16()
code()
function Init {
    sep     #$20
a8()

    // A.bit0 = left/right flag
    // A.bit1 = down/up gravity
    // A.bit2 = crouching
    sta.b   EntityStruct.tmp

    lsr.b   EntityStruct.tmp
    bcc     +
        lda.b   #Entity.MovementState.MOVING_DOWN_FLAG | Entity.MovementState.FACING_RIGHT_FLAG
        tsb.b   EntityStruct.movementState
    +

    lsr.b   EntityStruct.tmp
    bcc     +
        lda.b   #Entity.MovementState.UP_GRAVITY_FLAG
        tsb.b   EntityStruct.movementState
    +


    ldx.w   #State.NORMAL
    ldy.w   #{GetAnimationId(stand_left)}
    lsr.b   EntityStruct.tmp
    bcc     +
        ldx.w   #State.CROUCHED
        ldy.w   #{GetAnimationId(crouching_left)}
    +

    stx.b   EntityStruct.state

    rep     #$30
a16()
    jmp     SetAnimation_FacingAndGravity_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Destructor {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Deactivated {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HitboxCollision {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HurtboxCollision {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with hitbox
a16()
i16()
code()
function ShieldCollision {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process {
    ldx.b   EntityStruct.state
    jsr     (ProcessFunctionTable,x)

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Normal {
    MoveEntityWithTileCollisionsAndInteractiveTiles()

    ldy.w   #{GetAnimationId(stand_left)}
    bra     _SetAnimation
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Crouched {
    MoveEntityAndProcessTileCollisions()

    ldy.w   #{GetAnimationId(crouching_left)}
    bra     _SetAnimation
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: Y = normal animation
a16()
i16()
code()
function _SetAnimation {
    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.STANDING_FLAG
    bne     +
        ldy.w   #{GetAnimationId(falling_left)}
    +
    jmp     SetAnimation_FacingAndGravity_Y
}

}
}
