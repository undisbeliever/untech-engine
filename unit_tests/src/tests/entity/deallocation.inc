// SPDX-FileCopyrightText: © 2018 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2018 Marcus Rowe <undisbeliever@gmail.com>
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

namespace EntityTest {


a16()
i16()
code()
Test.add("Entities.DeleteEntitiesInActivateNextFrameList")
function DeleteEntitiesInActivateNextFrameList {
constant tmp = Test.tmp

constant FIRST_ENTITY_ID = Project.EntityIds.Blank_0
constant LAST_ENTITY_ID = Project.EntityIds.Blank_9

constant N_ENTITIES_SPAWNED = LAST_ENTITY_ID - FIRST_ENTITY_ID + 1
assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS)

    jsr     Entity.Init

    // Prevent the entities from spawning
    jsl     WaitFrame__far
    stz.w   Dma.transfersLeft


    // Spawn many entities (none of which would be activated)

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    lda.w   #FIRST_ENTITY_ID
    Loop:
        sta.w   tmp

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #LAST_ENTITY_ID + 1
        bcc     Loop


    jsr     _ActivateNextFrameListLength
    cmp.w   #N_ENTITIES_SPAWNED
    bne     Fail


    jsr     Entity.DeleteEntitiesInActivateNextFrameList


    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES
    bne     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.DeleteDeactivatedEntities")
function DeleteDeactivatedEntities {
constant N_ENTITIES_TO_SPAWN = 15
constant counter = Test.tmp

assert(N_ENTITIES_TO_SPAWN < Entity.N_ENTITIES + 2)


    jsr     Entity.Init

    // Spawn N_ENTITIES_TO_SPAWN DeactivateIfOutsideEntity entities offscreen

    lda.w   #0x8000
    sta.w   Camera.xPos
    sta.w   Camera.yPos

    // update inactive window
    jsr     ProcessEntityGameLoop


    lda.w   #N_ENTITIES_TO_SPAWN
    sta.w   counter


    Loop:
        lda.w   #0x1000
        sta.w   Entity.Spawn.xPos
        sta.w   Entity.Spawn.yPos

        lda.w   #Project.EntityIds.DeactivateIfOutside_0
        jsr     Entity.Spawn
        bcc     Fail

        dec.w   counter
        bne     Loop


    jsr     _DeactivatedListLength
    cmp.w   #N_ENTITIES_TO_SPAWN
    bne     Fail


    jsr     Entity.DeleteDeactivatedEntities


    jsr     _DeactivatedListLength
    cmp.w   #0
    bne     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.DeleteEntitiesInList")
function DeleteEntitiesInList {
constant tmp = Test.tmp

constant ENTITIES_PER_LIST = 3
constant TEST_LIST_ID = EntityLists.EntityList_2

assert(Entity.N_ENTITY_LISTS * ENTITIES_PER_LIST < Entity.N_ENTITIES + 2)
assert(Entity.N_ENTITY_LISTS > 2)

constant FIRST_ENTITY_ID = Project.EntityIds.Blank_0
constant LAST_ENTITY_ID = Project.EntityIds.Blank_9

assert(LAST_ENTITY_ID - FIRST_ENTITY_ID + 1 == 10)
assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS)

    jsr     Entity.Init

    // Spawn 3 entities per list

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    lda.w   #FIRST_ENTITY_ID
    Loop:
        sta.w   tmp

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        jsr     Entity.Spawn
        bcc     Fail

        assert(ENTITIES_PER_LIST == 3)

        lda.w   tmp
        inc
        cmp.w   #LAST_ENTITY_ID + 1
        bcc     Loop


    jsr     ProcessEntityGameLoop


    ldx.w   #ListLengthTable_Initial
    jsr     _TestListLengthTable
    bcc     Fail

    lda.w   #TEST_LIST_ID
    jsr     Entity.DeleteEntitiesInList

    ldx.w   #ListLengthTable_AfterDelete
    jsr     _TestListLengthTable
    bcc     Fail

    // test calling routine on empty list works
    lda.w   #TEST_LIST_ID
    jsr     Entity.DeleteEntitiesInList

    ldx.w   #ListLengthTable_AfterDelete
    jsr     _TestListLengthTable
    bcc     Fail

    jmp     _TestEntityListsValid


Fail:
    clc
    rts


rodata(rom0)
ListLengthTable_Initial:
    evaluate n = 0
    while {n} < EntityLists.EntityList_0 {
        dw  0
        evaluate n = {n} + 1
    }
    while {n} < Entity.N_ENTITY_LISTS {
        dw  ENTITIES_PER_LIST
        evaluate n = {n} + 1
    }

rodata(rom0)
ListLengthTable_AfterDelete:
    evaluate n = 0
    while {n} < EntityLists.EntityList_0 {
        dw  0
        evaluate n = {n} + 1
    }
    while {n} < Entity.N_ENTITY_LISTS {
        dw {n} != TEST_LIST_ID ? ENTITIES_PER_LIST : 0
        evaluate n = {n} + 1
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

