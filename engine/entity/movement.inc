// entity/movement.inc
// ===================
//
// Entity Movement Routines.
//
// NOTE: These routines do not handle tile-collisions.
//       Please see `src/metatiles/collisions.inc` for a movement routine with tile-collisions.
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


// ::TODO move into room subsystem::
// (0:0:16 unsigned fixed point)
allocate(Room.gravity_sx, wram7e, 2)

namespace Entity {
namespace Movement {

// The maximum momentum allowed by the engine
constant MAX_MOMENTUM = 0x080000

// Confirm entity momentum is a 24 bit value
assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)

// Generate a Y-Axis movement function table
//
// PARAM: prefix = the prefix on all the functions
// PARAM: processStanding = If zero then the STANDING_FLAG will be ignored
inline _GenerateYAxisMovementStateFunctionTable(define prefix, evaluate processStanding) {
    assert(Entity.MovementState.UP_GRAVITY_FLAG  == %00010)
    assert(Entity.MovementState.MOVING_DOWN_FLAG == %00100)
    assert(Entity.MovementState.STANDING_FLAG    == %01000)
    assert(Entity.MovementState.NO_GRAVITY_FLAG  == %10000)

    assert(Entity.MovementState.UP_GRAVITY_FLAG | Entity.MovementState.MOVING_DOWN_FLAG | Entity.MovementState.STANDING_FLAG | Entity.MovementState.NO_GRAVITY_FLAG == 30)

    evaluate _n = 0
    while {_n} < 32 {
        if {_n} & Entity.MovementState.MOVING_DOWN_FLAG {
            define _label = Down
        } else {
            define _label = Up
        }

        if {processStanding} {
            if {_n} & Entity.MovementState.STANDING_FLAG {
                define _label = Standing_{_label}
            } else {
                define _label = Falling_{_label}
            }
        }

        if {_n} & Entity.MovementState.NO_GRAVITY_FLAG {
            define _label = NoGravity_{_label}
        } else {
            if {_n} & Entity.MovementState.UP_GRAVITY_FLAG {
                define _label = UpGravity_{_label}
            } else {
                define _label = DownGravity_{_label}
            }
        }
        dw  {prefix}_{_label}
        evaluate _n = {_n} + 2
    }
}
constant N_Y_AXIS_MOVEMENT_STATE_FUNCTIONS = 16
constant Y_AXIS_MOVEMENT_STATE_FUNCTION_TABLE_MASK = (N_Y_AXIS_MOVEMENT_STATE_FUNCTIONS - 1) * 2


// Adds a 24 bit momentum to a 32 bit position
//
// REQUIREMENTS: 16 bit A
// PARAM: axis (x/y) to preform the addition upon
// INPUT: dp = entity
// OUTPUT: A = pixel position on {axis}
macro _AddMomentumToPosition(axis) {
    assert16a()

    clc
    lda.b   BaseEntity.{axis}Momentum.sx
    adc.b   BaseEntity.{axis}Pos.sx
    sta.b   BaseEntity.{axis}Pos.sx

    lda.b   BaseEntity.{axis}Momentum.px
    and.w   #0xff
    adc.b   BaseEntity.{axis}Pos.px
    sta.b   BaseEntity.{axis}Pos.px
}


// Subtracts a 24 bit momentum from a 32 bit position
//
// REQUIREMENTS: 16 bit A
// PARAM: axis (x/y) to preform the addition upon
// INPUT: dp = entity
// OUTPUT: A = pixel position on {axis}
macro _SubtractMomentumFromPosition(axis) {
    assert16a()

    sec
    lda.b   BaseEntity.{axis}Pos.sx
    sbc.b   BaseEntity.{axis}Momentum.sx
    sta.b   BaseEntity.{axis}Pos.sx

    lda.b   BaseEntity.{axis}Momentum.px
    and.w   #0xff
    eor.w   #0xffff
    adc.b   BaseEntity.{axis}Pos.px
    sta.b   BaseEntity.{axis}Pos.px
}


// Add 16 bit gravity to 24 bit yMomentum, clamping yMomentum to MAX_MOMENTUM
//
// REQUIREMENTS: 16 bit A
// INPUT: dp = entity
macro _AddGravityToYMomentum() {
    assert16a()

    clc
    lda.b   BaseEntity.yMomentum.sx
    adc.w   Room.gravity_sx
    sta.b   BaseEntity.yMomentum.sx
    bcc     EndIf{#}
        lda.b   BaseEntity.yMomentum.px - 1
        // carry set
        adc.w   #0x0100 - 1

        // ::TODO make entity & region independent ::
        cmp.w   #Entity.Movement.MAX_MOMENTUM >> 8
        bcc     NotMaxMomentum{#}
            lda.w   #Entity.Movement.MAX_MOMENTUM >> 8
    NotMaxMomentum{#}:
        sta.b   BaseEntity.yMomentum.px - 1
EndIf{#}:
}


// Subtract 16 bit gravity from 24 bit yMomentum.
// If yMomentum becomes negative, set Y direction to {change_direction} and goto {branch}.
//
// REQUIREMENTS: 16 bit A
// INPUT: dp = entity
// PARAM: branch = label to brnach to if direction changes
// PARAM: change_direction = new direction to go to if yMomentum becomes negative
// BRANCH: goto {branch} label if Y direction changes
macro _SubtractGravityFromYMomentum_AndBranchIfDirectionChanges(branch, change_direction) {
    assert16a()

    // Subtract 16 bit gravity from 24 bit yMomentum
    sec
    lda.b   BaseEntity.yMomentum.sx
    sbc.w   Room.gravity_sx
    sta.b   BaseEntity.yMomentum.sx
    bcs     End{#}
        sep     #$20
    a8()
        dec.b   BaseEntity.yMomentum.px
        bpl     YMomentum_Positive{#}
            // yMomentum is negative, change Y direction and negate momentum

            if Entity.Movement.__Y_Axis.{change_direction} == Entity.Movement.__Y_Axis.Down {
                lda.b   #Entity.MovementState.MOVING_DOWN_FLAG
                tsb.b   BaseEntity.movementState
            } else {
                lda.b   #Entity.MovementState.MOVING_DOWN_FLAG
                trb.b   BaseEntity.movementState
            }

            stz.b   BaseEntity.yMomentum.px

            // yMomentum.sx = -yMomentum.sx
            rep     #$21
        a16()
            // carry clear
            lda.w   #1
            sbc.b   BaseEntity.yMomentum.sx
            sta.b   BaseEntity.yMomentum.sx

            jmp     {branch}
    YMomentum_Positive{#}:
        rep     #$30
    a16()
End{#}:
}
namespace __Y_Axis {
    constant Up = 1
    constant Down = 2
}



// Update entity position based on its momentum and movementState
//
// NOTE: The entity's standing flag is ignored by this routine.
//
// NOTE: This routine will apply gravity to the entity's `yMomentum` if the
//       entity's `NO_GRAVITY_FLAG` movementState flag is clear.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT:    dp = entity
a16()
i16()
code()
function MoveEntity {
    assert(Entity.MovementState.MOVING_RIGHT_FLAG == 0x40)
    bit.b   BaseEntity.movementState - 1
    bvc     MovingLeft
        // Entity moving rightwards
        _AddMomentumToPosition(x)
        bra     EndLeftRight

    MovingLeft:
        // Entity moving leftwards
        _SubtractMomentumFromPosition(x)

EndLeftRight:

    lda.b   BaseEntity.movementState
    and.w   #Entity.Movement.Y_AXIS_MOVEMENT_STATE_FUNCTION_TABLE_MASK
    tax
    jmp     (YAxis_FunctionTable,x)



YAxis_FunctionTable:
    _GenerateYAxisMovementStateFunctionTable(YAxis, 0)
assert(pc() - YAxis_FunctionTable == N_Y_AXIS_MOVEMENT_STATE_FUNCTIONS * 2)


YAxis_UpGravity_Up:
    _AddGravityToYMomentum()
    _SubtractMomentumFromPosition(y)
    rts


YAxis_DownGravity_Up:
    _SubtractGravityFromYMomentum_AndBranchIfDirectionChanges(YAxis_NoGravity_Down, Down)
// Fallthrough

YAxis_NoGravity_Up:
    _SubtractMomentumFromPosition(y)

    rts


YAxis_DownGravity_Down:
    _AddGravityToYMomentum()
    _AddMomentumToPosition(y)
    rts


YAxis_UpGravity_Down:
    _SubtractGravityFromYMomentum_AndBranchIfDirectionChanges(YAxis_NoGravity_Up, Up)
// Fallthrouth

YAxis_NoGravity_Down:
    _AddMomentumToPosition(y)

    rts
}



// Move entity to with tile collisions, interactive tiles and detecting ledges.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// OUTPUT: carry set if the entity is standing on a ledge
a8()
i16()
code()
function MoveEntityWithTileCollisionsInteractiveTilesAndDetectEdges {
    jsr     MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles

    assert(Entity.MovementState.MOVING_RIGHT_FLAG == 0x40)
    lda.w   #Entity.MovementState.STANDING_FLAG << 8
    bit.b   BaseEntity.movementState - 1
    beq     ReturnFalse
        // Entity is standing
        bvc     MovingLeft
            // Moving right
            // Return true if the Y-axis collision is to the right of the entity's y-position
            //   return (yCollisionMapIndex & dataIndexXmask) < tileMapIndex
            lda.w   MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles.yCollisionMapIndex
            and.w   MetaTiles.map.dataIndexXmask
            cmp.w   MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles.tileIndexTmp
            bcc     ReturnTrue
        ReturnFalse:
            clc
            rts

        ReturnTrue:
            sec
            rts

        MovingLeft:
            // Moving Left
            // Return true if the Y-axis collision is to the left of the entity's y-position
            //   return (yCollisionMapIndex & dataIndexXmask) > tileMapIndex
            lda.w   MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles.yCollisionMapIndex
            and.w   MetaTiles.map.dataIndexXmask
            dec
            cmp.w   MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles.tileIndexTmp
            rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

