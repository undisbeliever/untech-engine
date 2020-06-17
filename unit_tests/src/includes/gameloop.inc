// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace GameLoop {


// Initialize the Game Loop and load a room
//
// REQUIRES: 16 bit A, 16 bit Index, Autojoy enabled, DB = 0x7e
//
// INPUT: A = roomId
//        X = playerEntityDataId
//        Y = entranceId
a16()
i16()
code()
function Init_LoadRoom {
    sep     #$10
i8()
    stx.w   Room.playerEntityDataId
    sty.w   Room.entranceId

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


// Spawn all entities in the room
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
a16()
i16()
code()
function SpawnAllEntitiesInRoom {
    lda.w   #0xff
    tsb.w   Room.EntityGroups.toActivateBitfield

    jsr     Room.EntityGroups.SpawnEntityGroups

    ldx.w   {Entity.listAddress(Players)}
    beq     +
        // Assumes only one player
        jsr     Camera.CenterOnEntity_X
    +
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


// Draw the first player entity on the screen
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
macro _DrawPlayer() {
    assert16a()
    assert16i()

    // Assumes only one player
    // Assumes player has no custom drawing routine
    lda.w   {Entity.listAddress(Players)}
    beq     Skip{#}
        tcd
        jsr     MetaSprite.Render.RenderEntity

        lda.w   #0
        tcd
Skip{#}:
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
        MetaSprite.EntityHitbox.TestCollisions(PlayerProjectiles,   Enemies)
        MetaSprite.EntityHitbox.TestCollisions(Players,             EnemyProjectiles, Enemies)
    MetaSprite.EntityHitbox.End()


    // ::KUDOS NovaSquirrel::
    // ::: For making Nova the Squirrel 2 open source and showing me how to fix a camera/player-sprite desync::

    // Player entities do not draw MetaSprites in their Process routine, that happens later.
    Entity.ProcessEntityList(Players)

    // Update camera based on player's position
    // This is done outside the render loop to prevent a desync between the player sprites and camera
    ldx.w   {Entity.listAddress(Players)}
    beq     +
        // Assumes only one player
        jsr     Camera.CenterOnEntity_X
    +


    // Update MetaTiles (if room is loaded in memory)
    jsr     MetaTiles.Render.Update
    jsr     Resources.ProcessAnimations


    jsr     MetaSprite.Render.InitLoop

        Entity.ProcessEntityList(Particles)
        Entity.ProcessEntityList(PlayerProjectiles)
        Entity.ProcessEntityList(EnemyProjectiles)

        _DrawPlayer()

        Entity.ProcessEntityList(Enemies)

        evaluate n = 0
        while {n} < 10 {
            Entity.ProcessEntityList(EntityList_{n})
            evaluate n = {n} + 1
        }

    jsr     MetaSprite.Render.EndLoop

    jsr     Entity.ValidateEntityLists

    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

