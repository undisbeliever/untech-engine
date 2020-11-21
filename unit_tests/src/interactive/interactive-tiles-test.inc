// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// This test is completely automatic.
//
// Tests:
//  * MetaTiles.Render
//  * MetaTiles.LoadTileset
//  * MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles
//  * MetaTiles.MovePlayerWithTileCollisionsAndInteractiveTiles
//  * MetaTiles.CrumblingTiles
//  * MetaSprite Tile Collision Hitbox


namespace Entities {
namespace Players {

namespace InteractiveTilesTest_Player {
    buildFunctionTable(BasePlayerFunctionTable, InteractiveTilesTest_Player)

    namespace EntityStruct {
        basestruct(BaseEntity)
            field(changeInY, 2)
            field(oldYposPx, 2)
        endstruct()
    }

    constant DrawPlayer = MetaSprite.Render.RenderEntity

    constant X_MOMENTUM_PX = 3


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Init {
        SetFrame(frame)

        stz.b   EntityStruct.changeInY
        lda.b   EntityStruct.yPos.px
        sta.b   EntityStruct.oldYposPx


        sep     #$20
    a8()
        lda.w   GameState.roomId
        cmp.b   #Project.RoomList.InteractiveTilesTestSmall
        bne     +
            lda.b   #X_MOMENTUM_PX
            sta.b   BaseEntity.xMomentum.px
        +

        rep     #$30
    a16()

        // Always activate entity
        // (This function is reused by blue "enemy" entity)
        sec
        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    RoomAboutToChange:
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
        MovePlayerWithTileCollisionsAndInteractiveTiles()

        jmp     UpdateChangeInYAndMovementState
    }


    a16()
    i16()
    code()
    function UpdateChangeInYAndMovementState {
        lda.b   EntityStruct.yPos.px
        tax
        sec
        sbc.b   EntityStruct.oldYposPx
        bcs     +
            eor.w   #0xffff
            inc
        +
        clc
        adc.b   EntityStruct.changeInY
        sta.b   EntityStruct.changeInY
        stx.b   EntityStruct.oldYposPx


        // Change movement direction on X axis collision
        sep     #$20
    a8()
        assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
        lda.b   EntityStruct.movementState
        bpl     +
            eor.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
            sta.b   EntityStruct.movementState

            lda.b   #X_MOMENTUM_PX
            sta.b   EntityStruct.xMomentum.px
        +

        rep     #$30
    a16()

        rts
    }
}
}

namespace InteractiveTilesTest_Entity {
    buildFunctionTable(BaseEntityFunctionTable, InteractiveTilesTest_Entity)

    namespace EntityStruct {
        childstruct(Players.InteractiveTilesTest_Player.EntityStruct)
        endstruct()
    }

    constant Init = Players.InteractiveTilesTest_Player.Init


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
        MoveEntityWithTileCollisionsAndInteractiveTiles()

        jsr     Players.InteractiveTilesTest_Player.UpdateChangeInYAndMovementState

        jmp     Entity.DrawAndGotoNextEntity
    }
}
}


namespace InteractiveTiles {

namespace SetUpGravity {
// An entity collided with the tile.
// INPUT: dp = entity
// INPUT:  Y = map data index
a16()
i16()
code()
EntityCollision:
EntityAirCollision:
PlayerOriginCollision:
PlayerLeftRightCollision:
PlayerAirCollision:
function Collision {
    lda.w   #Entity.MovementState.UP_GRAVITY_FLAG
    tsb.b   BaseEntity.movementState

    rts
}
}

namespace SetDownGravity {
// An entity collided with the tile.
// INPUT: dp = entity
// INPUT:  Y = map data index
a16()
i16()
code()
EntityCollision:
EntityAirCollision:
PlayerOriginCollision:
PlayerLeftRightCollision:
PlayerAirCollision:
function Collision {
    lda.w   #Entity.MovementState.UP_GRAVITY_FLAG
    trb.b   BaseEntity.movementState

    rts
}
}


namespace Token {
a16()
i16()
code()
EntityCollision:
EntityAirCollision:
PlayerOriginCollision:
PlayerLeftRightCollision:
function NullFunction {
    rts
}


// The player touched a token tile
// INPUT: dp = entity
// INPUT:  Y = map data index
function PlayerAirCollision {
    inc.w   Project.GameState.Words.tokensCollected

    lda.w   #0
    jmp     MetaTiles.SetMapCell
}
}

}


namespace InteractiveTests {
namespace InteractiveTilesTest {

// Strong gravity makes the tests faster
constant GRAVITY = 0xFFFF


a16()
i16()
code()
Test.add("Interactive Tiles Test Small")
function Test_Small {
    lda.w   #Project.RoomList.InteractiveTilesTestSmall
    bra     Test
}


a16()
i16()
code()
Test.add("Interactive Tiles Test Large")
function Test_Large {
    lda.w   #Project.RoomList.InteractiveTilesTestLarge
    bra     Test
}


// Tile Collision Test
// INPUT: A = room Id
a16()
i16()
code()
function Test {

// Number of frames remaining in this test
// (sint16)
constant _framesRemaining = Test.dpTmp

constant _previousTime = Test.dpTmp + 2
constant _nRollbacks   = Test.dpTmp + 4


    // A = room Id
    ldx.w   #Project.PlayerIds.InteractiveTilesTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom

    jsr     GameLoop.SpawnAllEntitiesInRoom


    // Setup `MetaTiles.CrumblingTiles.time` overflow test
    // -60 chosen to ensure that there are crumbling tiles in the Queue when time is rolled back
    lda.w   #0x8000 - 60
    sta.w   MetaTiles.CrumblingTiles.time
    sta.b   _previousTime
    stz.b   _nRollbacks


    // Execute test for 5 seconds (on NTSC)
    lda.w   #60 * 5
    sta.b   _framesRemaining


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx


    jsr     Test.PrintCurrentTestName


    jsr     Resources.TransferToPpu

    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    sep     #$20
a8()

    jsr     EnableDisplay_Full


    Text.Console.SetCursor(0, 3)
    Text.Console.PrintConstString("Time:\n")
    Text.Console.PrintConstString("Tokens:\n")
    Text.Console.PrintConstString("Red Y:\n")
    Text.Console.PrintConstString("Blue Y:\n")

    rep     #$30
a16()


    Loop:
        jsr     WaitFrame_PrintFreeCycles

        jsr     GameLoop.ProcessFrame


        Text.Console.SetCursor(8, 3)
        ldy.w   MetaTiles.CrumblingTiles.time
        jsr     Text.Console.PrintU16Y

        Text.Console.SetCursor(8, 4)
        ldy.w   Project.GameState.Words.tokensCollected
        jsr     Text.Console.PrintU16Y

        Text.Console.SetCursor(8, 5)
        ldy.w   Entity.playerEntity + Entities.Players.InteractiveTilesTest_Player.EntityStruct.changeInY
        jsr     Text.Console.PrintU16Y

        Text.Console.SetCursor(8, 6)
        jsr     GetChangeInYForBlueEntity
        jsr     Text.Console.PrintU16Y


        // Test `MetaTiles.CrumblingTiles.time` is never 0x8000 and is rolled back correctly
        lda.w   MetaTiles.CrumblingTiles.time
        bmi     Fail

        cmp.b   _previousTime
        sta.b   _previousTime
        bcs     +
            inc.b   _nRollbacks

            sep     #$10
        i8()
            // Confirm that there is at least one tile in the ChainB (respawning tiles) queue
            // when time was rolled back
            ldx.w   MetaTiles.CrumblingTiles.ChainB.Queue.secondPos
            cpx.w   MetaTiles.CrumblingTiles.ChainB.Queue.firstPos
            beq     Fail
        +
        rep     #$30
    i16()

        dec.b   _framesRemaining
        bpl     Loop


    sep     #$20
a8()
    lda.w   GameState.roomId
    cmp.b   #Project.RoomList.InteractiveTilesTestSmall
    bne     RoomIsLarge
        // Room is Small
        // Test passes if
        //   * more than three rows of tokens are collected
        //   * Two tiles in the crumbling tiles bridge have been restored

        define mapTile(x, y, mapHeight) = MetaTiles.map.data + {y} + {x} * MetaTiles.MAP_HEIGHT_{mapHeight}

        ldy.w   Project.GameState.Words.tokensCollected
        cpy.w   #32 * 3
        bcc     Fail

        lda.w   {mapTile(11, 7, SMALL)}
        cmp.b   #70
        bne     Fail

        lda.w   {mapTile(20, 7, SMALL)}
        cmp.b   #70
        bne     Fail

        bra     EndRoomSizeIf


    RoomIsLarge:
        // Room is Large
        // Test passes if both entities `changeInY` are large enough
        //
        // If this test fails then:
        //  a) a crumbling tile did not crumble, or
        //  b) a non-respawning crumble tile respawned, or
        //  c) the enemy did not interact with the crumbling blocks

        rep     #$30
    a16()
        ldy.w   Entity.playerEntity + Entities.Players.InteractiveTilesTest_Player.EntityStruct.changeInY
        cpy.w   #450
        bcc     Fail

        jsr     GetChangeInYForBlueEntity
        cpy.w   #112
        bcc     Fail


EndRoomSizeIf:
    rep     #$30
a16()

    // Confirm there was only one rollback
    lda.b   _nRollbacks
    cmp.w   #1
    bne     Fail


    // Test passes
    Resources.AnimatedTileset.DisableAnimation()
    Resources.Palette.DisableAnimation()

    jmp     Test.ResetPpuState


Fail:
    clc
    rts
}


// Retrieves the `changeInY` for the InteractiveTilesTest_Entity
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// OUTPUT: Y = blue entity's `changeInY` value
a16()
i16()
code()
function GetChangeInYForBlueEntity {
    lda.w   #2
    Entity.GetEntityWithIdOrBranchIfEmpty_X(Fail)
    ldy.b   Entities.InteractiveTilesTest_Entity.EntityStruct.changeInY,x
    rts

Fail:
    ldy.w   #0
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

