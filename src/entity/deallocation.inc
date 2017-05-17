// entity/deallocation.inc
// =======================
//
// Routines for de-allocating entire lists of entities.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


scope Entity {

// Deletes all of the entities in the given list
//
// Entities in this list will be destructed and have their resources
// freed.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _DeleteAllInList(listId) {
    assert16a()
    assert16i()

    scope DeleteEntities_{listId}: {
        lda.w   lists.{listId}
        beq     NoneToDelete

        Loop:
            tcd

            MetaSprite.EntityDestructor()

            ldx.b   BaseEntity.functionTable
            jsr     (BaseEntityFunctionTable.Destructor,x)

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
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// MODIFIES: DP
a16()
i16()
code()
scope DeleteDeactivatedEntities: {
    lda.w   lists.deactivated
    beq     NoneToDelete

    Loop:
        tcd

        MetaSprite.EntityDestructor()

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

NoneToDelete:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

