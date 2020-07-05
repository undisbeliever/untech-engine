// entity/validation.inc
// =====================
//
// Entity state validation routine.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_ENTITY_COUNT)
RegisterBreakId(INFINITE_LOOP)
RegisterBreakId(ENTITY_IS_NOT_ACTIVATED)
RegisterBreakId(ENTITY_IS_NOT_DEACTIVATED)
RegisterBreakId(FREE_ENTITY_FUNC_TABLE_INVALID)
RegisterBreakId(FREE_INSTANCE_ID_NOT_ZERO)
RegisterBreakId(LAST_FREE_ENTITY_CORRUPT)
RegisterBreakId(PLAYER_IS_INVALID)
RegisterBreakId(INSTANCE_ID_INVALID)
RegisterBreakId(INSTANCE_ID_TABLE_INVALID)


namespace Entity {


// Validate the player
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// BRANCH: `PlayerIsInvalid` if the player is invalid
macro __ValidatePlayer() {
    assert16a()
    assert16i()

    sep     #$20
a8()
    // Player is not part of a linked list, its next field must be NULL
    ldx.w   playerEntity + BaseEntity.next
    bne     PlayerIsInvalid

    // Player must not have an instanceId
    lda.w   playerEntity + BaseEntity.instanceId
    bne     PlayerIsInvalid

    // Player's listId must be 0xff
    lda.w   playerEntity + BaseEntity.listId
    cmp.b   #0xff
    bne     PlayerIsInvalid


    // The player's MetaSprite state is not checked.
    // The room might not be loaded, so the player might not have an active MetaSprite.
    // (The entity unit tests do not load a room or spawn a player)


    rep     #$30
a16()
}



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



// Tests that an entity's instanceId value is correct
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// BRANCH: `InstanceIdInvalid` if the entity's instanceId is invalid
macro __TestEntity_InstanceId() {
    //  if instanceId != 0:
    //      if instanceId & 1 != 0: goto InstanceIdInvalid
    //      if instanceId > MAX_INSTANCE_ID: goto InstanceIdInvalid
    //      if instanceIdTable[instanceId] != entity: goto InstanceIdInvalid

    assert16a()
    assert8i()

    ldx.b   BaseEntity.instanceId
    beq     EndTest
        txa
        lsr
        bcs     InstanceIdInvalid

        assert(MAX_INSTANCE_ID + 1 < 256)
        cpx.b   #MAX_INSTANCE_ID + 1
        bcs     InstanceIdInvalid

        tdc
        cmp.w   instanceIdTable,x
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
// BRANCH: `InstanceIdInvalid` if the entity's instanceId is invalid
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
                __TestEntity_InstanceId()
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
// BRANCH: `InstanceIdInvalid` if the entity's instanceId is invalid
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
                __TestEntity_InstanceId()
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
// BRANCH: `FreeInstanceIdNotZero` if the `BaseEntity.instanceId` is not zero
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

            // test instanceId is zero
            lda.b   BaseEntity.instanceId
            and.w   #0xff
            bne     FreeInstanceIdNotZero

            lda.b   BaseEntity.next
            bne     Loop
    SkipTest:


    // test lastFreeEntity is correct

    // DP = last entity in free list (or 0 if list is empty)
    tdc
    cmp.w   lastFreeEntity
    bne     LastFreeEntityCorrupt
}



// Validate the instanceIdTable
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// BRANCH: `InstanceIdTableInvalid` if a value in the instanceIdTable is invalid
// BRANCH: `InstanceIdInvalid` if an entity's instanceId does not match the instanceIdTable value
// BRANCH: `InvalidMinimimInstaceId` if InvalidMinimimInstaceId is invalid
macro __ValidateInstanceIdTable() {
    assert16a()
    assert16i()

    // The minimumInstanceId must be >= Entity.FIRST_INSTANCE_ID
    sep     #$20
a8()
    lda.w   minimumInstanceId
    cmp.b   #Entity.FIRST_INSTANCE_ID
    bcc     InvalidMinimimInstaceId

    rep     #$30
a16()


    // Show correct X value on Break BSOD if the first entry is invalid
    ldx.w   #0

    // instanceId 0 MUST BE unused
    ldy.w   instanceIdTable
    cpy.w   #_unusedEntity
    bne     InstanceIdTableInvalid

    inx
    inx

    Loop:
        // X = instanceIdTable index

        ldy.w   instanceIdTable,x
        cpy.w   #_unusedEntity
        beq     UnusedEntity
            // Y = entity address

            // test entity address is valid
            cpy.w   #entityPool
            bcc     InstanceIdTableInvalid
            cpy.w   #entityPool + ENTITY_SIZE * N_ENTITIES
            bcs     InstanceIdTableInvalid

            assertPowerOfTwo(ENTITY_SIZE)
            tya
            and.w   #(ENTITY_SIZE - 1)
            bne     InstanceIdTableInvalid

            // Test BaseEntity.instanceId is valid
            txa
            sep     #$20
        a8()
            cmp.w   BaseEntity.instanceId,y
            rep     #$30
        a16()
            bne     InstanceIdInvalid

    UnusedEntity:
        inx
        inx
        cpx.w   #MAX_INSTANCE_ID + 1
        bcc     Loop
}



namespace ValidateEntityLists {

PlayerIsInvalid:
    break(PLAYER_IS_INVALID)

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
//  * Breaks with `FREE_INSTANCE_ID_NOT_ZERO` if a free entity's instanceId is not
//    zero.
//  * Breaks with `LAST_FREE_ENTITY_CORRUPT` if `lastFreeEntity` does not
//    point to the last entity in the free list.
//  * Breaks with `INSTANCE_ID_INVALID` if an entity's instanceId valus is invalid
//  * Breaks with `INSTANCE_ID_TABLE_INVALID` if the instanceIdTable is invalid
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

    __ValidatePlayer()

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

        FreeInstanceIdNotZero:
            // DP = entity
            // A = instanceId
            break(FREE_INSTANCE_ID_NOT_ZERO)

        LastFreeEntityCorrupt:
            break(LAST_FREE_ENTITY_CORRUPT)

        InstanceIdTableInvalid:
            // X = instanceIdTable index
            // Y = value of table cell
            break(INSTANCE_ID_TABLE_INVALID)

        InstanceIdInvalid:
            break(INSTANCE_ID_INVALID)

        InvalidMinimimInstaceId:
            break(INSTANCE_ID_INVALID)
    +

    __ValidateFreeList()

    __ValidateInstanceIdTable()

    lda.w   #0
    tcd

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

