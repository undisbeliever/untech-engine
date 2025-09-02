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
namespace JumpTowardsPlayer {


// ::TODO adjust::
constant ENTER_WATER_COUNTDOWN = 20


namespace State {
    createEnum(0, 2)
        enum(FALL)
        enum(LANDED)
        enum(RISE)
        enum(UNDERWATER)
        enum(DEFEATED)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__Fall
    dw  Process__Landed
    dw  Process__Rise
    dw  Process__Underwater
    dw  Process__Defeated


buildFunctionTable(BaseEntityFunctionTable, JumpTowardsPlayer)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        field(swimCountdown, 2)
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
    // A.bit0 = down/up gravity
    lsr
    bcc     +
        lda.b   #Entity.MovementState.UP_GRAVITY_FLAG
        tsb.b   EntityStruct.movementState
    +

    ldx.b   EntityStruct.xPos.px
    cpx.w   Entity.Player.xPos.px
    bcs     +
        lda.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
        tsb.b   EntityStruct.movementState
    +

    rep     #$30
a16()

    assert(State.FALL == 0)
    stz.b   EntityStruct.state

    // Cannot use fall animation - the jump and shoot enemy that spawns a projectile when falling
    ldy.w   #{GetAnimationId(land_left)}
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
function Process__Fall {
    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.STANDING_FLAG
    beq     +
        ldx.w   #State.LANDED
        stx.b   EntityStruct.state

        stz.b   EntityStruct.xMomentum.sx
        stz.b   EntityStruct.xMomentum.px - 1

        ldy.w   #{GetAnimationId(land_left)}
        jsr     SetAnimation_FacingAndGravity_Y
    +

    jmp     __UnderwaterTestAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Landed {
    sep     #$20
a8()
    // Animation id is 0xff if animation ended
    lda.b   EntityStruct.MetaSprite.Animation.id
    inc
    bne     +
        jsr     __Jump__au__returnsA16
    a16()
        jmp     __UnderwaterTestAndGotoNextEntity
    +

a8()
    // Change direction if required, restarting the land animation
    lda.b   EntityStruct.movementState
    and.b   #~(Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG)

    ldx.b   EntityStruct.xPos.px
    cpx.w   Entity.Player.xPos.px
    bcs     +
        ora.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    +
    cmp.b   EntityStruct.movementState
    beq     +
        sta.b   EntityStruct.movementState

        rep     #$30
    a16()
        ldy.w   #{GetAnimationId(land_left)}
        jsr     SetAnimation_FacingAndGravity_Y
    +

    rep     #$30
a16()

    jmp     __UnderwaterTestAndGotoNextEntity
}


// REQUIRES: 16 bit Index, DB = 0x7e
// RETURNS: 16 bit A
// INPUT: DP = entity
au()
i16()
code()
function __Jump__au__returnsA16 {
    sep     #$20
a8()

    lda.b   #State.RISE
    sta.b   EntityStruct.state

    jsr     __FacePlayerAndClearMovingDown_a8
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    beq     +
        ora.b   #Entity.MovementState.MOVING_DOWN_FLAG
    +
    sta.b   EntityStruct.movementState


    ldx.b   EntityStruct.romPtr

    lda.l   {RomStruct}.airXMomentum + 2,x
    sta.b   EntityStruct.xMomentum.px

    lda.l   {RomStruct}.airYMomentum + 2,x
    sta.b   EntityStruct.yMomentum.px

    rep     #$30
a16()
    lda.l   {RomStruct}.airXMomentum,x
    sta.b   EntityStruct.xMomentum.sx

    lda.l   {RomStruct}.airYMomentum,x
    sta.b   EntityStruct.yMomentum.sx

    ldy.w   #{GetAnimationId(rise_left)}
    jmp     SetAnimation_FacingAndGravity_Y
}



// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
// OUT: A = new entity movementState
a8()
i16()
code()
function __FacePlayerAndClearMovingDown_a8 {
    lda.b   EntityStruct.movementState
    and.b   #~(Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG | Entity.MovementState.STANDING_FLAG | Entity.MovementState.MOVING_DOWN_FLAG | Entity.MovementState.NO_GRAVITY_FLAG)

    ldx.b   EntityStruct.xPos.px
    cpx.w   Entity.Player.xPos.px
    bcs     +
        ora.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    +

    rts
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Rise {
    sep     #$20
a8()

    // Test if the enemy is falling
    assert(Entity.MovementState.MOVING_DOWN_FLAG == Entity.MovementState.UP_GRAVITY_FLAG << 1)
    lda.b   EntityStruct.movementState
    asl
    eor.b   EntityStruct.movementState
    and.b   #Entity.MovementState.MOVING_DOWN_FLAG
    beq     EndIf
        ldx.w   #State.FALL
        stx.b   EntityStruct.state

        rep     #$30
    a16()
        ldy.w   #{GetAnimationId(fall_left)}
        jsr     SetAnimation_FacingAndGravity_Y
    EndIf:

Fallthrough:
}



// REQUIRES: 16 bit Index, DB = 0x7e
// INPUT: DP = entity
au()
i16()
code()
function __UnderwaterTestAndGotoNextEntity {
    assert(pc() == Process__Rise.Fallthrough)

au()
    BranchIfAboveWater_Y(EndIf)
        sep     #$20
    a8()
        ldx.w   #State.UNDERWATER
        stx.b   EntityStruct.state

        lda.b   #Gravity.UNDERWATER
        sta.b   EntityStruct.gravity

        rep     #$30
    a16()
        lda.w   #ENTER_WATER_COUNTDOWN
        sta.b   EntityStruct.swimCountdown
    EndIf:

    rep     #$30
a16()

    MoveEntityWithTileCollisionsAndInteractiveTiles()

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Underwater {
    dec.b   EntityStruct.swimCountdown
    bpl     EndIf
        ldx.b   EntityStruct.romPtr

        lda.l   {RomStruct}.swimDelay,x
        and.w   #0xff
        sta.b   EntityStruct.swimCountdown

        lda.l   {RomStruct}.swimXMomentum,x
        sta.b   EntityStruct.xMomentum.sx

        lda.l   {RomStruct}.swimYMomentum,x
        sta.b   EntityStruct.yMomentum.sx

        sep     #$20
    a8()
        lda.l   {RomStruct}.swimXMomentum + 2,x
        sta.b   EntityStruct.xMomentum.px

        lda.l   {RomStruct}.swimYMomentum + 2,x
        sta.b   EntityStruct.yMomentum.sx + 2


        jsr     __FacePlayerAndClearMovingDown_a8
        sta.b   EntityStruct.movementState

        rep     #$30
    a16()

        ldy.w   #{GetAnimationId(rise_left)}
        jsr     SetAnimation_FacingAndGravity_Y
    EndIf:


    BranchIfUnderWater_Y(Underwater)
        sep     #$20
    a8()
        // Now above water
        assert(Gravity.NORMAL == 0)
        stz.b   EntityStruct.gravity

        jsr     __Jump__au__returnsA16
    a16()
        bra     EndUnderwaterTest

    Underwater:
        // Test if the enemy is falling
        sep     #$20
    a8()
        assert(Entity.MovementState.MOVING_DOWN_FLAG == Entity.MovementState.UP_GRAVITY_FLAG << 1)
        lda.b   EntityStruct.movementState
        asl
        eor.b   EntityStruct.movementState
        and.b   #Entity.MovementState.MOVING_DOWN_FLAG
        beq     +
            rep     #$30
        a16()
            ldy.w   #{GetAnimationId(fall_left)}
            jsr     SetAnimation_FacingAndGravity_Y
        +
    au()
        rep     #$30
    a16()

EndUnderwaterTest:
    MoveEntityWithTileCollisionsAndInteractiveTiles()

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

    // ::TODO is this a good idea with projectiles.  It works for melee::
    assert(EntityStruct.xMomentum.sx + 3 == EntityStruct.yMomentum.sx)
    stz.b   EntityStruct.xMomentum.sx
    stz.b   EntityStruct.xMomentum.px
    stz.b   EntityStruct.yMomentum.sx + 1

    sep     #$20
a8()
    QueueSoundEffect(robotDefeated)

    rep     #$30
a16()

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

    MoveEntityWithTileCollisionsAndInteractiveTiles()

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}

}
}

