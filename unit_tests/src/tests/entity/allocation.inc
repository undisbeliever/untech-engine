// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

scope EntityTest {

a16()
i16()
code()
Test.add("Entity.Init")
scope Init: {
    jsr     Entity.Init

    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.ClearTryToActivateAgainList")
scope ClearTryToActivateAgainList: {
constant tmp(Test.tmp)

    jsr     Entity.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft

    // Create many try to activate again entities

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    assert(Entity.N_ENTITIES >= Entity.lists.N_LISTS)

    lda.w   #Entities.BlankEntity.EntityId_0
    Loop:
        sta.w   tmp

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #Entities.BlankEntity.EntityId_0 + Entity.lists.N_LISTS
        bcc     Loop


    jsr     _TryToActivateAgainListLength
    cmp.w   #Entity.lists.N_LISTS
    bne     Fail


    jsr     Entity.ClearTryToActivateAgainList


    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES
    bne     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

