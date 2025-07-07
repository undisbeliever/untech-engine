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


// Number of frames after standing before the player can jump
// ::TODO adjust::
constant COYOTE_TIME_FRAMES = 8

// ::TODO adjust::
constant LADDER_COYOTE_TIME_FRAMES = 5


// Player entity
//
// Manually controlled by Joypad1
namespace Player {

namespace State {
    createEnum(0, 2)
        enum(NORMAL)
        enum(LADDER)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__Normal
    dw  Process__Ladder


buildFunctionTable(BasePlayerFunctionTable, Player)

constant DrawPlayer = MetaSprite.Render.RenderEntity


namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)

        // Used for coyote time
        //  * Right shifted every frame
        //  * Set to (1 << COYOTE_TIME_FRAMES - 1) if standing.
        //
        // (byte)
        field(coyoteJumpBuffer, 1)


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

    // Set Facing right
    lda.w   #Entity.MovementState.FACING_RIGHT_FLAG
    tsb.b   EntityStruct.movementState

    SetAnimation(standing_right)

    rts
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
// INPUT: dp = entity
a16()
i16()
code()
function Process {
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
    sep     #$20
a8()

    ldx.w   #MovementTableIndex.PlayerWalk

    lda.w   Controller.Joy1.current + 1
    bit.b   #JOY_H_RUN_BUTTON
    beq     NoRun
        assert(MovementTableIndex.PlayerRun - MovementTableIndex.PlayerWalk == 2)
        inx
        inx
    NoRun:

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

constant _rightFlag = zpTmp0

    sep     #$20
a8()

    // ::TODO do not change animation if firing/blocked/hurt etc::


    assert(Entity.MovementState.FACING_RIGHT_FLAG == 1)
    lda.b   EntityStruct.movementState
    and.b   #1
    sta.w   _rightFlag

    // ::TODO up gravity::

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
                bra     _SetAnimation

        NoLeft:
            // A = Joy1.current + 1
            bit.b   #JOYH.right
            beq     NoRight
                lda.b   EntityStruct.movementState
                bit.b   #Entity.MovementState.MOVING_RIGHT_FLAG
                bne     WalkingOrRunning
                    lda.b   #{GetAnimationId(sliding_left)}
                    bra     _SetAnimation

                WalkingOrRunning:
                    ldx.b   EntityStruct.xMomentum.px - 1
                    cpx.w   #MIN_RUN_MOMENTUM
                    bcs     _Running
                        lda.b   #{GetAnimationId(walking_left)}
                        bra     _SetAnimation

                    _Running:
                        lda.b   #{GetAnimationId(running_left)}
                        bra     _SetAnimation

        NoRight:
            ldx.b   EntityStruct.xMomentum.px - 1
            beq     Stopped
                lda.b   #{GetAnimationId(breaking_left)}
                bra     _SetAnimation

            Stopped:
                lda.b   #{GetAnimationId(standing_left)}
                bra     _SetAnimation

    NotStanding:
        // A = movementState
        bit.b   #Entity.MovementState.MOVING_DOWN_FLAG
        beq     +
            lda.b   #{GetAnimationId(falling_left)}
            bra     _SetAnimation
        +
        lda.b   #{GetAnimationId(rising_left)}
        bra     _SetAnimation


a8()
_SetAnimation:
    clc
    adc.w   _rightFlag

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
function Process__Normal {
    jsr     _Movement
    jmp     _SetAnimation
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

        // Set normal state
        assert(State.LADDER & 0xff00 == State.NORMAL)
        stz.b   EntityStruct.state

        // Set jump velocity
        ldx.w   #MovementTableIndex.PlayerWalk
        jsr     SetJumpYVelocity__a8i16
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
    cmp.b   #2
    beq     +
        // Not on a ladder

        // Enable gravity
        lda.b   #Entity.MovementState.STANDING_FLAG | Entity.MovementState.NO_GRAVITY_FLAG
        trb.b   EntityStruct.movementState

        assert(LADDER_COYOTE_TIME_FRAMES < 9)
        lda.b   #1 << (LADDER_COYOTE_TIME_FRAMES - 1)
        sta.b   EntityStruct.coyoteJumpBuffer

        // Not on a ladder
        assert(State.LADDER & 0xff00 == State.NORMAL)
        stz.b   EntityStruct.state

        stz.b   EntityStruct.yMomentum.px

        rep     #$30
    a16()
    i16()
        stz.b   EntityStruct.yMomentum.sx

        rts
    +


    // Use player y-position to determine metasprite frame
    assert({GetFrameId(ladder_0)} == 0)
    assert({GetFrameId(ladder_1)} == 1)
    lda.b   EntityStruct.yPos.px
    and.b   #8
    lsr
    lsr
    lsr

    rep     #$30
a16()
i16()
    jmp     MetaSprite.SetFrame
}

}
}
}

