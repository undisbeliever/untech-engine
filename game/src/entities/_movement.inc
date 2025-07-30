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


namespace Gravity {
    createEnum(0, 2)
        enum(NORMAL)
        enum(PLAYER)
        enum(PLAYER_HOLDING_JUMP)
        enum(SWIMMING_PLAYER)
        enum(SWIMMING_PLAYER_HOLDING_JUMP)
    endEnum()
}

rodata()
namespace Project {
    constant _PLAYER_GRAVITY = 8600
    constant _PLAYER_SWIM_GRAVITY = 2000

// 0.16 fixed point px/frame/frame
GravityTable:
    // ::TODO tweak::
    dw  _PLAYER_GRAVITY             // default
    dw  _PLAYER_GRAVITY             // player
    dw  _PLAYER_GRAVITY / 2         // player holding jump
    dw  _PLAYER_SWIM_GRAVITY        // swimming player
    dw  _PLAYER_SWIM_GRAVITY / 2    // swimming player holding jump
}


namespace Entities {



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, X = movement table index
// KEEP: X/Y
a16()
i16()
code()
function AccelerateLeft {
    assert(Entity.MovementState.MOVING_RIGHT_FLAG == 0x40)
    assert16a()
    bit.b   BaseEntity.movementState - 1
    bvs     DecelerateAndTurnXmomentum

// Fallthrough into AccelerateXmomentum
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, X = movement table index
// KEEP: X/Y
a16()
i16()
code()
function AccelerateXmomentum {
    // Assert 24 bit 0:8:16 fixed-point momentum
    assert(BaseEntity.xMomentum.sx + 2 == BaseEntity.xMomentum.px)
    assert(BaseEntity.xMomentum.px + 1 == BaseEntity.yMomentum.sx)

    lda.b   BaseEntity.xMomentum.sx
    clc
    adc.l   MovementTable.MoveAccelerationTable,x
    sta.b   BaseEntity.xMomentum.sx

    lda.b   BaseEntity.xMomentum.sx + 1
    bcc     +
        // carry set
        adc.w   #0x0100 - 1
    +
    cmp.l   MovementTable.MaxMomentum,x
    bcc     +
        stz.b   BaseEntity.xMomentum.sx
        lda.l   MovementTable.MaxMomentum,x
    +
    sta.b   BaseEntity.xMomentum.sx + 1

    rts
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, X = movement table index
// KEEP: X/Y
a16()
i16()
code()
function AccelerateRight {
    assert(Entity.MovementState.MOVING_RIGHT_FLAG == 0x40)
    assert16a()
    bit.b   BaseEntity.movementState - 1
    bvs     AccelerateXmomentum

// Fallthrough into DecelerateAndTurnXmomentum
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, X = movement table index
// KEEP: X/Y
a16()
i16()
code()
function DecelerateAndTurnXmomentum {
    // Assert 24 bit 0:8:16 fixed-point momentum
    assert(BaseEntity.xMomentum.sx + 2 == BaseEntity.xMomentum.px)
    assert(BaseEntity.xMomentum.px + 1 == BaseEntity.yMomentum.sx)

    lda.b   BaseEntity.xMomentum.sx
    sec
    sbc.l   MovementTable.TurnDecelerationTable,x
    sta.b   BaseEntity.xMomentum.sx

    bcs     ++
        sep     #$20
    a8()
        dec.b   BaseEntity.xMomentum.px
        bpl     +
            // Invert moving direction
            lda.b   BaseEntity.movementState
            eor.b   #Entity.MovementState.MOVING_RIGHT_FLAG
            sta.b   BaseEntity.movementState

            // Negate xMomentum
            stz.b   BaseEntity.xMomentum.px

            rep     #$31
        a16()
            // carry clear
            lda.w   #1
            sbc.b   BaseEntity.xMomentum.sx
            sta.b   BaseEntity.xMomentum.sx
        +

        rep     #$30
    a16()
    +

    rts
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, X = movement table index
// KEEP: X/Y
a16()
i16()
code()
function DecelerateXmomentum {
    // Assert 24 bit 0:8:16 fixed-point momentum
    assert(BaseEntity.xMomentum.sx + 2 == BaseEntity.xMomentum.px)
    assert(BaseEntity.xMomentum.px + 1 == BaseEntity.yMomentum.sx)

    lda.b   BaseEntity.xMomentum.sx
    sec
    sbc.l   MovementTable.BreakDecelerationTable,x
    sta.b   BaseEntity.xMomentum.sx

    bcs     ++
        lda.b   BaseEntity.xMomentum.sx + 1
        // carry set
        sbc.w   #0x0100
        bpl     +
            stz.b   BaseEntity.xMomentum.sx
            lda.w   #0
        +
        sta.b   BaseEntity.xMomentum.sx + 1
    +

    rts
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity, X = movement table index
// KEEP: X/Y
a8()
i16()
code()
function SetJumpYVelocity__a8i16 {
    // ::TODO should jump change based on xMomentum?::
    rep     #$30
a16()
    lda.l   MovementTable.JumpVelocity,x
    sta.b   BaseEntity.yMomentum.px - 1

    sep     #$20
a8()

    lda.b   BaseEntity.movementState
    and.b   #~(Entity.MovementState.STANDING_FLAG | Entity.MovementState.MOVING_DOWN_FLAG)
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    beq     +
        ora.b   #Entity.MovementState.MOVING_DOWN_FLAG
    +
    sta.b   BaseEntity.movementState

    rts
}


}

