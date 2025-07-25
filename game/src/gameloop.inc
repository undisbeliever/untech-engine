// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
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


constant DEFAULT_ROOM_GRAVITY = 0x1c80


// Loads a room and plays it
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
//
// INPUT: A = roomId
// INPUT: Y = entranceId
au()
iu()
code()
function GameLoop {

RoomChangeRequested:
    rep     #$30
a16()
i16()

    jsr     Room.LoadRoom


    // Set gravity
    lda.w   #DEFAULT_ROOM_GRAVITY
    sta.w   Room.gravity_sx


    jsr     Resources.TransferToPpu
    jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far


    jsr     EnableDisplay_Full

    Loop:
        jsl     WaitFrame__far


        assert(Room.roomChangeRequested.LOAD_NEW_ROOM_FLAG == 0x80)
        bit.w   Room.roomChangeRequested - 1
        bmi     RoomChangeRequested


        sep     #$30
    a8()
    i8()
        InputBuffer.Process()

        rep     #$30
    a16()
    i16()


        Entity.ProcessSpecialLists()

        MetaSprite.CollisionBoxes.Begin()
            MetaSprite.CollisionBoxes.TestPlayerHitboxAndHurtboxAgainstEntityList(enemyProjectiles, enemies)

            MetaSprite.CollisionBoxes.TestEntityListHitboxAgainstEntityList(playerProjectiles, enemies)
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
            Entity.ProcessEntityList(particles)
            Entity.ProcessEntityList(playerProjectiles)
            Entity.ProcessEntityList(enemyProjectiles)

            Entity.DrawPlayer()

            Entity.ProcessEntityList(enemies)
        jsr     MetaSprite.Render.EndLoop

        jmp     Loop
}

