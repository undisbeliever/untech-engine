// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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
//  * Spawning Room Entities
//  * Room Entity Parameters
//  * Not Spawning Room Entity Groups
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

constant _collisionBits               = Test.tmp + 0
constant _processCollisionArgumentsOk = Test.tmp + 2

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


    lda.w   #Project.RoomList.EntityHitboxCollisionsTest
    ldx.w   #Project.PlayerIds.EhCollisionTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    // Only spawn entity groups 2 & 1.
    // Do not spawn entity group 0, 1, 3, or 4.
    lda.w   #0b001100
    tsb.w   Room.EntityGroups.toActivateBitfield

    jsr     Room.EntityGroups.SpawnEntityGroups


    jsr     Resources.TransferToPpu
    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank


    jsr     EnableDisplay_Full


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

