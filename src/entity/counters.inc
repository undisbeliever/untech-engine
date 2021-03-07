// entity/counters.inc
// ===================
//
// Routines for counting entities.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {



// Counts the number of entities in the given list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// PARAM: listName = the entity list name
// OUTPUT: A = number of entities in enemy list
macro CountEntitiesInList(listName) {
    assert16a()
    assert16i()

    ldx.w   #{Entity.listIndex({listName})}
    jsr     Entity.CountEntitiesInList._ListIndexX
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
    cmp.w   #N_ENTITY_LISTS
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

