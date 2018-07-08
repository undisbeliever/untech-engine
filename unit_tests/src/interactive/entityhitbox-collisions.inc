// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {

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
//  * EntityLoop
//  * MetaSprite EntityHitbox Collisions
namespace EntityHitboxCollisionsTest {

constant _waitCount                   = Test.tmp
constant _tmp_index                   = Test.tmp + 2
constant _collisionBits               = Test.tmp + 4
constant _processCollisionArgumentsOk = Test.tmp + 6


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

    // Corrupt all dpTmp variables.
    //
    // Tests that the dpTmp variables used by EntityHitbox subsystem
    // are not used across a call boundary.

    jmp     dirtyDpTmp


rodata(rom0)
BitsString:
    db      "WSABwsab"
}



namespace PlayerEntity {
    buildFunctionTable(BaseEntityFunctionTable, PlayerEntity)
    Entity.useDefaultRenderer()

    constant N_FRAMES = 8

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(selectedFrame, 2)
        endstruct()
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Init {
        stz.b   EntityStruct.selectedFrame

        lda.w   #0
        jmp     MetaSprite.SetFrame
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Destructor {
        Text.Console.PrintConstString("Player.Destructor\n")
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
        jsr     PrintProcessCollisionArgs

        // Spawn the CollisionCentreEntity
        lda.w   MetaSprite.Collision.xPos
        sta.w   Entity.Spawn.xPos

        lda.w   MetaSprite.Collision.yPos
        sta.w   Entity.Spawn.yPos

        lda.w   #CollisionCentreEntity.ENTITY_ID
        jmp     Entity.Spawn
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
            jsr     MetaSprite.SetFrame

        EndIf:

        rts
    }


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  Entity.ListId.PLAYER        // initial List
        dw  MSFS.EhCollisionTest_Player // frameSetId
        db  0                           // defaultPalette

    rodata(EN_EntityList)
    constant ENTITY_ID = (pc() - Entity.Data.EntityList) / 2
        dw  RomData
}



namespace EnemyEntity {
    buildFunctionTable(BaseEntityFunctionTable, EnemyEntity)
    Entity.useDefaultRenderer()

    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    //         A = frameId
    a16()
    i16()
    code()
    function Init {
        jmp     MetaSprite.SetFrame
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
    function Process {
        rts
    }


    constant ProcessCollision = PrintProcessCollisionArgs


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  Entity.ListId.ENEMY         // initial List
        dw  MSFS.EhCollisionTest_Enemy  // frameSetId
        db  0                           // defaultPalette

    rodata(EN_EntityList)
    constant ENTITY_ID = (pc() - Entity.Data.EntityList) / 2
        dw  RomData
}



// Shows the cursor sprite at the centre of the collision
// Deletes itself on the next frame
namespace CollisionCentreEntity {
    buildFunctionTable(BaseEntityFunctionTable, CollisionCentreEntity)
    Entity.useDefaultRenderer()

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(framesLeft, 2)
        endstruct()
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Init {
        lda.w   #1
        sta.b   EntityStruct.framesLeft

        lda.w   #0
        jmp     MetaSprite.SetFrame
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    Destructor:
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
            lda.w   #Entity.lists.toDelete
            sta.w   Entity.changeList
        +
        rts
    }

    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  Entity.ListId.PARTICLE      // initial List
        dw  MSFS.EhCollisionTest_Centre // frameSetId
        db  0                           // defaultPalette

    rodata(EN_EntityList)
    constant ENTITY_ID = (pc() - Entity.Data.EntityList) / 2
        dw  RomData
}



rodata(rom0)
namespace EnemyTable {
constant Y_DELTA = 20
constant N_COLUMNS = 4
constant Y_BOTTOM = Camera.ORIGIN + Camera.DISPLAY_HEIGHT - 30

constant X_LEFT = Camera.ORIGIN + Camera.DISPLAY_WIDTH / 3 + 20
constant X_RIGHT = Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2 + 10
constant X_CENTER = Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2

    struct(pc())
        field(entityId, 2)
        field(xPos, 2)
        field(yPos, 2)
        field(parameter, 2)
    endstruct()

evaluate EN_ID = Entity.ListId.ENEMY

constant PLAYER = PlayerEntity.ENTITY_ID
constant ENEMY  = EnemyEntity.ENTITY_ID
constant BLANK  = EntityTest.Entities.BlankEntity.EntityId_{EN_ID}


Data:
    //  entity, xPos,           yPos,                       frameId

    dw  PLAYER, X_CENTER,       Y_BOTTOM - Y_DELTA * 2,     0

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


    jsr     EntityLoop.Init

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

        lda.l   EnemyTable.entityId,x

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   _tmp_index
        clc
        adc.w   #EnemyTable.size
        tax
        cpx.w   #EnemyTable.size * EnemyTable.N_ENEMIES
        bcc     SpawnEnemyLoop


    // Process the Entity Loop
    // -----------------------

    GameLoop:
        jsr     Text.Console.ClearBuffer
        Text.Console.PrintConstString("EntityHitbox Collision Test")
        Text.Console.SetCursor(0, 2)
        Text.Console.PrintConstString("Use D-Pad to move Player\nSelect changes hitbox type")

        // Using hex as it is constant time
        Text.Console.SetCursor(Text.Console.TEXT_WIDTH - 5, 5)
        ldy.w   _waitCount
        jsr     Text.Console.PrintHex16Y

        Text.Console.SetCursor(0, 7)

        jsr     EntityLoop.ProcessFrame

        jsr     dirtyDpTmp

        // test ProcessCollision arguments still Ok
        lda.w   _processCollisionArgumentsOk
        beq     Fail

        jsr     WaitFrame_Count
        sty.w   _waitCount

        lda.l   JOY1
        and.w   #JOY.buttons
        beq     GameLoop


    jmp     EntityLoop.Cleanup

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

