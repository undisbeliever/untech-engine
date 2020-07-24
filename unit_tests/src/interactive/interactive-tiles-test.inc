// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

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

    constant DrawPlayer = MetaSprite.Render.RenderEntity

    constant X_MOMENTUM_PX = 3


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Init {
        SetFrame(frame)

        sep     #$20
    a8()
        lda.w   Room.roomId
        cmp.b   #Project.RoomList.InteractiveTilesTestSmall
        bne     +
            lda.b   #X_MOMENTUM_PX
            sta.b   BaseEntity.xMomentum.px
        +

        rep     #$30
    a16()
        SetFrame_A()

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

        // Change movement direction on X axis collision
        sep     #$20
    a8()
        assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
        lda.b   BaseEntity.movementState
        bpl     +
            eor.b   #Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG
            sta.b   BaseEntity.movementState

            lda.b   #X_MOMENTUM_PX
            sta.b   BaseEntity.xMomentum.px
        +

        rep     #$30
    a16()

        rts
    }
}
}

namespace InteractiveTilesTest_Entity {
    buildFunctionTable(BaseEntityFunctionTable, InteractiveTilesTest_Entity)

    constant DrawPlayer = MetaSprite.Render.RenderEntity


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: carry set = always activate entity
    a16()
    i16()
    code()
    function Init {
        SetFrame(frame)

        // Always activate entity
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
        MoveEntityWithTileCollisionsAndInteractiveTiles()

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
// Number of token tiles collected by the player
// (uint16)
allocateTmpWord(tokensCollected)

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
    inc.w   tokensCollected

    lda.w   #0
    tyx
    jmp     MetaTiles.SetMapCell
}
}

}


namespace InteractiveTests {
namespace InteractiveTilesTest {

constant GRAVITY = 0x1c80


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

    // A = room Id
    ldx.w   #Project.PlayerIds.InteractiveTilesTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom

    jsr     GameLoop.SpawnAllEntitiesInRoom


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx

    stz.w   InteractiveTiles.Token.tokensCollected


    jsr     Test.PrintCurrentTestName


    jsr     Resources.TransferToPpu

    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    sep     #$20
a8()

    jsr     EnableDisplay_Full


    Text.Console.SetCursor(0, 3)
    Text.Console.PrintConstString("Tokens: ")


    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        jsr     GameLoop.ProcessFrame


        sep     #$20
    a8()
        Text.Console.SetCursor(8, 3)

        ldy.w   InteractiveTiles.Token.tokensCollected
        jsr     Text.Console.PrintU16Y

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


Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

