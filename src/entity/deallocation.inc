// entity/deallocation.inc
// =======================
//
// Routines for de-allocating entire lists of entities.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {


// Entity Destructor
//
// Frees the entity's MetaSprites, calls the entity Destructor routine.
//
// if `NO_CLEAR_ENTITY_MEMORY` is not defined the system will override the
// memory used by the entity to `0xdeaddeaddead...`.
//
// NOTE: This routine does not move the entity to the free list.
// That is the responsibility of the calling code.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
function __EntityDestructor {
    MetaSprite.EntityDestructor()

    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.Destructor,x)


    lda.w   #InvalidEntityFunctionTable
    sta.b   BaseEntity.functionTable


    if !{defined NO_CLEAR_ENTITY_MEMORY} {
        // Clear entity memory with 0xdead when deleted

        assert(BaseEntity.next == 0)
        assert(BaseEntity.functionTable == 2)
        assert(ENTITY_SIZE % 2 == 0)

        lda.w   #0xdead
        evaluate n = 4
        while {n} < ENTITY_SIZE {
            if {n} != BaseEntity.MetaSprite.status & ~1 {
                sta.b   {n}
            }
            evaluate n = {n} + 2
        }
    }

    rts
}



// Delete entities in the activateNextFrame entity list.
//
// Should be called at the start of each level.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function DeleteEntitiesInActivateNextFrameList {
    ldx.w   #specialLists.ACTIVATE_NEXT_FRAME_INDEX
    bra     DeleteEntitiesInList._ListIndexX
}



// Deletes all of the entities in the deactivated list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function DeleteDeactivatedEntities {
    ldx.w   #specialLists.DEACTIVATED_INDEX
    bra     DeleteEntitiesInList._ListIndexX
}



// Deletes all entities in the given list
//
// MUST NOT be called inside an entity routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = entity list Id
a16()
i16()
code()
function DeleteEntitiesInList {
    cmp.w   #N_ENTITY_LISTS
    bcs     InvalidEntityListId

    asl
    tax

_ListIndexX:
    lda.w   lists.FIRST,x
    beq     NoneToDelete

    phx

        Loop:
            tcd

            jsr     __EntityDestructor

            lda.b   BaseEntity.next
            bne     Loop

    plx


    // Move the entity list to the end of the free list

    // lastFreeEntity->next = lists[listId].first
    // lastFreeEntity = lastEntity
    // lists[listId] = NULL

    // X = list Index
    // DP = last entity in list

    lda.w   lists.FIRST,x
    ldy.w   lastFreeEntity
    sta.w   BaseEntity.next,y

    tdc
    sta.w   lastFreeEntity

    stz.w   lists.FIRST,x


    lda.w   #0
    tcd


NoneToDelete:
    rts


InvalidEntityListId:
    break(INVALID_ENTITY_LIST_ID)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

