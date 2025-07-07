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
namespace WalkAvoidLedges {

namespace State {
    createEnum(0, 2)
        enum(WALK_LEFT)
        enum(WALK_RIGHT)
        enum(FALL_AND_TURN)
        enum(TURN)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__WalkLeft
    dw  Process__WalkRight
    dw  Process__FallAndTurn
    dw  Process__Turn


buildFunctionTable(BaseEntityFunctionTable, WalkAvoidLedges)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)
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
    // A.bit0 = left/right flag
    // A.bit1 = down/up gravity
    sep     #$20
a8()
    lsr
    bit.b   #1
    beq     +
        // MUST NOT MODIFY CARRY

        lda.b   #Entity.MovementState.UP_GRAVITY_FLAG
        tsb.b   EntityStruct.movementState
    +

    rep     #$30
a16()

    bcs     +
        jsr     SetState_WalkLeft
        clc
        rts
    +
        jsr     SetState_WalkRight
        clc
        rts
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
    jmp     (ProcessFunctionTable,x)
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState_WalkLeft {
    assert(State.WALK_LEFT == 0)
    stz.b   EntityStruct.state

    ldx.b   EntityStruct.romPtr
    lda.l   {RomStruct}.speed,x
    sta.b   EntityStruct.xMomentum.sx

    sep     #$20
a8()
    lda.l   {RomStruct}.speed + 2,x
    sta.b   EntityStruct.xMomentum.px

    lda.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    trb.b   EntityStruct.movementState

    rep     #$30
a16()

    ldy.w   #{GetAnimationId(walk_left)}
    jmp     SetAnimationWithGravityCheck_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState_WalkRight {
    lda.w   #State.WALK_RIGHT
    sta.b   EntityStruct.state

    ldx.b   EntityStruct.romPtr
    lda.l   {RomStruct}.speed,x
    sta.b   EntityStruct.xMomentum.sx

    sep     #$20
a8()
    lda.l   {RomStruct}.speed + 2,x
    sta.b   EntityStruct.xMomentum.px

    lda.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    tsb.b   EntityStruct.movementState

    rep     #$30
a16()

    ldy.w   #{GetAnimationId(walk_right)}
    jmp     SetAnimationWithGravityCheck_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WalkLeft {
    MoveEntityWithTileCollisionsInteractiveTilesAndDetectEdges()
    bcc     +
        jsr     SetState__Turn
        bra     End
    +

    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
    _Ledge:
        jsr     _XCollision
    +

End:
    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WalkRight {
    MoveEntityWithTileCollisionsInteractiveTilesAndDetectEdges()
    bcc     +
        jsr     SetState__Turn
        bra     End
    +

    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
        jsr     _XCollision
    +

End:
    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function _XCollision {
    lda.b   EntityStruct.movementState - 1
    bit.w   #Entity.MovementState.STANDING_FLAG
    bne     SetState__Turn

Fallthrough: // into SetState__FallAndTurn
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: X = state, Y = animation-id
a16()
i16()
code()
function SetState__FallAndTurn {
assert(pc() == _XCollision.Fallthrough)

    ldx.w   #State.FALL_AND_TURN
    stx.b   EntityStruct.state
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__FallAndTurn {
    MoveEntityAndProcessTileCollisions()
    bcc     +
        jsr     SetState__Turn
    +
    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: X = state, Y = animation-id
a16()
i16()
code()
function SetState__Turn {
    ldx.w   #State.TURN
    stx.b   EntityStruct.state

    stz.b   EntityStruct.xMomentum.sx
    stz.b   EntityStruct.xMomentum.px - 1

    assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)
    assert({GetAnimationId(turn_right)} == {GetAnimationId(turn_left)} + 2)
    ldy.w   #{GetAnimationId(turn_left)}
    lda.b   EntityStruct.movementState
    lsr
    bcs     +
        iny
        iny
    +
    jmp     SetAnimationWithGravityCheck_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Turn {
    // Animation id is 0xff if animation ended
    lda.b   EntityStruct.MetaSprite.Animation.id - 1
    cmp.w   #0xff00
    bcc     EndIf
        assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)
        lda.b   EntityStruct.movementState
        lsr
        bcc     +
            jsr     SetState_WalkLeft
            bra     EndIf
        +
            jsr     SetState_WalkRight
    EndIf:

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}

}
}

