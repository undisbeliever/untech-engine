// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace EntityLoop {

// Initialize the Entity Loop
//
// REQUIRES: 16 bit A, 16 bit Index, Autojoy enabled
// MODIFIES: DB = 0x7e
a16()
i16()
code()
function Init {
    jsr     Entity.Init

    rts
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


// Process the Entity Loop until a joypad button has been pressed
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


// Processes a single frame of the Entity Loop.
//
// This routine will also validate the entity lists after
// the entity loop has ended.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
//
// OUTPUT: z clear if a button has been pressed
a16()
i16()
code()
function ProcessFrame {
    jsr     MetaSprite.Render.InitLoop

    Entity.ProcessSpecialLists()

    MetaSprite.EntityHitbox.Begin()
        MetaSprite.EntityHitbox.TestCollisions(PlayerProjectiles,   Enemies)
        MetaSprite.EntityHitbox.TestCollisions(Players,             EnemyProjectiles, Enemies)
    MetaSprite.EntityHitbox.End()

    Entity.ProcessEntityList(Particles)
    Entity.ProcessEntityList(PlayerProjectiles)
    Entity.ProcessEntityList(EnemyProjectiles)
    Entity.ProcessEntityList(Platforms)
    Entity.ProcessEntityList(Players)
    Entity.ProcessEntityList(Enemies)

    jsr     MetaSprite.Render.EndLoop

    jsr     Entity.ValidateEntityLists

    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

