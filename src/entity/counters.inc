// entity/counters.inc
// ===================
//
// Routines for counting entities.
//
//
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

