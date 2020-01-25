// entity/movement.inc
// ===================
//
// Entity Movement Routines.
//
// NOTE: These routines do not handle tile-collisions.
//       Please see `src/metatiles/collisions.inc` for a movement routine with tile-collisions.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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
    assert(Entity.MovementState.MOVING_DOWN_FLAG == %00010)
    assert(Entity.MovementState.STANDING_FLAG    == %00100)
    assert(Entity.MovementState.UP_GRAVITY_FLAG  == %01000)
    assert(Entity.MovementState.NO_GRAVITY_FLAG  == %10000)


    evaluate _n = 0
    while {_n} < 32 {
        if {_n} & %00010 {
            define _label = Down
        } else {
            define _label = Up
        }

        if {processStanding} {
            if {_n} & %00100 {
                define _label = Standing_{_label}
            } else {
                define _label = Falling_{_label}
            }
        }

        if {_n} & %10000 {
            define _label = NoGravity_{_label}
        } else {
            if {_n} & %01000 {
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

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

