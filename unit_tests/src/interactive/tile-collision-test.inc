// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//  * MetaTiles.Render
//  * MetaTiles.LoadTileset
//  * MetaTiles.MoveEntityAndProcessTileCollisions
//  * MetaSprite Tile Collision Hitbox

namespace Entities {

namespace TileCollisionTestPlayer {
    buildFunctionTable(BaseEntityFunctionTable, TileCollisionTestPlayer)

    namespace EntityStruct {
    childstruct(BaseEntity)
        field(currentFrame, 2)
        field(tcReturnCarry, 1)
        field(tcReturnY, 2)
    endstruct()
    }

    constant X_MOMENTUM  = 0x028000
    constant Y_MOMENTUM  = 0x010000


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        lda.w   #0
        sta.b   EntityStruct.currentFrame
        SetFrame_A()

        // always activate
        sec
        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
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

        rep     #$31
    a16()
        // Move camera to center of object
        lda.b   EntityStruct.xPos.px
        // Carry clear
        adc.w   #Camera.DISPLAY_WIDTH / 2
        cmp.w   MetaTiles.map.right
        bcc     +
            lda.w   MetaTiles.map.right
        +
        sec
        sbc.w   #Camera.DISPLAY_WIDTH
        cmp.w   #MetaTiles.map.LEFT
        bcs     +
            lda.w   #MetaTiles.map.LEFT
        +
        sta.w   Camera.xPos

        jmp     Entity.DrawAndGotoNextEntity
    }
}
}


namespace InteractiveTests {
namespace TileCollisionTest {

constant ENTITY_X_START = 32
constant ENTITY_Y_START = 48

constant GRAVITY = 1555


// Loads the map into memory.
//
// Each time this routine is called it will change the map height
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro LoadMap() {
    assert16a()
    assert16i()

    lda.w   #Map.WIDTH
    sta.w   MetaTiles.map.width

    lda.w   #MetaTiles.map.LEFT + Map.WIDTH * 16
    sta.w   MetaTiles.map.right

    lda.w   #MetaTiles.map.TOP + Map.HEIGHT * 16
    sta.w   MetaTiles.map.bottom

    // Alternate between small and large map sizes
    lda.w   MetaTiles.map.bytesPerColumn
    cmp.w   #MetaTiles.MAP_HEIGHT_SMALL
    beq     +
        lda.w   #MetaTiles.MAP_HEIGHT_SMALL
        ldx.w   #Map.WIDTH * MetaTiles.MAP_HEIGHT_SMALL
        bra     ++
    +
        lda.w   #MetaTiles.MAP_HEIGHT_LARGE
        ldx.w   #Map.WIDTH * MetaTiles.MAP_HEIGHT_LARGE
    +
    sta.w   MetaTiles.map.bytesPerColumn
    stx.w   MetaTiles.map.dataSize


    // copy tile map

    assert(MetaTiles.MAP_SIZE % 2 == 0)
    ldx.w   #Map.Data
    ldy.w   #MetaTiles.map.data
    ColumnLoop:
        lda.w   #Map.HEIGHT - 1
        mvn     0x7e=(Map.Data >> 16)

        // Goto next column
        tya
        sec
        sbc.w   #Map.HEIGHT
        clc
        adc.w   MetaTiles.map.bytesPerColumn
        tay

        cpx.w   #Map.Data + Map.Data.size
        bcc     ColumnLoop

}


// Tile Collision Test
a16()
i16()
code()
Test.add("Tile Collision Test")
Test.add("Tile Collision Test")     // Test both small and large tile heights
function Test {
constant _entity = Test.tmp + 0

    jsr     Entity.Init

    lda.w   #Project.Scenes.TileCollisionTest
    jsr     Resources.LoadScene


    LoadMap()


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx

    lda.w   #MetaTiles.map.LEFT
    sta.w   Camera.xPos

    lda.w   #MetaTiles.map.TOP
    sta.w   Camera.yPos


    lda.w   #MetaTiles.map.LEFT + ENTITY_X_START
    sta.w   Entity.Spawn.xPos

    lda.w   #MetaTiles.map.TOP + ENTITY_Y_START
    sta.w   Entity.Spawn.yPos

    lda.w   #Project.EntityIds.TileCollisionTestPlayer
    jsr     Entity.Spawn
    bcs     +
        // carry clear
        rts
    +
    sty.w   _entity



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
        jsr     MetaSprite.Render.InitLoop

        jsr     EntityLoop.ProcessFrame

        jsr     Resources.ProcessAnimations

        jsr     MetaTiles.Render.Update

        jsr     MetaSprite.Render.EndLoop

        sep     #$20
    a8()
        Text.Console.SetCursor(25, 2)

        ldx.w   _entity
        lda.b   BaseEntity.movementState,x
        jsr     Text.Console.PrintHex8A

        Text.Console.SetCursor(21, 3)
        ldx.w   _entity
        lda.b   Entities.TileCollisionTestPlayer.EntityStruct.tcReturnCarry,x
        beq     +
            Text.Console.PrintConstString("C ")
            bra     ++
        +
            Text.Console.PrintConstString("  ")
        +

        ldx.w   _entity
        ldy.b   Entities.TileCollisionTestPlayer.EntityStruct.tcReturnY,x
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


// ::TODO replace with room subsystem::
rodata(rom0)
namespace Map {
    constant WIDTH = 32
    constant HEIGHT = 14

Data:
    db  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1f
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x18, 0x11, 0x11, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x19, 0x11, 0x11, 0x10, 0x10, 0x10, 0x10, 0x11
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x10, 0x14, 0x11, 0x1a, 0x10
    db  0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x10, 0x10, 0x14, 0x20, 0x11, 0x20, 0x1a, 0x10, 0x10
    db  0x10, 0x20, 0x11, 0x11, 0x11, 0x10, 0x10, 0x20, 0x11, 0x11, 0x11, 0x20, 0x10, 0x10, 0x10, 0x15
    db  0x20, 0x11, 0x11, 0x10, 0x10, 0x15, 0x20, 0x11, 0x20, 0x1b, 0x10, 0x10, 0x10, 0x10, 0x15, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x15, 0x11, 0x1b, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x10
    db  0x10, 0x10, 0x10, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x14, 0x11, 0x11, 0x10, 0x10, 0x10
    db  0x14, 0x11, 0x1a, 0x10, 0x10, 0x10, 0x10, 0x14, 0x20, 0x11, 0x11, 0x10, 0x10, 0x10, 0x15, 0x11
    db  0x1b, 0x10, 0x10, 0x10, 0x10, 0x20, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10, 0x16, 0x11, 0x1c, 0x10
    db  0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10, 0x17, 0x11, 0x1d, 0x10, 0x10, 0x12
    db  0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x10, 0x18, 0x11, 0x1e, 0x10, 0x10, 0x12, 0x10, 0x10
    db  0x10, 0x11, 0x11, 0x10, 0x10, 0x10, 0x19, 0x11, 0x1f, 0x10, 0x10, 0x12, 0x10, 0x10, 0x10, 0x11
    db  0x11, 0x13, 0x10, 0x10, 0x10, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x12, 0x11, 0x11, 0x10
    db  0x10, 0x10, 0x16, 0x11, 0x1c, 0x10, 0x10, 0x13, 0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x10
    db  0x17, 0x11, 0x1d, 0x10, 0x10, 0x13, 0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x16, 0x20, 0x11
    db  0x20, 0x1c, 0x10, 0x13, 0x10, 0x10, 0x16, 0x11, 0x11, 0x10, 0x10, 0x17, 0x11, 0x11, 0x11, 0x1d
    db  0x10, 0x10, 0x10, 0x10, 0x17, 0x11, 0x11, 0x10, 0x10, 0x20, 0x11, 0x11, 0x11, 0x20, 0x10, 0x10
    db  0x10, 0x16, 0x20, 0x11, 0x11, 0x10, 0x10, 0x18, 0x11, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10, 0x17
    db  0x20, 0x11, 0x11, 0x10, 0x10, 0x19, 0x20, 0x11, 0x20, 0x1f, 0x10, 0x10, 0x14, 0x20, 0x20, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x18, 0x11, 0x1e, 0x10, 0x10, 0x10, 0x20, 0x20, 0x20, 0x11, 0x11, 0x10
    db  0x10, 0x10, 0x19, 0x11, 0x1f, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    db  0x11, 0x1c, 0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x10, 0x14, 0x11, 0x10, 0x10, 0x11, 0x1d
    db  0x10, 0x10, 0x10, 0x11, 0x11, 0x10, 0x10, 0x14, 0x20, 0x11, 0x10, 0x10, 0x11, 0x20, 0x1a, 0x10
    db  0x16, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
constant Data.size = pc() - Data
assert(Data.size == WIDTH * HEIGHT)
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

