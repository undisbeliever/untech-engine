// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {

// Process the entity lists and special lists
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function ProcessEntityGameLoop {
    lda.w   #0
    tcd

    Entity.ProcessSpecialLists()
    Entity.ProcessEntityLists()

    jmp     Entity.ValidateEntityLists
}

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// OUT: A = number of entities in free list
a16()
i16()
code()
function _FreeListLength {
    ldx.w   #Entity.specialLists.FREE_INDEX
    jmp     Entity._CountEntitiesInListIndexX
}

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// OUT: A = number of entities in deactivated list
a16()
i16()
code()
function _DeactivatedListLength {
    ldx.w   #Entity.specialLists.DEACTIVATED_INDEX
    jmp     Entity._CountEntitiesInListIndexX
}

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// OUT: A = number of entities in the activateNextFrame list
a16()
i16()
code()
function _ActivateNextFrameListLength {
    ldx.w   #Entity.specialLists.ACTIVATE_NEXT_FRAME_INDEX
    jmp     Entity._CountEntitiesInListIndexX
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// OUT: A = number of entities in player list
a16()
i16()
code()
function _FirstListLength {
    ldx.w   #0
    jmp     Entity._CountEntitiesInListIndexX
}



// Tests that:
//  * deactivated lists have no metasprite data
//  * active lists have metasprite data
//  * total number of entities in all lists == Entity.N_ENTITIES
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
//
// OUT: C set if entity list OK
a16()
i16()
code()
function _TestEntityListsValid {
allocateTmpWord(tmp_index)
allocateTmpWord(tmp_count)

    stz.w   tmp_count

    // test that entities in the given special list are not active
    macro _TestSpecialListsDeactivated(list) {
        namespace {list} {
            lda.w   Entity.specialLists.{list}
            beq     Skip

            Loop:
                tcd

                inc.w   tmp_count

                lda.b   BaseEntity.MetaSprite.status
                and.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
                bne     Fail

                lda.b   BaseEntity.next
                bne     Loop

            Skip:
        }
    }

    _TestSpecialListsDeactivated(deactivated)
    _TestSpecialListsDeactivated(activateNextFrame)


    // test all other entities are active
    function ActiveListTest {
        ldx.w   #Entity.lists.FIRST
        Loop:
            stx.w   tmp_index

            lda.w   0,x
            beq     Skip
            EntityLoop:
                tcd

                inc.w   tmp_count

                lda.b   BaseEntity.MetaSprite.status
                and.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
                cmp.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
                bne     Fail

                lda.b   BaseEntity.next
                bne     EntityLoop
            Skip:

            ldx.w   tmp_index
            inx
            inx
            cpx.w   #Entity.lists.LAST + 1
            bcc     Loop
    }


    // restore DP
    lda.w   #0
    tcd

    jsr     _FreeListLength
    clc
    adc.w   tmp_count
    cmp.w   #Entity.N_ENTITIES
    bne     Fail

    jsr     Entity.ValidateEntityLists

    sec
    rts

Fail:
    clc
    rts
}


// Tests the entity list lengths against a
// list table.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
//
// IN: X - address of list table (list of words, in rom0 data block)
// OUT: C set if entity list == table
a16()
i16()
code()
function _TestListLengthTable {
allocateTmpWord(tmp_index)
allocateTmpWord(tmp_listId)

    lda.w   #0
    ListSizeLoop:
        stx.w   tmp_index
        sta.w   tmp_listId

        jsr     Entity.CountEntitiesInList

        ldx.w   tmp_index
        cmp.l   Table,x
        bne     Fail

        inx
        inx

        lda.w   tmp_listId
        inc
        cmp.w   #Entity.N_ENTITY_LISTS
        bcc     ListSizeLoop

    sec
    rts

Fail:
    clc
    rts

rodata(rom0)
    constant Table = pc() & 0xff0000
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

