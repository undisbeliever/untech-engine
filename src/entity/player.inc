// entity/player.inc
// =================
//
// Routines/Macros for handling the player entity
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

