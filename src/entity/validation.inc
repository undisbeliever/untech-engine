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
RegisterBreakId(FREE_ENTITY_ID_NOT_ZERO)
RegisterBreakId(LAST_FREE_ENTITY_CORRUPT)
RegisterBreakId(ENTITYID_INVALID)
RegisterBreakId(ENTITYID_TABLE_INVALID)


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
        cpx.w   #Entity.specialLists.LAST_SPECIAL_LIST_INDEX + 1
        bcc     OuterLoop


    cpy.w   #Entity.N_ENTITIES
    bne     EntityCountInvalid
}



// Tests that an entity's entityId value is correct
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// BRANCH: `EntityIdInvalid` if the entity's entityId is invalid
macro __TestEntity_EntityId() {
    //  if entityId != 0:
    //      if entityId & 1 != 0: goto EntityIdInvalid
    //      if entityId > MAX_ENTITY_ID: goto EntityIdInvalid
    //      if entityIdTable[entityId] != entity: goto EntityIdInvalid

    assert16a()
    assert8i()

    ldx.b   BaseEntity.entityId
    beq     EndTest
        txa
        lsr
        bcs     EntityIdInvalid

        assert(MAX_ENTITY_ID + 1 < 256)
        cpx.b   #MAX_ENTITY_ID + 1
        bcs     EntityIdInvalid

        tdc
        cmp.w   entityIdTable,x
        beq     EndTest

    EndTest:
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
// BRANCH: `EntityIdInvalid` if the entity's entityId is invalid
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


            phx
                sep     #$10
            i8()
                __TestEntity_EntityId()
                rep     #$30
            i16()
            plx


            lda.b   BaseEntity.next
            bne     InnerLoop

    Continue_OuterLoop:
        inx
        inx
        cpx.w   #Entity.N_ENTITY_LISTS * 2
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
// BRANCH: `EntityIdInvalid` if the entity's entityId is invalid
macro __ValidateSpecialEntityLists() {

    assert16a()
    assert16i()


    ldx.w   #specialLists.FIRST_SPECIAL_LIST_INDEX

    OuterLoop:
        lda.w   lists.FIRST,x
        beq     Continue_OuterLoop

        InnerLoop:
            tcd

            // branch to EntityIsNotDeactivated if entity has a MetaSprite palette
            //
            // We do not test vramSet as that flag indicates if an entity has a
            // VRAM slot, not that the tileset has been uploaded successfully.

            lda.b   BaseEntity.MetaSprite.status
            and.w   #MetaSprite.StatusFlags.paletteSet
            bne     EntityIsNotDeactivated


            phx
                sep     #$10
            i8()
                __TestEntity_EntityId()
                rep     #$30
            i16()
            plx


            lda.b   BaseEntity.next
            bne     InnerLoop

    Continue_OuterLoop:
        inx
        inx

        // do not test the free list, that is done separately
        assert(specialLists.LAST_SPECIAL_LIST_INDEX == specialLists.FREE_INDEX)
        cpx.w   #specialLists.LAST_SPECIAL_LIST_INDEX - 1
        bcc     OuterLoop
}



// Tests the free entity list is valid
//
// An entity in the free list is valid if:
//
//  * The entity is deactivated
//  * The entity's function table points to `InvalidEntityFunctionTable`
//
// This macro also tests that `lastFreeEntity` points to the last
// entity in the free list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
//
// BRANCH: `EntityIsNotDeactivated` if an entity was activated
// BRANCH: `FreeEntityFunctionTableInvalid` if the function table was incorrect
// BRANCH: `LastFreeEntityCorrupt` if `lastFreeEntity` is not the last entity in the free list.
// BRANCH: `FreeEntityIdNotZero` if the `BaseEntity.entityId` is not zero
macro __ValidateFreeList() {

    assert16a()
    assert16i()

    lda.w   #0
    tcd

    lda.w   Entity.specialLists.free
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

            // test entityId is zero
            lda.b   BaseEntity.entityId
            and.w   #0xff
            bne     FreeEntityIdNotZero

            lda.b   BaseEntity.next
            bne     Loop
    SkipTest:


    // test lastFreeEntity is correct

    // DP = last entity in free list (or 0 if list is empty)
    tdc
    cmp.w   lastFreeEntity
    bne     LastFreeEntityCorrupt
}



// Validate the entityIdTable
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// BRANCH: `EntityIdTableInvalid` if a value in the entityIdTable is invalid
// BRANCH: `EntityIdInvalid` if an entity's entityId does not match the entityIdTable value
macro __ValidateEntityIdTable() {

    assert16a()
    assert16i()

    // entityId 0 MUST BE unused
    lda.w   entityIdTable
    cmp.w   #_unusedEntity
    bne     EntityIdTableInvalid


    ldx.w   #2
    Loop:
        // X = entityIdTable index

        lda.w   entityIdTable,x
        cmp.w   #_unusedEntity
        beq     UnusedEntity
            tay
            // Y = entity address

            // test entity address is valid
            cmp.w   #entityPool
            bcc     EntityIdTableInvalid
            cmp.w   #entityPool + ENTITY_SIZE * N_ENTITIES
            bcs     EntityIdTableInvalid

            assertPowerOfTwo(ENTITY_SIZE)
            and.w   #(ENTITY_SIZE - 1)
            bne     EntityIdTableInvalid

            // Test BaseEntity.entityId is valid
            txa
            sep     #$20
        a8()
            cmp.w   BaseEntity.entityId,y
            rep     #$30
        a16()
            bne     EntityIdInvalid

    UnusedEntity:
        inx
        inx
        cpx.w   #MAX_ENTITY_ID + 1
        bcc     Loop
}


namespace ValidateEntityLists {

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
//  * Breaks with `FREE_ENTITY_ID_NOT_ZERO` if a free entity's entityId is not
//    zero.
//  * Breaks with `LAST_FREE_ENTITY_CORRUPT` if `lastFreeEntity` does not
//    point to the last entity in the free list.
//  * Breaks with `ENTITYID_INVALID` if an entity's entityId valus is invalid
//  * Breaks with `ENTITYID_TABLE_INVALID` if the entityIdTable is invalid
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

    bra     +
        EntityIsNotDeactivated:
            // DP = entity
            // X = list index
            // A = entity MetaSprite status
            break(ENTITY_IS_NOT_DEACTIVATED)

        FreeEntityFunctionTableInvalid:
            // DP = entity
            // X = expected function table
            break(FREE_ENTITY_FUNC_TABLE_INVALID)

        FreeEntityIdNotZero:
            // DP = entity
            // A = entityId
            break(FREE_ENTITY_ID_NOT_ZERO)

        LastFreeEntityCorrupt:
            break(LAST_FREE_ENTITY_CORRUPT)

        EntityIdTableInvalid:
            // X = entityIdTable index
            // Y = value of table cell
            break(ENTITYID_TABLE_INVALID)

        EntityIdInvalid:
            break(ENTITYID_INVALID)
    +

    __ValidateFreeList()

    __ValidateEntityIdTable()

    lda.w   #0
    tcd

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

