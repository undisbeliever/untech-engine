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
namespace Players {

// 8:8 fixed point
constant MIN_RUN_MOMENTUM = 0x0280

// 8:8 fixed point
constant LADDER_X_MOMENTUM = 0x0120
constant LADDER_Y_MOMENTUM = 0x0120


// Number of pixels the player must be out of the water before switching to normal state
constant SWIMMING_YPOS_BEFORE_OUT_OF_WATER = 10


// ::TODO adjust::
constant HURT_IFRAMES = 40


// Minimum number of frames between player particle shots
constant PROJECTILE_COOLDOWN_FRAMES = 20

constant MAX_PROJECTILES = 3


// Number of frames after standing before the player can jump
// ::TODO adjust::
constant COYOTE_TIME_FRAMES = 8

// ::TODO adjust::
constant LADDER_COYOTE_TIME_FRAMES = 5

// Number of frames between B swim up/down actions
// ::TODO adjust::
constant SWIM_COOLDOWN_FRAMES = 20


// Player gamestate variables
constant playerHealth = Project.GameState.Words.playerHealth


// Player entity
//
// Manually controlled by Joypad1
namespace Player {

namespace State {
    createEnum(0, 2)
        enum(NORMAL)
        enum(MELEE_ATTACK)
        enum(UNDERWATER)
        enum(UNDERWATER_ATTACK)
        enum(LADDER)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__Normal
    dw  Process__MeleeAttack
    dw  Process__Underwater
    dw  Process__UnderwaterAttack
    dw  Process__Ladder


buildFunctionTable(BasePlayerFunctionTable, Player)

constant DrawPlayer = MetaSprite.Render.RenderEntity


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        // Player i-frame counter
        // If set, the player has been recently hurt and will not take damage a second time
        // (byte)
        field(iFrames, 1)


        // Projectile cooldown timer
        // (byte)
        field(projectileCooldown, 1)

        // Used for coyote time
        //  * Right shifted every frame
        //  * Set to (1 << COYOTE_TIME_FRAMES - 1) if standing.
        //
        // (byte)
        field(coyoteJumpBuffer, 1)


        // Decrementing counter until an underwater player can swim again
        // (byte)
        field(swimCooldown, 1)


        // The X-position of the ladder the player is on
        // (word)
        field(ladderXpos, 2)
    endstruct()
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Init {
    assert(State.NORMAL == 0)
    stz.b   EntityStruct.state

    jmp     _SetNormalOrUnderwaterState
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function RoomAboutToChange {
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


// ASSUMES: Collision enemy has a `BaseEnemy`
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HurtboxCollision {
    sep     #$20
a8()

    // ::TODO test for shield::

    lda.b   EntityStruct.iFrames
    bne     Return
        lda.b   #HURT_IFRAMES
        sta.b   EntityStruct.iFrames

        // Enemy hurts the player
        ldx.w   BaseEntity.romPtr,y

        lda.w   playerHealth
        sec
        sbc.l   Project.EntityRomStructs.BaseEnemy.attack,x
        sta.w   playerHealth


        // ::TODO add optional knockback (stored in BaseEntity struct)::


        // Change the player to the hurt palette
        SetPalette(1)


        // ::TODO play sound::

Return:
    rep     #$30
a16()
i16()
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



// REQUIRES: 8 bit A, DB = 0x7e
// INPUT: dp = entity
a8()
iu()
code()
function InvertGravity__a8iu {
    lda.b   EntityStruct.movementState
    eor.b   #Entity.MovementState.UP_GRAVITY_FLAG | Entity.MovementState.MOVING_DOWN_FLAG
    sta.b   EntityStruct.movementState

    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process {
    sep     #$20
a8()
    lda.b   EntityStruct.iFrames
    beq     +
        dec
        sta.b   EntityStruct.iFrames
        bne     +
            // Restore the player's palette
            SetPalette(0)
    +

    rep     #$30
a16()

    ldx.b   EntityStruct.state
    jsr     (ProcessFunctionTable,x)

    // Player entities MUST NOT draw in the Process Routine
    jmp     MetaSprite.Animation.Process
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _Movement {
    sep     #$30
a8()
i8()

    // Use different gravity if the jump button is held down
    assert(JOY_H_JUMP_BUTTON == 0x80)
    ldy.b   #Gravity.PLAYER

    lda.w   Controller.Joy1.current + 1
    bpl     +
        ldy.b   #Gravity.PLAYER_HOLDING_JUMP
    +
    sty.b   EntityStruct.gravity


    ldx.b   #MovementTableIndex.PlayerWalk

    // A = Controller.Joy1.current + 1
    bit.b   #JOY_H_RUN_BUTTON
    beq     NoRun
        assert(MovementTableIndex.PlayerRun - MovementTableIndex.PlayerWalk == 2)
        inx
        inx
    NoRun:

    rep     #$10
i16()

    bit.b   #JOYH.left
    beq     NoLeft
        lda.b   #Entity.MovementState.FACING_RIGHT_FLAG
        trb.b   EntityStruct.movementState

        rep     #$30
    a16()
        jsr     AccelerateLeft

        bra     EndXMovement

    NoLeft:
        // A = Joy1.current + 1
        bit.b   #JOYH.right
        beq     NoRight
            lda.b   #Entity.MovementState.FACING_RIGHT_FLAG
            tsb.b   EntityStruct.movementState

            rep     #$30
        a16()
            jsr     AccelerateRight

            bra     EndXMovement

    NoRight:
        rep     #$30
    a16()
        // Neither left or right pressed
        jsr     DecelerateXmomentum

EndXMovement:
a16()


    sep     #$20
a8()

    // Manage coyote time
    lsr.b   EntityStruct.coyoteJumpBuffer

    lda.b   EntityStruct.movementState
    bit.b   #Entity.MovementState.STANDING_FLAG
    beq     +
        assert(COYOTE_TIME_FRAMES < 9)
        lda.b   #1 << (COYOTE_TIME_FRAMES - 1)
        sta.b   EntityStruct.coyoteJumpBuffer
    +


    // Test for jumping off the ladder
    lda.w   InputBuffer.jump
    beq     NoJump
        lda.b   EntityStruct.coyoteJumpBuffer
        beq     NoJump
            // Can jump and jump recently pressed
            stz.w   InputBuffer.jump

            jsr     SetJumpYVelocity__a8i16
    NoJump:
a8()
    rep     #$30
a16()
i16()

    MovePlayerWithTileCollisionsAndInteractiveTiles()

    rts
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _SetAnimation {

constant _msAnimationBits = zpTmp0

    sep     #$20
a8()

    // ::TODO do not change animation if firing/blocked/hurt etc::


    assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)
    assert(Entity.MovementState.UP_GRAVITY_FLAG == 2)
    lda.b   EntityStruct.movementState
    and.b   #3
    sta.w   _msAnimationBits

    lda.b   EntityStruct.movementState
    bit.b   #Entity.MovementState.STANDING_FLAG
    beq     NotStanding
        sep     #$20
    a8()

        lda.w   Controller.Joy1.current + 1
        bit.b   #JOYH.left
        beq     NoLeft
            lda.b   EntityStruct.movementState
            bit.b   #Entity.MovementState.MOVING_RIGHT_FLAG
            beq     WalkingOrRunning
                lda.b   #{GetAnimationId(sliding_left)}
                bra     _EndIf

        NoLeft:
            // A = Joy1.current + 1
            bit.b   #JOYH.right
            beq     NoRight
                lda.b   EntityStruct.movementState
                bit.b   #Entity.MovementState.MOVING_RIGHT_FLAG
                bne     WalkingOrRunning
                    lda.b   #{GetAnimationId(sliding_left)}
                    bra     _EndIf

                WalkingOrRunning:
                    ldx.b   EntityStruct.xMomentum.px - 1
                    cpx.w   #MIN_RUN_MOMENTUM
                    bcs     _Running
                        lda.b   #{GetAnimationId(walking_left)}
                        bra     _EndIf

                    _Running:
                        lda.b   #{GetAnimationId(running_left)}
                        bra     _EndIf

        NoRight:
            ldx.b   EntityStruct.xMomentum.px - 1
            beq     Stopped
                lda.b   #{GetAnimationId(breaking_left)}
                bra     _EndIf

            Stopped:
                lda.b   #{GetAnimationId(standing_left)}
                bra     _EndIf

    NotStanding:
        // Determine if the player is rising or falling.
        //
        //  if (movementState & MOVING_DOWN_FLAG) ^ (movementState & UP_GRAVITY_FLAG):
        //      falling
        //  else:
        //      rising
        //
        assert(Entity.MovementState.MOVING_DOWN_FLAG == Entity.MovementState.UP_GRAVITY_FLAG << 1)
        // A = movementState
        asl
        eor.b   EntityStruct.movementState
        and.b   #Entity.MovementState.MOVING_DOWN_FLAG
        beq     +
            lda.b   #{GetAnimationId(falling_left)}
            bra     _EndIf
        +
        lda.b   #{GetAnimationId(rising_left)}
        bra     _EndIf


a8()
_EndIf:
    // A = animation
    ora.w   _msAnimationBits

    rep     #$30
a16()
    jmp     MetaSprite.Animation.SetAnimationId


a16()
_Return:
    rep     #$30
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _SetUnderwaterAnimation {

constant _msFrameBits = zpTmp0

    sep     #$30
a8()
i8()
    lda.b   EntityStruct.movementState
    bit.b   #Entity.MovementState.STANDING_FLAG
    bne     Standing

        assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)
        assert(Entity.MovementState.UP_GRAVITY_FLAG == 2)
        and.b   #3
        sta.w   _msFrameBits


        lda.b   #{GetFrameId(underwater_left)}
        ldy.b   EntityStruct.swimCooldown
        beq     +
            lda.b   #{GetFrameId(swimming_left)}
        +
        ora.w   _msFrameBits

        rep     #$30
    a16()
    i16()
        jmp     MetaSprite.DisableAnimationAndSetFrame


a8()
i8()
Standing:
    rep     #$30
a16()
i16()
    ldy.w   #{GetAnimationId(walking_left)}
    jmp     SetAnimation_FacingAndGravity_Y
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _UnderwaterMovement {
    sep     #$30
a8()
i8()
    // Use different gravity if the jump button is held down
    assert(JOY_H_JUMP_BUTTON == 0x80)
    ldy.b   #Gravity.SWIMMING_PLAYER

    lda.w   Controller.Joy1.current + 1
    bpl     +
        ldy.b   #Gravity.SWIMMING_PLAYER_HOLDING_JUMP
    +
    sty.b   EntityStruct.gravity


    ldx.b   #MovementTableIndex.PlayerSwim

    // A = Controller.Joy1.current + 1
    bit.b   #JOY_H_RUN_BUTTON
    beq     NoRun
        assert(MovementTableIndex.PlayerSwimFast == MovementTableIndex.PlayerSwim + 2)
        inx
        inx
    NoRun:

    lda.b   EntityStruct.movementState
    bit.b   #Entity.MovementState.STANDING_FLAG
    beq     +
        ldx.b   #MovementTableIndex.PlayerSwimStanding
    +


    rep     #$10
i16()

    lda.w   Controller.Joy1.current + 1
    bit.b   #JOYH.left
    beq     NoLeft
        lda.b   #Entity.MovementState.FACING_RIGHT_FLAG
        trb.b   EntityStruct.movementState

        rep     #$30
    a16()
        jsr     AccelerateLeft

        bra     EndXMovement

    NoLeft:
        // A = Joy1.current + 1
        bit.b   #JOYH.right
        beq     NoRight
            lda.b   #Entity.MovementState.FACING_RIGHT_FLAG
            tsb.b   EntityStruct.movementState

            rep     #$30
        a16()
            jsr     AccelerateRight

            bra     EndXMovement

    NoRight:
        rep     #$30
    a16()
        // Neither left or right pressed
        jsr     DecelerateXmomentum

EndXMovement:
a16()


    sep     #$20
a8()

    // No coyote time when swimming

    lda.b   EntityStruct.swimCooldown
    beq     CanSwimUp
        dec.b   EntityStruct.swimCooldown
        bra     EndJumpTest
        CanSwimUp:
            lda.w   InputBuffer.jump
            beq     EndJumpTest
                stz.w   InputBuffer.jump

                lda.b   #SWIM_COOLDOWN_FRAMES
                sta.b   EntityStruct.swimCooldown

                jsr     SetJumpYVelocity__a8i16
    EndJumpTest:

au()
    rep     #$30
a16()
i16()

    MovePlayerWithTileCollisionsAndInteractiveTiles()

    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process__Normal {
    sep     #$20
a8()

    lda.w   InputBuffer.attack
    bne     __SetMeleeAttackState_A8


    // Launch projectile with cooldown
    lda.b   EntityStruct.projectileCooldown
    beq     +
        dec.b   EntityStruct.projectileCooldown
        bra     ++
    +
        lda.w   InputBuffer.projectile
        beq     +
            lda.w   nPlayerProjectiles
            cmp.b   #MAX_PROJECTILES
            bcs     +
                stz.w   InputBuffer.projectile

                lda.b   #PROJECTILE_COOLDOWN_FRAMES
                sta.b   EntityStruct.projectileCooldown

                rep     #$30
            a16()
                SpawnManualPlayerProjectiles()
    +
au()
    rep     #$30
a16()
i16()
    jsr     _Movement

    lda.b   EntityStruct.yPos.px
    cmp.w   Project.GameState.Words.waterPosition
    bcc     +
        // ::TODO splash sound::
        // ::TODO spawn splash particle::

        jmp     _SetUnderwaterState
    +

    jmp     _SetAnimation
}


// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// RETURNS: 16 bit A
// INPUT: dp = entity
a8()
i16()
code()
function __SetMeleeAttackState_A8 {
    stz.w   InputBuffer.attack

    lda.b   #State.MELEE_ATTACK
    sta.b   EntityStruct.state

    lda.b   EntityStruct.movementState
    and.b   #3
    ora.b   #{GetAnimationId(melee_left)}
    jmp     MetaSprite.Animation.SetAnimationId
}


// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process__MeleeAttack {
    jsr     _Movement

    // Switch to normal state when attack animation is over
    lda.b   EntityStruct.MetaSprite.Animation.id - 1
    cmp.w   #0xff00
    bcc     +
        assert(State.NORMAL == 0)
        stz.b   EntityStruct.state

        jmp     _SetAnimation
    +
    rts
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _SetUnderwaterState {
    lda.w   #State.UNDERWATER
    sta.b   EntityStruct.state

    // Safe to clear the byte after swimCooldown
    assert(EntityStruct.swimCooldown + 1 == EntityStruct.ladderXpos)
    stz.b   EntityStruct.swimCooldown

    jmp     _SetUnderwaterAnimation
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _SetNormalOrUnderwaterState {
    lda.b   EntityStruct.yPos.px
    cmp.w   Project.GameState.Words.waterPosition
    bcs     _SetUnderwaterState

Fallthrough:
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function _SetNormalState {
    assert(pc() == _SetNormalOrUnderwaterState.Fallthrough)

    assert(State.NORMAL == 0)
    stz.b   EntityStruct.state

    jmp     _SetAnimation
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process__Underwater {
    sep     #$20
a8()

    lda.w   InputBuffer.attack
    bne     __SetUnderwaterAttackState_A8

    // Cannot fire a projectile underwater

    rep     #$30
a16()
i16()
    jsr     _UnderwaterMovement

    lda.b   EntityStruct.yPos.px
    clc
    adc.w   #SWIMMING_YPOS_BEFORE_OUT_OF_WATER
    cmp.w   Project.GameState.Words.waterPosition
    bcs     InWater
        // Jump out of the water
        asl.b   EntityStruct.yMomentum.px - 1

        // ::TODO splash sound::
        // ::TODO splash particle::

        jmp     _SetNormalState
    InWater:

    jmp     _SetUnderwaterAnimation
}


// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// RETURNS: 16 bit A
// INPUT: dp = entity
a8()
i16()
code()
function __SetUnderwaterAttackState_A8 {
    stz.w   InputBuffer.attack

    lda.b   #State.UNDERWATER_ATTACK
    sta.b   EntityStruct.state

    // ::TODO underwater attack animation::
    lda.b   EntityStruct.movementState
    and.b   #3
    ora.b   #{GetAnimationId(melee_left)}
    jmp     MetaSprite.Animation.SetAnimationId
}


// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process__UnderwaterAttack {
    jsr     _UnderwaterMovement

    // Switch to underwater state when attack animation is over
    lda.b   EntityStruct.MetaSprite.Animation.id - 1
    cmp.w   #0xff00
    bcc     +
        jmp     _SetUnderwaterState
    +
    rts
}


// Called by interactive tile
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, Y = tile index
a16()
i16()
code()
function SetLadderStateIfUpDownPressed {
    lda.w   Controller.Joy1.pressed
    bit.w   #JOY.up | JOY.down
    beq     Return
        lda.w   MetaTiles.map.data,y
        and.w   #0xff
        tax
        lda.w   MetaTiles.Tileset.collisionMap,x
        and.w   #0xff
        cmp.w   #MetaTiles.TileCollisionType.UP_PLATFORM
        bne     +
            // Move the player down 1 unit so it is below an up-platform tile
            inc.b   EntityStruct.yPos.px
            bra     ++
        +
        cmp.w   #MetaTiles.TileCollisionType.DOWN_PLATFORM
        bne     +
            // Move the player down 1 unit so it is below an up-platform tile
            inc.b   EntityStruct.yPos.px
        +

        ldx.w   #State.LADDER
        stx.b   EntityStruct.state

        rep     #$30
    a16()
        lda.b   EntityStruct.xPos.px
        and.w   #0xfff0
        ora.w   #8
        sta.b   EntityStruct.ladderXpos

Return:
    rep     #$30
a16()
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process__Ladder {
constant _tmp = zpTmp0

    // Clear momentum
    assert(EntityStruct.xMomentum.sx + 3 == EntityStruct.yMomentum.sx)
    stz.b   EntityStruct.xMomentum.sx
    stz.b   EntityStruct.xMomentum.sx + 2
    stz.b   EntityStruct.xMomentum.sx + 4


    // Check controller
    sep     #$20
a8()
    lda.b   #Entity.MovementState.NO_GRAVITY_FLAG | Entity.MovementState.MOVING_DOWN_FLAG
    tsb.b   EntityStruct.movementState


    lda.w   Controller.Joy1.current + 1
    bit.b   #JOYH.up | JOYH.down
    beq     NoUpDown
        bit.b   #JOYH.up
        beq     +
            // up pressed
            lda.b   #Entity.MovementState.MOVING_DOWN_FLAG
            trb.b   EntityStruct.movementState
        +

        ldx.w   #LADDER_Y_MOMENTUM
        stx.b   EntityStruct.yMomentum.sx + 1

        // Nudge player towards the middle of the ladder
        rep     #$30
    a16()
        lda.b   EntityStruct.xPos.px
        cmp.b   EntityStruct.ladderXpos
        beq     NoNudge
            bcc     +
                dec.b   EntityStruct.xPos.px
                bra     ++
            +
                inc.b   EntityStruct.xPos.px
            +
        NoNudge:

        sep     #$20
    a8()
        bra     EndIf

NoUpDown:
    // A = Joy1.current + 1
    bit.b   #JOYH.left | JOYH.right
    beq     NoLeftRight
        // left/right inputs must also set facing direction
        lsr
        lda.b   #Entity.MovementState.MOVING_RIGHT_FLAG | Entity.MovementState.FACING_RIGHT_FLAG
        bcs     +
            // left pressed
            trb.b   EntityStruct.movementState
            bra     ++
        +
            // right pressed
            tsb.b   EntityStruct.movementState
        +

        ldx.w   #LADDER_X_MOMENTUM
        stx.b   EntityStruct.xMomentum.sx + 1
NoLeftRight:
EndIf:

    // Test for jumping
    lda.w   InputBuffer.jump
    beq     NoJump
        stz.w   InputBuffer.jump

        // Enable gravity
        lda.b   #Entity.MovementState.STANDING_FLAG | Entity.MovementState.NO_GRAVITY_FLAG
        trb.b   EntityStruct.movementState

        ldx.w   #MovementTableIndex.PlayerWalk
        jsr     SetJumpYVelocity__a8i16

        rep     #$30
    a16()
    i16()
        jmp     _SetNormalOrUnderwaterState
    NoJump:


    rep     #$30
a16()
i16()
    MovePlayerWithTileCollisionsAndInteractiveTiles()

    // Test if still on the ladder
    lda.b   BaseEntity.yPos.px
    and.w   MetaTiles.map.yPositionMask
    assert(MetaTiles.METATILE_SIZE_PX == 1 << 4)
    lsr
    lsr
    lsr
    lsr
    ora.w   MetaTiles.MoveEntityAndProcessTileCollisions__far._tileIndexTmp
    tax
    // X = map index at the tile
    lda.w   MetaTiles.map.data,x
    sep     #$30
a8()
i8()
    tax
    lda.w   MetaTiles.Tileset.interactiveTilesMap,x
    and.b   #MetaTiles.Tileset.interactiveTilesMap.MASK
    cmp.b   #InteractiveTiles.LADDER_INTERACTIVE_TILES_INDEX
    beq     +
        // Not on a ladder

        // Enable gravity
        lda.b   #Entity.MovementState.STANDING_FLAG | Entity.MovementState.NO_GRAVITY_FLAG
        trb.b   EntityStruct.movementState

        assert(LADDER_COYOTE_TIME_FRAMES < 9)
        lda.b   #1 << (LADDER_COYOTE_TIME_FRAMES - 1)
        sta.b   EntityStruct.coyoteJumpBuffer

        // Not on a ladder
        stz.b   EntityStruct.yMomentum.px

        rep     #$30
    a16()
    i16()
        stz.b   EntityStruct.yMomentum.sx

        jmp     _SetNormalOrUnderwaterState
    +


    // Use player y-position to determine metasprite frame
    assert({GetFrameId(ladder_0)} == 0)
    assert({GetFrameId(ladder_1)} == 1)
    assert({GetFrameId(ladder_0_up)} == {GetFrameId(ladder_0)} | Entity.MovementState.UP_GRAVITY_FLAG)
    assert({GetFrameId(ladder_1_up)} == {GetFrameId(ladder_1)} | Entity.MovementState.UP_GRAVITY_FLAG)

    lda.b   EntityStruct.movementState
    and.b   #Entity.MovementState.UP_GRAVITY_FLAG
    sta.w   _tmp

    lda.b   EntityStruct.yPos.px
    and.b   #8
    lsr
    lsr
    lsr
    ora.w   _tmp

    rep     #$30
a16()
i16()
    jmp     MetaSprite.DisableAnimationAndSetFrame
}

}
}
}

