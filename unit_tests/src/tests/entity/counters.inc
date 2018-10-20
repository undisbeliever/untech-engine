// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {

a16()
i16()
code()
function _SetupCounterTest {
    // Spawns 3 enemies on-screen, 5 off-screen
    // Four of the entities are BlankEntity

    constant N_ACTIVE_ENEMIES = 3
    constant N_BLANK_ENTITIES = 4
    constant N_DEACTIVATED_ENTITIES = 5

    assert(Entity.ListId.ENEMY == 2)
    constant BLANK_ENTITY_ID = Entities.BlankEntity.EntityId_2
    constant DELETE_DELAY_ENTITY_ID = Entities.DeleteAfterDelayEntity.EntityId_2

    jsr     Entity.Init
    bcc     Fail

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos
    sta.w   Camera.xPos
    sta.w   Camera.yPos

    lda.w   #BLANK_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DELETE_DELAY_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #BLANK_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail



    lda.w   #0x8000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    lda.w   #BLANK_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #BLANK_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DELETE_DELAY_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DELETE_DELAY_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DELETE_DELAY_ENTITY_ID
    jsr     Entity.Spawn
    bcc     Fail

    jsr     Entity.ProcessGameLoop

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Entity.CountEnemiesWithId")
function CountEnemiesWithId {
    jsr     _SetupCounterTest
    bcc     Fail

    lda.w   #_SetupCounterTest.BLANK_ENTITY_ID
    jsr     Entity.CountEnemiesWithId
    cmp.w   #_SetupCounterTest.N_BLANK_ENTITIES
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
Test.add("Entity.CountActiveEnemies")
function CountActiveEnemies {
    jsr     _SetupCounterTest
    bcc     Fail

    jsr     Entity.CountActiveEnemies
    cmp.w   #_SetupCounterTest.N_ACTIVE_ENEMIES
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
Test.add("Entity.CountEntitiesInList")
function CountEntitiesInList {
    jsr     _SetupCounterTest
    bcc     Fail

    lda.w   #Entity.ListId.ENEMY
    jsr     Entity.CountEntitiesInList
    cmp.w   #_SetupCounterTest.N_ACTIVE_ENEMIES
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

