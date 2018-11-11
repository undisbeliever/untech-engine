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


    if !{defined NO_CLEAR_ENTITY_MEMORY} {
        // Clear entity memory with 0xdead when deleted

        assert(BaseEntity.next == 0)
        assert(ENTITY_SIZE % 2 == 0)

        lda.w   #0xdead
        evaluate n = 2
        while {n} < ENTITY_SIZE {
            if {n} != BaseEntity.MetaSprite.status & ~1 {
                sta.b   {n}
            }
            evaluate n = {n} + 2
        }
    }

    rts
}

// Deletes all of the entities in the given list
//
// Entities in this list will be destructed and have their resources
// freed.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
inline _DeleteAllInList(listId) {
    assert16a()
    assert16i()

    function DeleteEntities_{listId} {
        lda.w   lists.{listId}
        beq     NoneToDelete

        Loop:
            tcd

            jsr     __EntityDestructor

            lda.b   BaseEntity.next
            bne     Loop


        // Move the list into the free list

        // {listId}.last->next = lists.free
        // lists.free = lists.{listId}
        // lists.{listId} = NULL

        lda.w   lists.free
        sta.b   BaseEntity.next

        lda.w   lists.{listId}
        sta.w   lists.free

        stz.w   lists.{listId}

    NoneToDelete:
    }
}



// Deletes all of the entities in the deactivated list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function DeleteDeactivatedEntities {
    lda.w   lists.deactivated
    beq     NoneToDelete

    Loop:
        tcd

        jsr     __EntityDestructor

        // BaseEntity.functionPtr is invalid, retrieve it from romPtr

        ldx.b   BaseEntity.romPtr
        lda.l   BaseEntityData.functionTable,x
        tax
        jsr     (BaseEntityFunctionTable.Destructor,x)

        lda.b   BaseEntity.next
        bne     Loop


    // Move the deactivated list into the free list

    lda.w   lists.free
    sta.b   BaseEntity.next

    lda.w   lists.deactivated
    sta.w   lists.free

    stz.w   lists.deactivated


    lda.w   #0
    tcd

NoneToDelete:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

