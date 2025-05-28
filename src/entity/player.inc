// entity/player.inc
// =================
//
// Routines/Macros for handling the player entity
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Entity {
namespace Player {


// Initialize the player entity
macro _Init() {
    assert16a()
    assert16i()


    ldx.w   #InvalidEntityFunctionTable
    stx.w   Entity.playerEntity + BaseEntity.functionTable


    stz.w   Entity.playerEntity + BaseEntity.next
    stz.w   Entity.playerEntity + BaseEntity.instanceId
    stz.w   Entity.playerEntity + BaseEntity.MetaSprite.status

    // Set listId to 0xff
    assert(BaseEntity.listId + 1 == BaseEntity.MetaSprite.frameSet)
    lda.w   #0xff
    sta.w   Entity.playerEntity + BaseEntity.listId


    // Reset entity position (ensure entity is always inside map bounds)
    lda.w   #MetaTiles.map.LEFT
    sta.w   Entity.Player.xPos.px
    assert(MetaTiles.map.LEFT == MetaTiles.map.TOP)
    sta.w   Entity.Player.yPos.px
}

}


// Public API
// ==========


// Calls the player's Process method.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro ProcessPlayer() {
    assert16a()
    assert16i()


    lda.w   #Entity.playerEntity
    tcd

    ldx.b   BaseEntity.functionTable
    jsr     (BasePlayerFunctionTable.Process,x)

    lda.w   #0
    tcd
}



// Calls the player's DrawPlayer method.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: MetaSprite
macro DrawPlayer() {
    assert16a()
    assert16i()


    lda.w   #Entity.playerEntity
    tcd

    ldx.b   BaseEntity.functionTable
    jsr     (BasePlayerFunctionTable.DrawPlayer,x)

    lda.w   #0
    tcd
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

