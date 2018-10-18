// entity/counters.inc
// ===================
//
// Routines for counting entities.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {

// Counts the number of enemies of a specific Id
//
// This routine will also search the deactivated and tryToActivateAgain entity
// lists.
//
// This routine SHOULD ONLY be called on entities that will not change its
// entity type as it does not check which list a deactivated entity belongs to.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = entity Id
// OUTPUT: A = number of matching entities
a16()
i16()
code()
function CountEnemiesWithId {
allocateTmpWord(tmp_count)

    stz.w   tmp_count

    asl
    tax
    lda.l   Entity.Data.EntityList,x

    ldx.w   Entity.lists.enemies
    jsr     _TestList
    ldx.w   Entity.lists.deactivated
    jsr     _TestList
    ldx.w   Entity.lists.tryToActivateAgain
    jsr     _TestList

    lda.w   tmp_count
    rts

    // IN: A = entity romPtr
    //     X = first entity in the list
    //     n = set if list if empty
    function _TestList {
        beq     SkipLoop

        Loop:
            cmp.w   BaseEntity.romPtr,x
            bne     +
                inc.w   tmp_count
            +
            txy
            ldx.w   BaseEntity.next,y
            bne     Loop
    SkipLoop:

        rts
    }
}



// Counts the number of enemy entities active
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// OUTPUT: A = number of entities in enemy list
a16()
i16()
code()
function CountActiveEnemies {
    lda.w   #0
    ldx.w   lists.enemies
    beq     EndLoop

    Loop:
        inc
        txy
        ldx.w   BaseEntity.next,y
        bne     Loop

EndLoop:
    rts
}



// Counts the number of entities in the given list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// PARAM: listName = the entity list name
// OUTPUT: A = number of entities in enemy list
macro CountEntitiesInList(listName) {
    assert16a()
    assert16i()

    lda.w   #Entity.ListId.{listName}
    jsr     Entity.CountEntitiesInList
}



// Counts the number of entities in the given list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = entity list Id
// OUTPUT: A = number of entities in enemy list
a16()
i16()
code()
function CountEntitiesInList {
    cmp.w   #lists.N_LISTS
    bcs     InvalidEntityListId

    asl
    tax

_ListIndexX:
    lda.w   #0
    ldy.w   lists.FIRST,x
    beq     EndLoop

    Loop:
        inc
        tyx
        ldy.w   BaseEntity.next,x
        bne     Loop

EndLoop:
    rts

InvalidEntityListId:
    break(INVALID_ENTITY_LIST_ID)
}


// Counts the number of entities in the given list index.
//
// NOTE: This routine does not test that the list index is valid.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: X = entity list index
// OUTPUT: A = number of entities in enemy list
constant _CountEntitiesInListIndexX = CountEntitiesInList._ListIndexX

}

// vim: ft=bass-65816 ts=4 sw=4 et:

