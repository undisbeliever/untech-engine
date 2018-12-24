// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {

a16()
i16()
code()
Test.add("Entity.TemporarilyDeactivateEntities")
function TemporarilyDeactivateEntities {

    jsr     _SetupCounterTest

    // Add one entity in the first list
    lda.w   #Entities.BlankEntity.EntityDataId_0
    jsr     Entity.Spawn

    // Add one entity in the last list
    assert(Entity.N_ENTITY_LISTS == 8)
    lda.w   #Entities.BlankEntity.EntityDataId_7
    jsr     Entity.Spawn

constant N_ACTIVE_ENTITIES = _SetupCounterTest.N_ACTIVE_ENEMIES + 2

    jsr     ProcessEntityGameLoop

    // Ensure last entity list has one entity in it
    lda.w   #Entity.N_ENTITY_LISTS - 1
    jsr     Entity.CountEntitiesInList
    cmp.w   #1
    bne     Fail

    // Ensure activateNextFrame special list is empty before doing test
    jsr     _ActivateNextFrameListLength
    cmp.w   #0
    bne     Fail


    jsr     Entity.TemporarilyDeactivateEntities


    jsr     _ActivateNextFrameListLength
    cmp.w   #N_ACTIVE_ENTITIES
    bne     Fail

    // Test passes
    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entity.PreloadEntityMetaSprites")
function PreloadEntityMetaSprites {
// The test MUST NOT USE Test.tmp variables

    jsr     Entity.Init
    bcc     Fail

    // Spawn entities that have MetaSprite Data
    jsr     InteractiveTests.MetaSpriteInteractiveTest.SpawnEntities
    bcc     Fail

constant N_ENTITIES = InteractiveTests.MetaSpriteInteractiveTest.EntityTable.N_ENTITIES

    // Ensure ActivateNextFrame list length is correct
    jsr     _ActivateNextFrameListLength
    cmp.w   #N_ENTITIES
    bne     Fail


    jsr     EnableDisplay_Full
    lda.w   #4
    Loop:
        pha
            jsr     Entity.PreloadEntityMetaSprites
            jsr     WaitFrame
        pla
        dec
        bne     Loop


    // Ensure All entities are active
    jsr     _ActivateNextFrameListLength
    cmp.w   #0
    bne     Fail

    Entity.CountEntitiesInList(PLAYERS)
    cmp.w   #N_ENTITIES
    bne     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

