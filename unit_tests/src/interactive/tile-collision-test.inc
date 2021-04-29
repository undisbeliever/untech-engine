// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//  * MetaTiles.Render
//  * MetaTiles.LoadTileset
//  * MetaTiles.MoveEntityAndProcessTileCollisions
//  * MetaSprite Tile Collision Hitbox
//  * MetaTile Tile Priority

namespace Entities {
namespace Players {

namespace TileCollisionTestPlayer {
    buildFunctionTable(BasePlayerFunctionTable, TileCollisionTestPlayer)

    namespace EntityStruct {
    childstruct(BaseEntity)
        field(currentFrame, 2)
        field(tcReturnCarry, 1)
        field(tcReturnY, 2)
    endstruct()
    }

    constant X_MOMENTUM  = 0x028000
    constant Y_MOMENTUM  = 0x010000


    constant DrawPlayer = MetaSprite.Render.RenderEntity


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Init {
        lda.w   #0
        sta.b   EntityStruct.currentFrame
        SetFrame_A()

        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    RoomAboutToChange:
    HitboxCollision:
    HurtboxCollision:
    ShieldCollision:
    function Null {
        rts
    }


    // REQUIRES: Map Loaded
    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Process {
        // Confirm entity momentum is a 24 bit value
        assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)

        // Update movement
        lda.w   Controller.Joy1.current
        bit.w   #JOY.left | JOY.right
        beq     NoLeftRight
            bit.w   #JOY.left
            beq     +
                lda.w   #Entity.MovementState.MOVING_RIGHT_FLAG
                trb.b   EntityStruct.movementState
                bra     ++
            +
            bit.w   #JOY.right
            beq     +
                lda.w   #Entity.MovementState.MOVING_RIGHT_FLAG
                tsb.b   EntityStruct.movementState
            +

            lda.w   #X_MOMENTUM
            sta.b   BaseEntity.xMomentum.sx
            lda.w   #X_MOMENTUM >> 8
            sta.b   BaseEntity.xMomentum.sx + 1

            bra     EndLeftRight

        NoLeftRight:
            stz.b   BaseEntity.xMomentum.sx
            stz.b   BaseEntity.xMomentum.sx + 1
    EndLeftRight:


        lda.w   Controller.Joy1.current
        bit.w   #JOY.up | JOY.down
        beq     NoUpDown
            bit.w   #JOY.up
            beq     +
                lda.w   #Entity.MovementState.MOVING_DOWN_FLAG
                bit.b   EntityStruct.movementState
                beq     +
                    trb.b   EntityStruct.movementState
                    // Clear standing flag
                    lda.w   #Entity.MovementState.STANDING_FLAG
                    trb.b   EntityStruct.movementState
            +
            lda.w   Controller.Joy1.current
            bit.w   #JOY.down
            beq     +
                lda.w   #Entity.MovementState.MOVING_DOWN_FLAG
                bit.b   EntityStruct.movementState
                bne     +
                    tsb.b   EntityStruct.movementState
                    // Clear standing flag
                    lda.w   #Entity.MovementState.STANDING_FLAG
                    trb.b   EntityStruct.movementState
            +

            lda.w   #Y_MOMENTUM
            sta.b   BaseEntity.yMomentum.sx
            lda.w   #Y_MOMENTUM >> 8
            sta.b   BaseEntity.yMomentum.sx + 1

            bra     EndUpDown

        NoUpDown:
            lda.b   BaseEntity.movementState
            bit.w   #Entity.MovementState.NO_GRAVITY_FLAG
            beq     +
                // No Gravity, clear momentum when not pressing up or down
                stz.b   BaseEntity.yMomentum.sx
                stz.b   BaseEntity.yMomentum.sx + 1
            +
    EndUpDown:


        // Change frame on L press
        lda.w   Controller.Joy1.pressed
        bit.w   #JOY.l
        beq     +
            assert({ExportOrder}.Frames.frame8 == 7)
            lda.b   EntityStruct.currentFrame
            inc
            and.w   #7
            sta.b   EntityStruct.currentFrame
            SetFrame_A()
        +

        // Change gravity settings on R press
        sep     #$20
    a8()
        lda.w   Controller.Joy1.pressed
        bit.b   #JOYL.r
        beq     EndChangeGravity
            // Alternate between down gravity, up gravity and no gravity
            lda.b   EntityStruct.movementState
            bit.b   #Entity.MovementState.NO_GRAVITY_FLAG
            beq     +
                and.b   #~(Entity.MovementState.NO_GRAVITY_FLAG | Entity.MovementState.UP_GRAVITY_FLAG)
                bra     ++
            +
            lda.b   EntityStruct.movementState
            eor.b   #Entity.MovementState.UP_GRAVITY_FLAG
            bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
            bne     +
                ora.b   #Entity.MovementState.NO_GRAVITY_FLAG
                ora.b   #Entity.MovementState.UP_GRAVITY_FLAG
            +
            sta.b   EntityStruct.movementState
    EndChangeGravity:

        rep     #$20
    a16()

        MoveEntityAndProcessTileCollisions()
        sty.b   EntityStruct.tcReturnY
        sep     #$20
    a8()
        // Carry set/clear from MoveEntityAndProcessTileCollisions call
        lda.b   #0
        adc.b   #0
        sta.b   EntityStruct.tcReturnCarry


        // Move camera to center of entity
        rep     #$30
    a16()
        rts
    }
}
}
}


namespace InteractiveTests {
namespace TileCollisionTest {

constant GRAVITY = 1555


a16()
i16()
code()
Test.add("Tile Collision Test")
function Test_Small {
    lda.w   #Project.RoomList.TileCollisionTestSmall
    bra     Test
}


a16()
i16()
code()
Test.add("Tile Collision Test")
function Test_Large {
    lda.w   #Project.RoomList.TileCollisionTestLarge
    bra     Test
}


namespace Test {
a16()
i16()
Fail:
    clc
    rts
}


// Tile Collision Test
// INPUT: A = room Id
a16()
i16()
code()
function Test {

    // A = room Id
    ldx.w   #Project.PlayerIds.TileCollisionTestPlayer
    ldy.w   #2              // Test loading a non-zero entranceId
    jsr     GameLoop.Init_LoadRoom


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx


    jsr     Test.PrintCurrentTestName

    Text.Console.SetCursor(0, 21)
    Text.Console.PrintString(UnitTestMessage)




    jsr     Resources.TransferToPpu

    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    sep     #$20
a8()

    jsr     EnableDisplay_Full


    Text.Console.SetCursor(0, 3)
    Text.Console.PrintConstString("Map Height:    ")

    Text.Console.SetCursor(12, 3)
    ldy.w   MetaTiles.map.bytesPerColumn
    jsr     Text.Console.PrintU16Y


    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        jsr     GameLoop.ProcessFrame


        sep     #$20
    a8()
        Text.Console.SetCursor(25, 2)

        lda.w   Entity.Player.movementState
        jsr     Text.Console.PrintHex8A

        Text.Console.SetCursor(21, 3)
        lda.w   Entity.playerEntity + Entities.Players.TileCollisionTestPlayer.EntityStruct.tcReturnCarry
        beq     +
            Text.Console.PrintConstString("C ")
            bra     ++
        +
            Text.Console.PrintConstString("  ")
        +

        ldy.w   Entity.playerEntity + Entities.Players.TileCollisionTestPlayer.EntityStruct.tcReturnY
        jsr     Text.Console.PrintHex16Y

        rep     #$20
    a16()


        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Resources.AnimatedTileset.DisableAnimation()
    Resources.Palette.DisableAnimation()

    jmp     Test.ResetPpuState
}


// ROM Data
// ========

rodata(rom0)
UnitTestMessage:
    db  "Use D-PAD to move entity\n"
    db  "Press L to change size\n"
    db  "Press R to change gravity\n"
    db  0

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

