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
namespace WalkingTankBoss {


namespace State {
    createEnum(0, 2)
        enum(ATTACKING_SCIENTISTS)
        enum(ATTACKING_PLAYER)
        enum(CHARGING)
        enum(WALL_COLLISION)
        enum(TURN)
        enum(HURT)
        enum(DEFEATED)
    endEnum()
}

ProcessFunctionTable:
    dw  Process__AttackScientists
    dw  Process__AttackingPlayer
    dw  Process__Charging
    dw  Process__WallCollision
    dw  Process__Turn
    dw  Process__Hurt
    dw  Process__Defeated


buildFunctionTable(BaseEntityFunctionTable, WalkingTankBoss)

// Having this be even on a perfect playthrough will have the net fall to the left side, which looks better
constant STARTING_HEALTH = 6


constant ATTACK_SCIENTISTS_COOLDOWN = 90

constant MIN_ATTACK_PLAYER_COOLDOWN = 40
constant ATTACK_PLAYER_SPEEDUP_PER_HURT = 5

constant ATTACKS_BEFORE_CHARGING = 3

constant CHARGE_DELAY = 20

// 8.8 fixed point
constant WALL_COLLISION_XMOMENTUM = 0x0080
constant WALL_COLLISION_YMOMENTUM = 0x0180

constant MIN_WALL_COLLISION_TIME = 30
constant EXTRA_WALL_COLLISION_TIME_PER_HP = 20



namespace EntityStruct {
    childstruct(BaseEntity)
        field(state, 2)
        field(health, 2)

        field(countdown, 2)

        field(attackCooldown, 2)

        field(attacksRemaining, 2)
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
    assert(State.ATTACKING_SCIENTISTS == 0)
    stz.b   EntityStruct.state

    stz.b   EntityStruct.countdown


    lda.w   #STARTING_HEALTH
    sta.b   EntityStruct.health

    lda.w   #MIN_ATTACK_PLAYER_COOLDOWN + ATTACK_PLAYER_SPEEDUP_PER_HURT * STARTING_HEALTH
    sta.b   EntityStruct.attackCooldown


    sep     #$20
a8()

    LoadSong(BossTheme)


    lda.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
    tsb.b   EntityStruct.movementState


    rep     #$30
a16()
i16()
    lda.w   #{GetAnimationId(attack_right)}
    jmp     MetaSprite.Animation.SetAnimationId
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
function Process__AttackScientists {

    dec.b   EntityStruct.countdown
    bpl     +
        lda.w   #ATTACK_SCIENTISTS_COOLDOWN
        sta.b   EntityStruct.countdown

        ldy.w   #{GetAnimationId(attack_left)}
        jsr     SetAnimation_Facing_Y
    +

    MoveEntityAndProcessTileCollisions()


    lda.w   Entity.Player.yPos.px
    sec
    sbc.b   EntityStruct.yPos.px
    bcc     +
        jsr     SetState__AttackingPlayer

        stz.b   EntityStruct.attacksRemaining
    +



    jmp     Entity.DrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState__AttackingPlayer {
    sep     #$20
a8()
    SetProjectileId(LargeFireball)

    lda.b   #State.ATTACKING_PLAYER
    sta.b   EntityStruct.state

    rep     #$30
a16()

    lda.b   EntityStruct.attackCooldown
    sta.b   EntityStruct.countdown

    lda.w   #ATTACKS_BEFORE_CHARGING - 1
    sta.b   EntityStruct.attacksRemaining

    ldy.w   #{GetAnimationId(attack_left)}
    jmp     SetAnimation_Facing_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__AttackingPlayer {
    dec.b   EntityStruct.countdown
    bpl     +
        // Animation id is 0xff if animation ended
        lda.b   EntityStruct.MetaSprite.Animation.id - 1
        cmp.w   #0xff00
        bcc     +
            lda.b   EntityStruct.attackCooldown
            sta.b   EntityStruct.countdown

            dec.b   EntityStruct.attacksRemaining
            bmi     SetStateAndGotoNextEntity__Charging

            ldy.w   #{GetAnimationId(attack_left)}
            jsr     SetAnimation_Facing_Y
    +

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetStateAndGotoNextEntity__Charging {
    lda.w   #State.CHARGING
    sta.b   EntityStruct.state

    lda.w   #CHARGE_DELAY
    sta.b   EntityStruct.countdown

    ldy.w   #{GetAnimationId(charge_left)}
    jsr     SetAnimation_Facing_Y

    jmp     Entity.DrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Charging {
    lda.b   EntityStruct.countdown
    beq     +
        dec.b   EntityStruct.countdown
        bra     ++
    +
        ldx.w   #MovementTableIndex.WalkingTankBoss
        jsr     AccelerateInFacingDirection
    +

    MoveEntityAndProcessTileCollisions()


    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
        jsr     SetState__WallCollision
    +

    jmp     Entity.DrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState__WallCollision {
    sep     #$20
a8()
    lda.b   EntityStruct.movementState
    eor.b   #Entity.MovementState.MOVING_RIGHT_FLAG
    and.b   #~(Entity.MovementState.STANDING_FLAG | Entity.MovementState.MOVING_DOWN_FLAG)
    sta.b   EntityStruct.movementState

    QueueSoundEffect(bossHitsWall)


    lda.b   #State.WALL_COLLISION
    sta.b   EntityStruct.state


    lda.b   EntityStruct.health
    sta.l   WRMPYA

    lda.b   #EXTRA_WALL_COLLISION_TIME_PER_HP
    sta.l   WRMPYB

    // must wait 8 cycles before reading RDMPY

    ldx.w   #WALL_COLLISION_XMOMENTUM
    stx.b   EntityStruct.xMomentum.px - 1

    ldx.w   #WALL_COLLISION_YMOMENTUM
    stx.b   EntityStruct.yMomentum.px - 1

    rep     #$30
a16()

    lda.l   RDMPY
    clc
    adc.w   #MIN_WALL_COLLISION_TIME
    sta.b   EntityStruct.countdown


    ldy.w   #{GetAnimationId(wall_collision_left)}
    jmp     SetAnimation_Facing_Y
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WallCollision {
    dec.b   EntityStruct.countdown
    bmi     SetStateAndGotoNextEntity__Turn

    lda.b   EntityStruct.movementState
    bit.w   #Entity.MovementState.STANDING_FLAG
    beq     +
        stz.b   EntityStruct.xMomentum.sx
        stz.b   EntityStruct.xMomentum.px - 1
    +

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetStateAndGotoNextEntity__Turn {
    // Assumes moving direction changed when switching to WallCollision state
    lda.b   EntityStruct.movementState
    eor.w   #Entity.MovementState.FACING_RIGHT_FLAG
    sta.b   EntityStruct.movementState

    lda.w   #State.TURN
    sta.b   EntityStruct.state

    ldy.w   #{GetAnimationId(turn_left)}
    jsr     SetAnimation_Facing_Y

    jmp     Entity.DrawAndGotoNextEntity
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
    bcc     +
        jsr     SetState__AttackingPlayer
    +

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DrawAndGotoNextEntity
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HurtboxCollision {

    // Prevent enemy from sliding when hurt/defeated
    stz.b   EntityStruct.xMomentum.sx
    stz.b   EntityStruct.xMomentum.px - 1

    lda.b   EntityStruct.state
    cmp.w   #State.WALL_COLLISION
    bne     NotDizzy
        lda.b   EntityStruct.health
        dec
        beq     SetState__Defeated
        bmi     SetState__Defeated
        sta.b   EntityStruct.health

        bra     SetState__Hurt


    NotDizzy:
        sep     #$20
    a8()
        QueueSoundEffect(projectileShieldBounce)
        rep     #$30
    a16()
        rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState__Hurt {
    lda.w   #State.HURT
    sta.b   EntityStruct.state

    sep     #$20
a8()
    QueueSoundEffect(bossHurt)
    rep     #$30
a16()

    lda.b   EntityStruct.attackCooldown
    sec
    sbc.w   #ATTACK_PLAYER_SPEEDUP_PER_HURT
    cmp.w   #MIN_ATTACK_PLAYER_COOLDOWN
    bcs     +
        lda.w   #MIN_ATTACK_PLAYER_COOLDOWN
    +
    sta.b   EntityStruct.attackCooldown


    ldy.w   #{GetAnimationId(hurt_left)}
    jmp     SetAnimation_Facing_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Hurt {
    // Animation id is 0xff if animation ended
    lda.b   EntityStruct.MetaSprite.Animation.id - 1
    cmp.w   #0xff00
    bcc     +
        jmp     SetStateAndGotoNextEntity__Turn
    +

    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function SetState__Defeated {
    lda.w   #State.DEFEATED
    sta.b   EntityStruct.state


    ldx.w   #0x1000 + 208
    ldy.w   #0x1000 + 42
    stx.w   SpawnEntity.xPos
    sty.w   SpawnEntity.yPos

    ldy.b   EntityStruct.instanceId
    SpawnEntity(boss_net)

    ldy.w   #{GetAnimationId(wall_collision_left)}
    jmp     SetAnimation_Facing_Y
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__Defeated {


    MoveEntityAndProcessTileCollisions()

    jmp     Entity.DrawAndGotoNextEntity
}

}
}
