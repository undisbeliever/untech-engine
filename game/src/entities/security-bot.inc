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
namespace SecurityBot {

// ::TODO making the flashing beacon flash::
// ::TODO better defeated animation::


// ::TODO adjust::
constant TURN_COUNTDOWN = 50


namespace State {
    createEnum(0, 2)
        enum(FACE_PLAYER)
        enum(LEFT)
        enum(RIGHT)
        enum(UNDERWATER)
        enum(DEFEATED)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__FacePlayer
    dw  Process__Left
    dw  Process__Right
    dw  Process__Underwater
    dw  Process__Defeated


buildFunctionTable(BaseEntityFunctionTable, SecurityBot)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        // Countdown until the enemy can turn
        field(turnCountdown, 1)
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
    // A.bit0 = down/up gravity
    lsr
    bcc     +
        lda.w   #Entity.MovementState.UP_GRAVITY_FLAG
        tsb.b   EntityStruct.movementState
    +

    assert(State.FACE_PLAYER == 0)
    stz.b   EntityStruct.state

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
    // Face player when reactivated
    assert(State.FACE_PLAYER == 0)
    stz.b   EntityStruct.state

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
    // ::TODO should the bot react to the player?::
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
function Process__FacePlayer {
    sep     #$20
a8()
    lda.b   EntityStruct.movementState
    and.b   #~(Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG)

    ldy.w   #State.LEFT

    ldx.b   EntityStruct.xPos.px
    cpx.w   Entity.Player.xPos.px
    bcs     +
        ora.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
        ldy.w   #State.RIGHT
    +
    sta.b   EntityStruct.movementState
    sty.b   EntityStruct.state


    lda.b   #TURN_COUNTDOWN
    sta.b   EntityStruct.turnCountdown

    stz.b   EntityStruct.xMomentum.px

    rep     #$30
a16()
    stz.b   EntityStruct.xMomentum.sx

    rep     #$30
a16()
    ldy.w   #{GetAnimationId(move_left)}
    jsr     SetAnimation_FacingAndGravity_Y

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Left {
    MoveEntityAndProcessTileCollisions()

    sep     #$20
a8()
    lda.b   EntityStruct.turnCountdown
    beq     CheckPlayerPos
        dec
        sta.b   EntityStruct.turnCountdown
        bra     UpdateXMomentum

    CheckPlayerPos:
        ldx.b   EntityStruct.xPos.px
        cpx.w   Entity.Player.xPos.px
        bcs     UpdateXMomentum
            // Player right of entity
            lda.b   #Entity.MovementState.FACING_RIGHT_FLAG
            tsb.b   EntityStruct.movementState

            lda.b   #State.RIGHT
            bra     __Turning_a8i16

    UpdateXMomentum:
        lda.b   EntityStruct.movementState
        bit.b   #Entity.MovementState.STANDING_FLAG
        beq     EndIf
            rep     #$30
        a16()
            ldx.w   #MovementTableIndex.SecurityBot
            jsr     AccelerateLeft
    EndIf:
au()

    jmp     __UnderwaterTestAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a8()
i16()
code()
function __Turning_a8i16 {
    sta.b   EntityStruct.state

    lda.b   #TURN_COUNTDOWN
    sta.b   EntityStruct.turnCountdown

    rep     #$30
a16()
    ldy.w   #{GetAnimationId(turn_left)}
    jsr     SetAnimation_FacingAndGravity_Y

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Right {
    MoveEntityAndProcessTileCollisions()

    sep     #$20
a8()
    lda.b   EntityStruct.turnCountdown
    beq     CheckPlayerPos
        dec
        sta.b   EntityStruct.turnCountdown
        bra     UpdateXMomentum

    CheckPlayerPos:
        ldx.b   EntityStruct.xPos.px
        cpx.w   Entity.Player.xPos.px
        bcc     UpdateXMomentum
            // Player left of entity
            lda.b   #Entity.MovementState.FACING_RIGHT_FLAG
            trb.b   EntityStruct.movementState

            lda.b   #State.LEFT
            bra     __Turning_a8i16

    UpdateXMomentum:
        lda.b   EntityStruct.movementState
        bit.b   #Entity.MovementState.STANDING_FLAG
        beq     EndIf
            rep     #$30
        a16()
            ldx.w   #MovementTableIndex.SecurityBot
            jsr     AccelerateRight
    EndIf:
au()

Fallthrough:

}



// REQUIRES: 16 bit Index, DB = 0x7e
// INPUT: DP = entity
au()
i16()
code()
function __UnderwaterTestAndGotoNextEntity {
    assert(pc() == Process__Right.Fallthrough)

au()
    BranchIfAboveWater_Y(EndIf)
        sep     #$20
    a8()
        ldx.w   #State.UNDERWATER
        stx.b   EntityStruct.state

        lda.b   #Gravity.UNDERWATER
        sta.b   EntityStruct.gravity
    EndIf:

    rep     #$30
a16()
    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Underwater {
    ldx.w   #MovementTableIndex.SecurityBot
    jsr     DecelerateXmomentum

    MoveEntityAndProcessTileCollisions()


    BranchIfUnderWater_Y(EndIf)
        // Now above water
        assert(Gravity.NORMAL == 0)
        stz.b   EntityStruct.gravity

        ldy.w   #State.LEFT

        lda.b   EntityStruct.movementState
        bit.w   #Entity.MovementState.FACING_RIGHT_FLAG
        beq     +
            ldy.w   #State.RIGHT
        +
        sty.b   EntityStruct.state

EndIf:
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
    bcc     SetState__Defeated

    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// INPUT: X = state, Y = animation-id
a16()
i16()
code()
function SetState__Defeated {
    ldx.w   #State.DEFEATED
    stx.b   EntityStruct.state

    ldy.w   #{GetAnimationId(defeated_left)}
    jmp     SetAnimation_FacingAndGravity_Y
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

