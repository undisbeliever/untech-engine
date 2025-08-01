// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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
Test.add("Entity.Spawn")
function Spawn {
    jsr     Entity.Init

    stz.w   Entities.Blank.parameterSum

    lda.w   #0x1234
    sta.w   Entity.Spawn.xPos
    lda.w   #0x5678
    sta.w   Entity.Spawn.yPos

    lda.w   #Project.EntityIds.Blank_0
    ldy.w   #0x1337
    jsr     Entity.Spawn
    bcc     Fail

    // test x/y pos
    lda.w   BaseEntity.xPos.px,y
    cmp.w   #0x1234
    bne     Fail

    lda.w   BaseEntity.yPos.px,y
    cmp.w   #0x5678
    bne     Fail

    lda.w   BaseEntity.spawnXPos,y
    cmp.w   #0x1234
    bne     Fail

    lda.w   BaseEntity.spawnYPos,y
    cmp.w   #0x5678
    bne     Fail

    // test parameter worked
    lda.w   Entities.Blank.parameterSum
    cmp.w   #0x1337
    bne     Fail

    // test list decreased by one
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - 1
    bne     Fail

    // Must process the game loop to move the new entity into the entity list
    jsr     ProcessEntityGameLoop

    // Test first list length
    Entity.CountEntitiesInList(EntityList_0)
    cmp.w   #1
    bne     Fail

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Entity.Spawn (Overflow)")
function Spawn_Overflow {
constant tmp = Test.tmp

    jsr     Entity.Init

    stz.w   Entities.Blank.parameterSum

    ldx.w   #Entity.N_ENTITIES

    Loop:
        stx.w   tmp

        lda.w   #Project.EntityIds.Blank_0
        ldy.w   #5
        jsr     Entity.Spawn
        bcc     Fail

        ldx.w   tmp
        dex
        bne     Loop


    jsr     _TestEntityListsValid
    bcc     Fail

    // Test cannot make new entity
    lda.w   #Project.EntityIds.Blank_0
    jsr     Entity.Spawn
    bcs     Fail

    // Test ENTITY_POOL_EMPTY warning was emitted
    lda.w   Warnings.current
    cmp.w   #__WARNINGS__.ids.ENTITY_POOL_EMPTY
    bne     Fail
    stz.w   Warnings.current

    // Must process the game loop to move the new entities into their entity lists
    jsr     ProcessEntityGameLoop

    // Test free list is 0
    jsr     _FreeListLength
    bne     Fail

    // Test parameter Sum is correct value
    lda.w   Entities.Blank.parameterSum
    cmp.w   #5 * Entity.N_ENTITIES
    bne     Fail

    // Test first list length
    Entity.CountEntitiesInList(EntityList_0)
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
Test.add("Entity.Spawn (Multiple Lists)")
function Spawn_MultipleLists {
constant tmp = Test.tmp

constant FIRST_ENTITY = Project.EntityIds.Blank_0
constant LAST_ENTITY = Project.EntityIds.Blank_9
constant N_ENTITIES_SPAWNED = (LAST_ENTITY - FIRST_ENTITY + 1) * 2

    jsr     Entity.Init

    stz.w   Entities.Blank.parameterSum

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    // spawn each entity twice
    lda.w   #FIRST_ENTITY
    Loop:
        sta.w   tmp

        ldy.w   #1
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #1
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #LAST_ENTITY + 1
        bcc     Loop


    // Test parameter Sum is correct value
    lda.w   Entities.Blank.parameterSum
    cmp.w   #N_ENTITIES_SPAWNED
    bne     Fail


    // Must process the game loop to move the new entities into their entity lists
    jsr     ProcessEntityGameLoop

    // test list decreased by N_LISTS
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - N_ENTITIES_SPAWNED
    bne     Fail

    // Test each list length
    lda.w   #EntityLists.EntityList_0
    ListLoop:
        pha
            jsr     Entity.CountEntitiesInList
        ply

        cmp.w   #2
        bne     Fail

        tya
        inc
        cmp.w   #EntityLists.EntityList_9 + 1
        bcc     ListLoop

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}


// For the first {N_LISTS_TESTED} lists, spawn
//   * 2 onscreen Blank
//   * 2 onscreen DeactivateIfOutside
//   * 4 offscreen Blank
//   * 4 offscreen DeactivateIfOutside
a16()
i16()
code()
Test.add("Entity.Spawn (Offscreen)")
function Spawn_Offscreen {

constant tmp_listId = Test.tmp

constant ONSCREEN = 0x1080

constant ACTIVE_PER_LIST = 8
constant DEACTIVATED_PER_LIST = 4

// There are not enough entities to do this test on all the entity lists.
// Therefore we are only preforming this test on as many lists as we can while
// still leaving some free.
evaluate N_LISTS_TESTED = (Entity.N_ENTITIES - 2) / (ACTIVE_PER_LIST + DEACTIVATED_PER_LIST)
assert({N_LISTS_TESTED} > 3)
if {N_LISTS_TESTED} > Entity.N_ENTITY_LISTS {
    evaluate N_LISTS_TESTED = Entity.N_ENTITY_LISTS
}

    jsr     Entity.Init

    lda.w   #{N_LISTS_TESTED} - 1
    sta.w   tmp_listId
    Loop:
        macro spawn(xPos, yPos) {
            ldx.w   #{xPos}
            ldy.w   #{yPos}
            jsr     SpawnEntities_XY
            bcc     Fail
        }

        spawn(ONSCREEN, ONSCREEN)
        spawn(ONSCREEN + 0x200, ONSCREEN)
        spawn(ONSCREEN, ONSCREEN + 0x200)
        spawn(ONSCREEN, ONSCREEN)
        spawn(ONSCREEN - 0x200, ONSCREEN)
        spawn(ONSCREEN, ONSCREEN - 0x200)

        dec.w   tmp_listId
        bpl     Loop

    // Must process the game loop to move the new entities into their entity lists
    jsr     ProcessEntityGameLoop

    // Verify list sizes
    jsr     _FreeListLength
    cmp.w   #ExpectedSizes.FREE
    bne     Fail

    jsr     _DeactivatedListLength
    cmp.w   #ExpectedSizes.DEACTIVATED
    bne     Fail


    ldx.w   #ExpectedSizes
    jsr     _TestListLengthTable
    bcc     Fail


    jmp     _TestEntityListsValid

Fail:
    clc
    rts

function SpawnEntities_XY {
    stx.w   Entity.Spawn.xPos
    sty.w   Entity.Spawn.yPos

    lda.w   tmp_listId
    clc
    adc.w   #Project.EntityIds.Blank_0
    jsr     Entity.Spawn
    bcc     Return

    lda.w   tmp_listId
    clc
    adc.w   #Project.EntityIds.DeactivateIfOutside_0
    jsr     Entity.Spawn

Return:
    rts
}

rodata(rom0)
ExpectedSizes:
namespace ExpectedSizes {
    evaluate n = 0
    while {n} < Entity.N_ENTITY_LISTS {
        if {n} >= EntityLists.EntityList_0 && {n} - EntityLists.EntityList_0 < {N_LISTS_TESTED} {
            dw  ACTIVE_PER_LIST
        } else {
            dw  0
        }
        evaluate n = {n} + 1
    }

    constant ACTIVE = ACTIVE_PER_LIST * {N_LISTS_TESTED}
    constant DEACTIVATED = DEACTIVATED_PER_LIST * {N_LISTS_TESTED}

    constant FREE = Entity.N_ENTITIES - ACTIVE - DEACTIVATED
    assert(FREE > 0)

    constant size = pc() - ExpectedSizes
    assert(size / 2 == Entity.N_ENTITY_LISTS)
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

