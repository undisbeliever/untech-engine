// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// EntityHitbox Interactive Collisions Test.
//
// Displays a single player entity and multiple enemy entities on screen.
// The player entity is moveable with the D-PAD, and entity type can be
// changed with SELECT.
//
// When two entities collide the arguments sent to entity's ProcessCollision
// routine are displayed on screen and a particle is spawned at the collision
// centre.
//
// Tests:
//  * The Entity Loop
//  * MetaSprite EntityHitbox Collisions
//  * Entity.SpawnAtMetaSpriteCollisionCentre

namespace Entities {
namespace Players {

namespace EhCollisionTest_Player {
    buildFunctionTable(BasePlayerFunctionTable, EhCollisionTest_Player)

    constant N_FRAMES = 8

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(selectedFrame, 2)
        endstruct()
    }


    constant DrawPlayer = MetaSprite.Render.RenderEntity


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        lda.w   #0
        sta.b   EntityStruct.selectedFrame

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
    function RoomAboutToChange {
        Text.Console.PrintConstString("Player.RoomAboutToChange\n")
        rts
    }


    // Print the Move the entity's position with D-PAD, change the entity type on SELECT.
    //
    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // INPUT:  A = MetaSprite.Collision.bits
    // INPUT:  A = MetaSprite.Collision.bits
    a16()
    i16()
    code()
    function ProcessCollision {
        jsr     InteractiveTests.EntityHitboxCollisionsTest.PrintProcessCollisionArgs

        // Spawn the CollisionCentre
        SpawnEntityAtMetaSpriteCollisionCentre(EhCollisionTest_CollisionCentre)

        rts
    }


    // Move the entity's position with the D-PAD, change the entity type on SELECT.
    //
    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Process {
        lda.w   Controller.Joy1.current
        bit.w   #JOY.left
        beq     +
            dec.b   EntityStruct.xPos.px
        +
        bit.w   #JOY.right
        beq     +
            inc.b   EntityStruct.xPos.px
        +
        bit.w   #JOY.up
        beq     +
            dec.b   EntityStruct.yPos.px
        +
        bit.w   #JOY.down
        beq     +
            inc.b   EntityStruct.yPos.px
        +

        lda.w   Controller.Joy1.pressed
        and.w   #JOY.select
        beq     EndIf
            lda.b   EntityStruct.selectedFrame
            inc
            cmp.w   #N_FRAMES
            bcc     +
                lda.w   #0
            +
            sta.b   EntityStruct.selectedFrame
            SetFrame_A()

        EndIf:

        rts
    }
}
}



namespace EhCollisionTest_Enemy {
    constant ProcessCollision = InteractiveTests.EntityHitboxCollisionsTest.PrintProcessCollisionArgs

    buildFunctionTable(BaseEntityFunctionTable, EhCollisionTest_Enemy)


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    //         A = frameId
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
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
    function Destructor {
        Text.Console.PrintConstString("E.Destructor\n")
        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Deactivated {
        Text.Console.PrintConstString("E.Deactivated\n")
        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Process {

        jmp     Entity.DrawAndGotoNextEntity
    }
}



// Shows the cursor sprite at the centre of the collision
// Deletes itself on the next frame
namespace EhCollisionTest_CollisionCentre {
    buildFunctionTable(BaseEntityFunctionTable, EhCollisionTest_CollisionCentre)

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(framesLeft, 2)
        endstruct()
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        lda.w   #1
        sta.b   EntityStruct.framesLeft

        SetFrame(frame)

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


    // Moves the entity to the toDelete list after `framesLeft` frames.
    //
    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Process {
        dec.b   EntityStruct.framesLeft
        bpl     +
            jmp     Entity.DeleteAndGotoNextEntity
        +

        jmp     Entity.DrawAndGotoNextEntity
    }
}
}


namespace InteractiveTests {
namespace EntityHitboxCollisionsTest {

constant _tmp_index                   = Test.tmp + 0
constant _collisionBits               = Test.tmp + 2
constant _processCollisionArgumentsOk = Test.tmp + 4

// Prints the ProcessCollision arguments to screen.
//
// Also Tests that A/Y are set correctly, sets `_processCollisionArgumentsOk`
// to zero if they do not match.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// DP = entity
// A = MetaSprite.Collision.bits
// Y = MetaSprite.Collision.otherEntity
a16()
i16()
code()
function PrintProcessCollisionArgs {
constant tmpString = Text.tmpString

    sep     #$20
a8()
    cmp.w   MetaSprite.Collision.bits
    bne     _ArgumentsFail

    rep     #$30
a16()
i16()
    cpy.w   MetaSprite.Collision.otherEntity
    beq     +
_ArgumentsFail:
        rep     #$30
        stz.w   _processCollisionArgumentsOk
    +

    tdc
    jsr     Text.Console.PrintHex16A


    Text.Console.PrintConstString(" ")

    sep     #$20
a8()
    lda.w    MetaSprite.Collision.bits
    sta.w   _collisionBits

    jsr     Text.Console.PrintHex8A


    // Convert _collisionBits to a human readable string
    lda.b   #Text.Font.SPACE
    sta.w   tmpString
    sta.w   tmpString + 8 + 1
    stz.w   tmpString + 8 + 2

    ldx.w   #7
    Loop:
        lsr.w   _collisionBits
        bcc     +
            lda.l   BitsString,x
            bra     ++
        +
            lda.b   #Text.Font.MINUS
        +
        sta.w   tmpString + 1,x
        dex
        bpl     Loop

    lda.b   #tmpString >> 16
    ldx.w   #tmpString
    jsr     Text.Console.PrintString

    ldy.w   MetaSprite.Collision.otherEntity
    jsr     Text.Console.PrintHex16Y

    jsr     Text.Console.NewLine

    rep     #$30
a16()
i16()

    // Corrupt all utDpTmp variables.
    //
    // Tests that the utDpTmp variables used by EntityHitbox subsystem
    // are not used across a call boundary.

    jmp     _DirtyUtDpTmp


rodata(rom0)
BitsString:
    db      "WSABwsab"
}



rodata(rom0)
namespace EnemyTable {
constant Y_DELTA = 20
constant N_COLUMNS = 4

constant Y_BOTTOM = MetaTiles.map.TOP + Camera.DISPLAY_HEIGHT - 30

constant X_LEFT = MetaTiles.map.LEFT + Camera.DISPLAY_WIDTH / 3 + 20
constant X_RIGHT = MetaTiles.map.LEFT + Camera.DISPLAY_WIDTH / 2 + 10
constant X_CENTER = MetaTiles.map.LEFT + Camera.DISPLAY_WIDTH / 2

    struct(pc())
        field(entityDataId, 2)
        field(xPos, 2)
        field(yPos, 2)
        field(parameter, 2)
    endstruct()

evaluate EN_ID = EntityLists.Enemies

constant ENEMY  = Project.EntityIds.EhCollisionTest_Enemy
constant BLANK  = Project.EntityIds.EhCollisionTest_BlankEnemy


Data:
    //  entity, xPos,           yPos,                       frameId

    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0
    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0
    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0

    dw  ENEMY,  X_LEFT,         Y_BOTTOM - Y_DELTA * 2,     MSEO.EhCollisionTest_Enemy.Frames.box
    dw  ENEMY,  X_RIGHT,        Y_BOTTOM - Y_DELTA * 3,     MSEO.EhCollisionTest_Enemy.Frames.row1
    dw  ENEMY,  X_RIGHT,        Y_BOTTOM - Y_DELTA * 2,     MSEO.EhCollisionTest_Enemy.Frames.row2
    dw  ENEMY,  X_RIGHT,        Y_BOTTOM - Y_DELTA * 1,     MSEO.EhCollisionTest_Enemy.Frames.row3
    dw  ENEMY,  X_RIGHT,        Y_BOTTOM - Y_DELTA * 0,     MSEO.EhCollisionTest_Enemy.Frames.row4

    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0
    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0
    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0

    dw  ENEMY,  X_LEFT  + 9,    Y_BOTTOM - Y_DELTA * 2 + 7, MSEO.EhCollisionTest_Enemy.Frames.box
    dw  ENEMY,  X_RIGHT + 9,    Y_BOTTOM - Y_DELTA * 3 + 7, MSEO.EhCollisionTest_Enemy.Frames.row1
    dw  ENEMY,  X_RIGHT + 9,    Y_BOTTOM - Y_DELTA * 2 + 7, MSEO.EhCollisionTest_Enemy.Frames.row2
    dw  ENEMY,  X_RIGHT + 9,    Y_BOTTOM - Y_DELTA * 1 + 7, MSEO.EhCollisionTest_Enemy.Frames.row3
    dw  ENEMY,  X_RIGHT + 9,    Y_BOTTOM - Y_DELTA * 0 + 7, MSEO.EhCollisionTest_Enemy.Frames.row4

    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0
    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0
    dw  BLANK,  X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0

constant N_ENEMIES = (pc() - Data) / size
}



// EntityHitbox Interactive test.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0 DB = 0x7e
a16()
i16()
code()
Test.add("EntityHitbox Collisions Tst")
function Test {

    lda.w   #0xffff
    sta.w   _processCollisionArgumentsOk


    lda.w   #Project.RoomList.EmptyRoom
    ldx.w   #Project.PlayerIds.EhCollisionTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    jsr     Resources.TransferToPpu
    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank


    jsr     EnableDisplay_Full


    // Load the entities

    ldx.w   #0
    SpawnEnemyLoop:
        stx.w   _tmp_index

        lda.l   EnemyTable.xPos,x
        sta.w   Entity.Spawn.xPos

        lda.l   EnemyTable.yPos,x
        sta.w   Entity.Spawn.yPos

        lda.l   EnemyTable.parameter,x
        tay

        lda.l   EnemyTable.entityDataId,x

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   _tmp_index
        clc
        adc.w   #EnemyTable.size
        tax
        cpx.w   #EnemyTable.size * EnemyTable.N_ENEMIES
        bcc     SpawnEnemyLoop


    // Process the Game Loop
    // ---------------------

    Loop:
        jsr     Text.Console.ClearBuffer
        Text.Console.PrintConstString("EntityHitbox Collision Test")
        Text.Console.SetCursor(0, 2)
        Text.Console.PrintConstString("Use D-Pad to move Player\nSelect changes hitbox type")

        // Using hex as it is constant time
        Text.Console.SetCursor(Text.Console.TEXT_WIDTH - 5, 5)
        ldy.w   freeCycles
        jsr     Text.Console.PrintHex16Y

        Text.Console.SetCursor(0, 7)

        jsr     GameLoop.ProcessFrame

        jsr     _DirtyUtDpTmp

        // test ProcessCollision arguments still Ok
        lda.w   _processCollisionArgumentsOk
        beq     Fail

        jsr     WaitFrame

        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        beq     Loop


    jmp     GameLoop.Cleanup

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

