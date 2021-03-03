// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace GameLoop {


// Initialize the Game Loop and load a room
//
// REQUIRES: 16 bit A, 16 bit Index, Autojoy enabled, DB = 0x7e
//
// INPUT: A = roomId
//        X = playerDataId
//        Y = entranceId
a16()
i16()
code()
function Init_LoadRoom {
    // Always reset game state on each test
    pha
    phx
    phy

    jsr     GameState.LoadInitialGameState

    ply
    plx
    pla


    sep     #$10
i8()
    stx.w   GameState.playerDataId
    sty.w   GameState.entranceId

    rep     #$30
i16()

    // LoadRoom also resets Entity and Camera modules
    jmp     Room.LoadRoom
}



// Cleans up the screen
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
//
// OUTPUT: c set
a16()
i16()
code()
function Cleanup {
    jsr     Test.ResetPpuState

    sec
    rts
}



// Process the Game Loop until a joypad button has been pressed
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
//
// OUTPUT: c set
a16()
i16()
code()
function ProcessUntilButtonPress {
    Loop:
        jsr     ProcessFrame

        jsr     WaitFrame_PrintFreeCycles

        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        beq     Loop

    jmp     Cleanup
}


// Processes a single frame of the Game Loop.
//
// This routine will also validate the entity lists at the end of the
// game loop.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, Init called
//
// OUTPUT: z clear if a button has been pressed
a16()
i16()
code()
function ProcessFrame {

    Entity.ProcessSpecialLists()

    MetaSprite.EntityHitbox.Begin()
        MetaSprite.EntityHitbox.TestForCollisionsWithPlayer(EnemyProjectiles, Enemies)
        MetaSprite.EntityHitbox.TestCollisions(PlayerProjectiles,   Enemies)
    MetaSprite.EntityHitbox.End()


    // ::KUDOS NovaSquirrel::
    // ::: For making Nova the Squirrel 2 open source and showing me how to fix a camera/player-sprite desync::

    // The player does not draw MetaSprites in its Process routine, that happens later.
    // ProcessPlayer is invoked outside the MetaSprite render loop to prevent a desync between the player sprites and camera
    Entity.ProcessPlayer()

    jsr     Camera.CenterOnPlayer

    jsr     MetaTiles.Process

    jsr     Resources.ProcessAnimations


    jsr     MetaSprite.Render.InitLoop

        Entity.ProcessEntityList(Particles)
        Entity.ProcessEntityList(PlayerProjectiles)
        Entity.ProcessEntityList(EnemyProjectiles)

        Entity.DrawPlayer()

        Entity.ProcessEntityList(Enemies)

        evaluate n = 0
        while {n} < 10 {
            Entity.ProcessEntityList(EntityList_{n})
            evaluate n = {n} + 1
        }

    jsr     MetaSprite.Render.EndLoop

    jsr     Room.Process

    jsr     Scripting.ProcessScripts

    jsr     Entity.ValidateEntityLists

    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

