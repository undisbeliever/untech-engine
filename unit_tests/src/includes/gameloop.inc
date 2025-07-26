// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


    sep     #$30
a8()
i8()
    sta.w   GameState.roomId
    stx.w   GameState.playerDataId
    sty.w   GameState.entranceId

    rep     #$30
a16()
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

    MetaSprite.CollisionBoxes.Begin()
        MetaSprite.CollisionBoxes.TestPlayerHitboxAndHurtboxAgainstEntityList(EnemyProjectiles, Enemies)

        MetaSprite.CollisionBoxes.TestEntityListHitboxAgainstEntityList(PlayerProjectiles, Enemies)
    MetaSprite.CollisionBoxes.End()


    // ::KUDOS NovaSquirrel::
    // ::: For making Nova the Squirrel 2 open source and showing me how to fix a camera/player-sprite desync::

    // The player does not draw MetaSprites in its Process routine, that happens later.
    // ProcessPlayer is invoked outside the MetaSprite render loop to prevent a desync between the player sprites and camera
    Entity.ProcessPlayer()

    jsr     Camera.CenterOnPlayer

    jsl     MetaTiles.Process__far

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

    jsl     Entity.ValidateEntityLists__far


    // Load a new room if roomChangeRequested set.
    // This is a hack, a proper GameLoop should exit the loop before loading the next room.
    // ::TODO cleanup gameloop and rewrite this::
    assert(Room.roomChangeRequested.LOAD_NEW_ROOM_FLAG == 0x80)
    lda.w   Room.roomChangeRequested - 1
    bpl     +
        // Save inidispBuffer, so this routine can be used on tests with the screen disabled.
        lda.w   inidispBuffer
        pha

            // This prevents screen tearing
            jsl     WaitFrame__far

            jsr     Room.LoadRoom

            jsr     Resources.TransferToPpu
            jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far
            jsr     EnableDisplay_Full

        // Restore inidispBuffer
        pla
        sep     #$20
    a8()
        sta.w   inidispBuffer

        rep     #$30
    a16()

    +

    rts
}


}
}


namespace Gravity {
    createEnum(0, 2)
        enum(NORMAL)
        enum(STRONGEST)
        enum(PLAYER)
    endEnum()
}

namespace Project {

// 0.16 fixed point px/frame/frame table
GravityTable:
    dw  10000       // normal gravity
    dw  0xffff      // strongest gravity (interactive tile test requires max gravity)
    dw  1555        // player gravity

}


// vim: ft=bass-65816 ts=4 sw=4 et:

