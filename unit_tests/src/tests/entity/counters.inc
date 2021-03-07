// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {

a16()
i16()
code()
function _SetupCounterTest {
    // Spawns 8 entities.
    //    * 2 onscreen DeactivateIfOutsideEntity
    //    * 1 onscreen DeleteAfterDelayEntity
    //    * 2 offscreen DeactivateIfOutsideEntity
    //    * 3 offscreen DeleteAfterDelayEntity

    constant N_ONSCREEN_DIO_ENTITIES = 2
    constant N_ONSCREEN_DAD_ENTITIES = 1

    constant N_OFFSCREEN_DIO_ENTITIES = 2
    constant N_OFFSCREEN_DAD_ENTITIES = 3

    constant N_DIO_ENTITIES = N_ONSCREEN_DIO_ENTITIES + N_OFFSCREEN_DIO_ENTITIES
    constant N_DAD_ENTITIES = N_ONSCREEN_DAD_ENTITIES + N_OFFSCREEN_DAD_ENTITIES


    constant N_ACTIVE_ENTITIES = N_ONSCREEN_DIO_ENTITIES + N_DAD_ENTITIES

    define ENTITY_LIST_ID = EntityList_9

    constant DIO_ENTITY_DATA_ID = Project.EntityIds.DeactivateIfOutside_9
    constant DAD_ENTITY_DATA_ID = Project.EntityIds.DeleteAfterDelay_9

    jsr     Entity.Init
    bcc     Fail

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos
    sta.w   Camera.xPos
    sta.w   Camera.yPos

    lda.w   #DIO_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DAD_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DIO_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail



    lda.w   #0x8000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    lda.w   #DIO_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DIO_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DAD_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DAD_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    lda.w   #DAD_ENTITY_DATA_ID
    jsr     Entity.Spawn
    bcc     Fail

    jsr     ProcessEntityGameLoop

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Entity.CountEntitiesInList (macro)")
function CountEntitiesInList_Macro {
    jsr     _SetupCounterTest
    bcc     Fail

    Entity.CountEntitiesInList({_SetupCounterTest.ENTITY_LIST_ID})
    cmp.w   #_SetupCounterTest.N_ACTIVE_ENTITIES
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
Test.add("Entity.CountEntitiesInList (active)")
function CountEntitiesInList_Active {
    jsr     _SetupCounterTest
    bcc     Fail

    Entity.CountEntitiesInList({_SetupCounterTest.ENTITY_LIST_ID})
    cmp.w   #_SetupCounterTest.N_ACTIVE_ENTITIES
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
Test.add("Entity.CountEntitiesInList (empty)")
function CountEntitiesInList_Empty {
    jsr     _SetupCounterTest
    bcc     Fail

    // An empty list
    Entity.CountEntitiesInList(EntityList_4)
    cmp.w   #0
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

