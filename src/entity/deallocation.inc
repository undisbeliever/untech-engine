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


    sep     #$10
i8()
    _EntityIdTable.RemoveEntity()
    rep     #$30
i16()
a16()

    if !{defined NO_CLEAR_ENTITY_MEMORY} {
        // Clear entity memory with 0xdead when deleted

        assert(BaseEntity.next == 0)
        assert(BaseEntity.functionTable == 2)
        assert(ENTITY_SIZE % 2 == 0)

        lda.w   #0xdead
        evaluate n = 4
        while {n} < ENTITY_SIZE {
            if {n} == BaseEntity.entityId & ~1 {
                // do nothing, cleared by _EntityIdTable.RemoveEntity
            } else if {n} == BaseEntity.MetaSprite.status & ~1 {
                // do nothing, cleared by MetaSprite.EntityDestructor
            } else {
                sta.b   {n}
            }
            evaluate n = {n} + 2
        }
    }

    rts

InvalidEntityId:
    break(ENTITYID_INVALID)
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



// Delete all entities outside the MetaTile map.
//
// MUST NOT BE CALLED in the Entity GameLoop
//
// NOTE: This routine will also delete entities in the special lists.
// NOTE: uses utDpTmp variables.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function DeleteEntitiesOutsideMetaTileMap {

    lda.w   MetaTiles.map.left
    sta.b   DeleteEntitiesOutsideRect.left

    lda.w   MetaTiles.map.right
    sta.b   DeleteEntitiesOutsideRect.right

    lda.w   MetaTiles.map.top
    sta.b   DeleteEntitiesOutsideRect.top

    lda.w   MetaTiles.map.bottom
    sta.b   DeleteEntitiesOutsideRect.bottom

FallThrough:
}



// Delete all entities outside a given rectangle.
//
// MUST NOT BE CALLED in the Entity GameLoop
//
// NOTE: This routine will also delete entities in the special lists.
// NOTE: uses utDpTmp variables.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: left, right, top, bottom = rectangle to test.
//                                   Entities outside this rect will be deleted.
//                                   (4x uint16 dp)
a16()
i16()
code()
function DeleteEntitiesOutsideRect {
constant left       = utDpTmp0
constant right      = utDpTmp1
constant top        = utDpTmp2
constant bottom     = utDpTmp3

constant _previousEntity  = Entity._previousEntity
constant _currentEntityList = Entity._currentEntityList

    assert(pc() == DeleteEntitiesOutsideMetaTileMap.FallThrough)

    ldx.w   #lists.FIRST
    OuterLoop:
        stx.w   _currentEntityList
        stx.w   _previousEntity

        lda.w   0,x
        beq     ListEmpty

        InnerLoop:
            tcd

            lda.b   BaseEntity.xPos.px
            cmp.w   left
            bcc     DeleteEntity
            cmp.w   right
            bcs     DeleteEntity

            lda.b   BaseEntity.yPos.px
            cmp.w   top
            bcc     DeleteEntity
            cmp.w   bottom
            bcs     DeleteEntity

                // Entity is inside the rect
                tdc
                sta.w   _previousEntity

                lda.b   BaseEntity.next
                bne     InnerLoop
                bra     EndInnerLoop


            DeleteEntity:
                // Entity is outside the rect
                jsr     __EntityDestructor

                __MoveEntityToSpecialList(free)
                // MUST NOT set _previousEntity after freeing an entity
                bne     InnerLoop

    EndInnerLoop:
        ldx.w   _currentEntityList
    ListEmpty:
        inx
        inx
        assert(specialLists.ACTIVATE_NEXT_FRAME_INDEX + 2 == specialLists.LAST_SPECIAL_LIST_INDEX)
        cpx.w   #specialLists.activateNextFrame
        bcc     OuterLoop

    lda.w   #0
    tdc

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

