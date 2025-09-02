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
namespace Projectiles {
namespace PlayerProjectile {

// 8:8 fixed point
constant PROJECTILE_MOMENTUM = 0x0400

namespace State {
    createEnum(0, 2)
        enum(MOVE)
        enum(POP)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__Move
    dw  Process__Pop


buildFunctionTable(BaseEntityFunctionTable, PlayerProjectile)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)
    endstruct()
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//       `MetaSprite.ActionPoint.positionPair` = MS frame position that spawned the entity
// OUTPUT: C set = always activate entity
a16()
i16()
code()
function Init {
    assert(State.MOVE == 0)
    stz.b   EntityStruct.state

    lda.w   #PROJECTILE_MOMENTUM
    sta.b   EntityStruct.xMomentum.px - 1

    sep     #$30
a8()
i8()
    assert(INT_MS8_OFFSET == 0x80)
    lda.w   MetaSprite.ActionPoint.positionPair.xPos
    bmi     Right
        ldx.b   #Entity.MovementState.NO_GRAVITY_FLAG
        lda.b   #{GetAnimationId(move_left)}

        bra     EndIf
    Right:
        // Action point is on the right, entity is moving right
        ldx.b   #Entity.MovementState.NO_GRAVITY_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG | Entity.MovementState.FACING_RIGHT_FLAG
        lda.b   #{GetAnimationId(move_right)}
    EndIf:

    stx.b   EntityStruct.movementState

    assert(Players.MAX_PROJECTILES < 0x100)
    inc.w   nPlayerProjectiles

    rep     #$30
a16()
i16()

    jmp     MetaSprite.Animation.SetAnimationId
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Destructor {
    dec.w   nPlayerProjectiles

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
    bcc     NoShield
        sep     #$20
    a8()
        QueueSoundEffect(projectileShieldBounce)
        rep     #$30
    a16()

    NoShield:
    jmp     SetState__Pop
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
    jmp     (ProcessFunctionTable,x)
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Move {
    MoveEntityWithTileCollisionsAndInteractiveTiles()

    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.X_COLLISION_FLAG | Entity.MovementState.STANDING_FLAG
    beq     +
        sep     #$20
    a8()
        QueueSoundEffect(projectileWallCollision)
        rep     #$30
    a16()
        jsr     SetState__Pop
    +

    jmp     Entity.DeleteIfOutsideOrDrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState__Pop {
    lda.w   #State.POP
    sta.b   EntityStruct.state

    lda.w   #{GetAnimationId(pop)}
    jmp     MetaSprite.Animation.SetAnimationId
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Pop {
    // Animation id is 0xff if animation ended
    lda.b   EntityStruct.MetaSprite.Animation.id - 1
    cmp.w   #0xff00
    bcc     EndIf
        jmp     Entity.DeleteAndGotoNextEntity
    EndIf:

    jmp     Entity.DeleteIfOutsideOrDrawAndGotoNextEntity
}

}
}
}
