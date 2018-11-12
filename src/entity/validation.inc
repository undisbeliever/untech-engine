// entity/validation.inc
// =====================
//
// Entity state validation routine.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_ENTITY_COUNT)
RegisterBreakId(INFINITE_LOOP)
RegisterBreakId(ENTITY_IS_NOT_ACTIVATED)
RegisterBreakId(ENTITY_IS_NOT_DEACTIVATED)
RegisterBreakId(FREE_ENTITY_FUNC_TABLE_INVALID)


namespace Entity {

// Validates the number of entities in all the entity lists
// and validates that the lists contain no infinite loop.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
//
// BRANCH: `InfiniteLoopDetected` if an infinite loop was detected
// BRANCH: `EntityCountInvalid` if the entity count was invalid
macro __ValidateEntityLists() {

    assert16a()
    assert16i()

    // Based on Floyd's Tortoise and Hare algorithm
    // I cannot compare registers X to Y so I had to rearrange things a bit.
    //
    //  linkedListCount = 0
    //
    //  for listIndex = 0 .. Entity.lists.LAST_SPECIAL_LIST_INDEX:
    //      hare = tortoise = lists[listIndex]
    //      if hare != null:
    //          linkedListCount++;
    //
    //          while true:
    //              if hare->next == null:
    //                  break loop
    //              hare = hare->next
    //              linkedListCount++;
    //
    //              if hare->next == null:
    //                  break loop
    //              hare = hare->next
    //              linkedListCount++
    //
    //              if hare == tortoise->next:
    //                  break(INFINITE_LOOP)
    //              else:
    //                  tortoise = tortoise->next
    //
    //  if _linkedListCount != Entity.N_ENTITIES:
    //      break(INVALID_ENTITY_STATE)
    //

    ldy.w   #0
    ldx.w   #0

    OuterLoop:
        phx

        lda.w   Entity.lists.FIRST,x
        beq     Continue_OuterLoop
            tcd
            tax

            iny

            InnerLoop:
                // DP = hare
                // X = tortoise
                // Y = linked list count

                lda.b   BaseEntity.next
                beq     Break_InnerLoop
                tcd

                iny


                lda.b   BaseEntity.next
                beq     Break_InnerLoop

                iny


                // A = hare (DP = previous hare position)
                cpx.b   BaseEntity.next
                beq     InfiniteLoopDetected

                tcd
                lda.w   BaseEntity.next,x
                tax
                bra     InnerLoop

        Break_InnerLoop:
    Continue_OuterLoop:
        plx

        inx
        inx
        cpx.w   #Entity.lists._LAST_SPECIAL_LIST_INDEX + 1
        bcc     OuterLoop


    cpy.w   #Entity.N_ENTITIES
    bne     EntityCountInvalid
}


// Tests that the normal entity lists are valid.
//
// An entity in a normal entity list is valid if:
//   * The `listId` is equal to the list tested.
//   * The entity is activated.
//
// NOTE: Only the MetaSprite tileset is tested, this routine does not
//       test if the MetaSprite palette is activated.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
//
// BRANCH: `EntityIsNotActivated` if an entity was not activated
// BRANCH: `InvalidEntityListId` if the entity's `listId` is invalid
macro __ValidateNormalEntityLists() {
allocateTmpWord(_currentListIndex)

    assert16a()
    assert16i()


    ldx.w   #0

    OuterLoop:
        lda.w   lists.FIRST,x
        beq     Continue_OuterLoop

        stx.w   _currentListIndex

        InnerLoop:
            tcd

            // branch to EntityIsNotActivated if entity does not have a VRAM allocated
            lda.b   BaseEntity.MetaSprite.status
            and.w   #MetaSprite.StatusFlags.vramSet
            beq     EntityIsNotActivated


            // test listId == currentList
            lda.b   BaseEntity.listId
            and.w   #0xff
            asl
            cmp.w   _currentListIndex
            bne     InvalidEntityListId


            lda.b   BaseEntity.next
            bne     InnerLoop

    Continue_OuterLoop:
        inx
        inx
        cpx.w   #Entity.lists.N_LISTS * 2
        bcc     OuterLoop
}



// Tests the special entity lists are valid.
//
// An entity in the special list is valid if:
//
//  * It is deactivated
//
// NOTE: This routine does not test the free entity list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
//
// BRANCH: `EntityIsNotDeactivated` if an entity was activated
macro __ValidateSpecialEntityLists() {

    assert16a()
    assert16i()


    ldx.w   #lists._FIRST_SPECIAL_LIST_INDEX

    OuterLoop:
        lda.w   lists.FIRST,x
        beq     Continue_OuterLoop

        InnerLoop:
            tcd

            // branch to EntityIsNotDeactivated if entity has a MetaSprite palette or vram allocated
            lda.b   BaseEntity.MetaSprite.status
            and.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
            bne     EntityIsNotDeactivated


            lda.b   BaseEntity.next
            bne     InnerLoop

    Continue_OuterLoop:
        inx
        inx

        // do not test the free list, that is done separately
        assert(lists._LAST_SPECIAL_LIST_INDEX == lists._FREE_INDEX)
        cpx.w   #lists._LAST_SPECIAL_LIST_INDEX - 1
        bcc     OuterLoop
}



// Tests the free entity list is valid
//
// An entity in the free list is valid if:
//
//  * The entity is deactivated
//  * The entity's function table points to `InvalidEntityFunctionTable`
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
//
// BRANCH: `EntityIsNotDeactivated` if an entity was activated
// BRANCH: `FreeEntityFunctionTableInvalid` if the function table was incorrect
macro __ValidateFreeList() {

    assert16a()
    assert16i()


    lda.w   Entity.lists.FIRST + Entity.lists._FREE_INDEX
    beq     SkipTest

        ldx.w   #InvalidEntityFunctionTable

        Loop:
            tcd

            // branch to EntityIsNotDeactivated if entity has a MetaSprite palette or vram allocated
            lda.b   BaseEntity.MetaSprite.status
            and.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
            bne     EntityIsNotDeactivated

            // X = #InvalidEntityFunctionTable
            cpx.b   BaseEntity.functionTable
            bne     FreeEntityFunctionTableInvalid

            lda.b   BaseEntity.next
            bne     Loop
    SkipTest:
}



// Validate the entity lists, breaking if an entity is invalid.
//
//  * Breaks with `INVALID_ENTITY_COUNT` if the number of entities in
//    the normal and special entity lists is not N_ENTITIES.
//  * Breaks with `INFINITE_LOOP` if a linked list contains an infinite loop.
//  * Breaks with `INVALID_ENTITY_LIST_ID` if the entity's listId is incorrect.
//  * Breaks with `ENTITY_IS_NOT_ACTIVATED` if the entity is in a normal
//    list but does not have an active MetaSprite tileset.
//  * Breaks with `ENTITY_IS_NOT_DEACTIVATED` if the entity is in a
//    special list but has an active MetaSprite palette or tileset.
//  * Breaks with `FREE_ENTITY_FUNC_TABLE_INVALID` if a free entity's
//    functionTable is not InvalidEntityFunctionTable.
//
// This routine should be called intermittently throughout the game.
// If you call it too often you get lag, not enough and you might
// miss a bug.
// Examples of when to call this routine are; at level end, on game over,
// when pausing, or when opening a menu.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
a16()
i16()
code()
function ValidateEntityLists {

    __ValidateEntityLists()

    __ValidateNormalEntityLists()

    __ValidateSpecialEntityLists()

    __ValidateFreeList()

    lda.w   #0
    tcd

    rts


InfiniteLoopDetected:
    break(INFINITE_LOOP)

EntityCountInvalid:
    // Y = entities counted
    break(INVALID_ENTITY_COUNT)

InvalidEntityListId:
    // DP = entity
    // X = list index
    // A = entity listId * 2
    break(INVALID_ENTITY_LIST_ID)

EntityIsNotActivated:
    // DP = entity
    // X = list index
    // A = entity MetaSprite status
    break(ENTITY_IS_NOT_ACTIVATED)

EntityIsNotDeactivated:
    // DP = entity
    // X = list index
    // A = entity MetaSprite status
    break(ENTITY_IS_NOT_DEACTIVATED)

FreeEntityFunctionTableInvalid:
    // DP = entity
    // X = expected function table
    break(FREE_ENTITY_FUNC_TABLE_INVALID)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

