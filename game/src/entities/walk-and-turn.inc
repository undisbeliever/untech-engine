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
namespace WalkAndTurn {

namespace State {
    createEnum(0, 2)
        enum(WALK_LEFT)
        enum(WALK_RIGHT)
        enum(FALL_AND_TURN)
        enum(TURN)
        enum(DEFEATED)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__WalkLeft
    dw  Process__WalkRight
    dw  Process__FallAndTurn
    dw  Process__Turn
    dw  Process__Defeated


buildFunctionTable(BaseEntityFunctionTable, WalkAndTurn)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        // Byte flag to manage underwater state.
        // non-zero when underwater.
        field(underwater, 1)
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
    stz.b   EntityStruct.underwater

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
    bcs     Return
        lda.w   #State.DEFEATED
        sta.b   EntityStruct.state

        ldy.w   #{GetAnimationId(defeated_left)}
        jmp     SetAnimation_FacingAndGravity_Y

Return:
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
    sep     #$20
a8()
    lda.b   EntityStruct.underwater
    bne     Underwater
        BranchIfAboveWater_Y(EndIf)
            // Now underwater
            lda.b   #Gravity.UNDERWATER
            sta.b   EntityStruct.gravity

            assert(Gravity.UNDERWATER != 0)
            sta.b   EntityStruct.underwater

            bra     MaybeChangeMomentum

    Underwater:
        BranchIfUnderWater_Y(EndIf)
            // Now above water
            assert(Gravity.NORMAL == 0)
            stz.b   EntityStruct.gravity

            stz.b   EntityStruct.underwater

            MaybeChangeMomentum:
                // Only change momentum if moving
                lda.b   EntityStruct.state
                cmp.b   #State.FALL_AND_TURN
                bcs     EndIf
                    jsr     __SetXMomentum_a8__returnsA16
    EndIf:
au()
    rep     #$30
a16()

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

    sep     #$20
a8()
    lda.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    trb.b   EntityStruct.movementState

    bra     __SetXMomentum_a8__returnsA16
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState_WalkRight {
    lda.w   #State.WALK_RIGHT
    sta.b   EntityStruct.state

    sep     #$20
a8()
    lda.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    tsb.b   EntityStruct.movementState

Fallthrough:
}


// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// RETURNS: 16 bit A
a16()
i16()
code()
function __SetXMomentum_a8__returnsA16 {
    assert(pc() == SetState_WalkRight.Fallthrough)

    ldx.b   EntityStruct.romPtr

    lda.b   EntityStruct.underwater
    bne     Underwater

        lda.l   {RomStruct}.speed,x
        sta.b   EntityStruct.xMomentum.sx

        rep     #$30
    a16()
        lda.l   {RomStruct}.speed + 1,x
        sta.b   EntityStruct.xMomentum.sx + 1

        ldy.w   #{GetAnimationId(walk_left)}
        jmp     SetAnimation_FacingAndGravity_Y


Underwater:
        lda.l   {RomStruct}.underwaterSpeed,x
        sta.b   EntityStruct.xMomentum.sx

        rep     #$30
    a16()
        lda.l   {RomStruct}.underwaterSpeed + 1,x
        sta.b   EntityStruct.xMomentum.sx + 1

        ldy.w   #{GetAnimationId(walk_left)}
        jmp     SetAnimation_FacingAndGravity_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WalkLeft {
    MoveEntityAndProcessTileCollisions()

    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
        jsr     _XCollision
    +

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WalkRight {
    MoveEntityAndProcessTileCollisions()

    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
        jsr     _XCollision
    +

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

    lda.b   EntityStruct.movementState
    eor.w   #Entity.MovementState.FACING_RIGHT_FLAG
    sta.b   EntityStruct.movementState

    ldy.w   #{GetAnimationId(turn_left)}
    jmp     SetAnimation_FacingAndGravity_Y
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
        bcs     +
            jsr     SetState_WalkLeft
            bra     EndIf
        +
            jsr     SetState_WalkRight
    EndIf:

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Defeated {
    // Stop x movement when on the ground
    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.STANDING_FLAG
    beq     +
        stz.b   EntityStruct.xMomentum.sx
        stz.b   EntityStruct.xMomentum.sx + 1
    +

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}

}
}

