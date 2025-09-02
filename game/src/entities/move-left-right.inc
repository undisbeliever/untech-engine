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
namespace MoveLeftRight {

namespace State {
    createEnum(0, 2)
        enum(LEFT)
        enum(RIGHT)
        enum(TURN)
        enum(DEFEATED)
    endEnum()
}


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        // The left/right xPos before turning around
        field(leftEnd, 2)
        field(rightEnd, 2)

        // Moving x momentum
        field(moveSpeed, 3)
    endstruct()
}

code()
ProcessFunctionTable:
    dw  Process__Left
    dw  Process__Right
    dw  Process__Turn
    dw  Process__Defeated


buildFunctionTable(BaseEntityFunctionTable, MoveLeftRight)



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//         A = moving distance
// OUTPUT: C set = always activate entity
a16()
i16()
code()
function Init {
    and.w   #0xff
    tay

    lsr
    // carry = rounding
    adc.b   EntityStruct.xPos.px
    sta.b   EntityStruct.rightEnd

    tya
    eor.w   #0xffff
    sec
    adc.b   EntityStruct.rightEnd
    sta.b   EntityStruct.leftEnd

    ldx.b   EntityStruct.romPtr

    lda.l   {RomStruct}.xMomentum,x
    sta.b   EntityStruct.moveSpeed
    sta.b   EntityStruct.xMomentum.sx

    assert(State.LEFT == 0)
    assert(State.RIGHT == 2)
    lda.l   {RomStruct}.initDirection,x
    and.w   #1
    asl
    sta.b   EntityStruct.state

    sep     #$20
a8()
    beq     +
        lda.b   #Entity.MovementState.NO_GRAVITY_FLAG | Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
        bra     ++
    +
        lda.b   #Entity.MovementState.NO_GRAVITY_FLAG
    +
    sta.b   EntityStruct.movementState

    lda.l   {RomStruct}.xMomentum + 2,x
    sta.b   EntityStruct.moveSpeed + 2
    sta.b   EntityStruct.xMomentum.px

    rep     #$31
a16()

    ldy.w   #{GetAnimationId(left)}
    jsr     SetAnimation_Facing_Y

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
function Process__Left {
    MoveEntityWithTileCollisionsAndInteractiveTiles()

    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.X_COLLISION_FLAG | Entity.MovementState.STANDING_FLAG
    bne     TurnRight

    ldx.b   EntityStruct.xPos.px
    cpx.b   EntityStruct.leftEnd
    bcc     TurnRight

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity


TurnRight:
    lda.w   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    tsb.b   EntityStruct.movementState

    ldy.w   #{GetAnimationId(turn_right)}
    bra     _SetState__Turn
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Right {
    MoveEntityWithTileCollisionsAndInteractiveTiles()

    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.X_COLLISION_FLAG | Entity.MovementState.STANDING_FLAG
    bne     TurnLeft

    ldx.b   EntityStruct.xPos.px
    cpx.b   EntityStruct.rightEnd
    bcs     TurnLeft

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity


TurnLeft:
    lda.w   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    trb.b   EntityStruct.movementState

    ldy.w   #{GetAnimationId(turn_left)}

Fallthrough:
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//
// X = new state
// Y = animation
a16()
i16()
code()
function _SetState__Turn {
    assert(pc() == Process__Right.Fallthrough)

    jsr     MetaSprite.Animation.SetAnimationId_Y

    ldx.w   #State.TURN
    stx.b   EntityStruct.state

    lda.w   #Entity.MovementState.STANDING_FLAG
    trb.b   EntityStruct.movementState

    stz.b   EntityStruct.xMomentum.sx
    stz.b   EntityStruct.xMomentum.sx + 1

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Turn {
    assert({GetAnimationId(left)} == 0)
    assert({GetAnimationId(right)} == 1)

    lda.b   EntityStruct.MetaSprite.Animation.id - 1
    cmp.w   #0x0200
    bcs     +
        lda.b   EntityStruct.moveSpeed
        sta.b   EntityStruct.xMomentum.sx

        lda.b   EntityStruct.moveSpeed + 1
        sta.b   EntityStruct.xMomentum.sx + 1


        assert(State.LEFT == 0)
        assert(State.RIGHT == Entity.MovementState.FACING_RIGHT_FLAG << 1)
        lda.b   EntityStruct.movementState
        and.w   #1
        asl
        sta.b   EntityStruct.state
    +
    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HurtboxCollision {
    bcs     Return
        lda.w   #State.DEFEATED
        sta.b   EntityStruct.state

Return:
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Defeated {
    // ::TODO spawn poof::

    sep     #$20
a8()
    // ::TODO use defeated sound from entity rom data::
    QueueSoundEffect(robotDefeated)

    rep     #$30
a16()

    jmp     Entity.DeleteAndGotoNextEntity
}

}
}

