// entity/entityidtable.inc
// ========================
//
// Routines for accessing the entityIdTable
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {
namespace _EntityIdTable {

// Test MAX_ENTITY_ID is valid
assertPowerOfTwo(MAX_ENTITY_ID + 2)
assert(MAX_ENTITY_ID & 1 == 0)
assert(MAX_ENTITY_ID < 255)
assert(MAX_ENTITY_ID > N_ENTITIES)


// Initializes the entityIdTable and _unusedEntity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro Init() {
    assert16a()
    assert16i()

    // reset entityIdTable
    lda.w   #_unusedEntity
    ldx.w   #MAX_ENTITY_ID
    -
        sta.w   entityIdTable,x
        dex
        dex
        bpl     -


    // clear _unusedEntity
    ldx.w   #ENTITY_SIZE - 2
    -
        stz.w   _unusedEntity,x
        dex
        dex
        bpl     -

    lda.w   #InvalidEntityFunctionTable
    sta.w   _unusedEntity + BaseEntity.functionTable


    // set currentEntityId to MAX_ENTITY_ID + 2
    // set minimumEntityId to 2
    assert(MAX_ENTITY_ID < 256)
    assert(currentEntityId + 1 == minimumEntityId)
    lda.w   #((MAX_ENTITY_ID + 2) & 0xff) | (2 << 8)
    sta.w   currentEntityId
}



// Assigns a new entityId to the entity.
//
// ASSUMES: the entity does not have an entityId.
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
macro AssignEntityId() {
    assert16a()
    assert8i()

    //  id = currentEntityId
    //
    //  repeat:
    //      id-- // (2 bytes)
    //      if id < minimumEntityId:
    //          id = MAX_ENTITY_ID
    //  until entityIdTable[id] < 0x8000:
    //
    //  entityIdTable[id] = dp
    //  currentEntityId = id

    ldx.w   currentEntityId

    Loop:
        dex
        dex
        cpx.w   minimumEntityId
        bcs     NoWrapping
            ldx.b   #MAX_ENTITY_ID
    NoWrapping:

        assert(_unusedEntity > 0x8000)
        lda.w   entityIdTable,x
        bpl     Loop

    // X = unused entityId
    tdc
    sta.w   entityIdTable,x
    stx.b   BaseEntity.entityId

    stx.w   currentEntityId
}



// Remove an entity from the EntityIdTable
//
// NOTE: Also clears listId
//
// REQUITES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
// BRANCH: InvalidEntityId if the entityId is invalid
macro RemoveEntity() {
    assert16a()
    assert8i()

    ldx.b   BaseEntity.entityId
    beq     SkipRemoveEntity

        tdc
        cmp.w   entityIdTable,x
        bne     InvalidEntityId

        lda.w   #_unusedEntity
        sta.w   entityIdTable,x

        assert(BaseEntity.entityId + 1 == BaseEntity.listId)
        stz.b   BaseEntity.entityId

SkipRemoveEntity:
}

}


// Public API
// ==========


// Get the address of the entity with the given entityId,
// branching to *label* if there is no entity with the
// given entityId.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: A = entityId (only the first n bits are used)
// OUTPUT: Y = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_Y(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_ENTITY_ID + 2)
    assert(Entity.MAX_ENTITY_ID & 1 == 0)

    assert16a()
    assert16i()

    and.w   #Entity.MAX_ENTITY_ID
    tax
    ldy.w   Entity.entityIdTable,x
    bmi     {label}
}


// Get the address of the entity with the given entityId,
// branching to *label* if there is no entity with the
// given entityId.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: A = entityId (only the first n bits are used)
// OUTPUT: X = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_X(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_ENTITY_ID + 2)
    assert(Entity.MAX_ENTITY_ID & 1 == 0)

    assert16a()
    assert16i()

    and.w   #Entity.MAX_ENTITY_ID
    tay
    ldx.w   Entity.entityIdTable,y
    bmi     {label}
}



// Get the address of the entity with the given entityId,
// branching to *label* if there is no entity with the
// given entityId.
//
// REQUIRES: 16 bit Index, DB = 0x7e
// INPUT: A = entityId
// OUTPUT: Y = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_A8Y(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_ENTITY_ID + 2)
    assert(Entity.MAX_ENTITY_ID & 1 == 0)
    assert(Entity.MAX_ENTITY_ID < 255)

    assert8a()
    assert16i()

    and.b   #Entity.MAX_ENTITY_ID
    sep     #$10
i8()
    tax
    rep     #$10
i16()
    ldy.w   Entity.entityIdTable,x
    bmi     {label}
}



// Get the address of the entity with the given entityId,
// branching to *label* if there is no entity with the
// given entityId.
//
// REQUIRES: 16 bit Index, DB = 0x7e
// INPUT: A = entityId
// OUTPUT: X = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_A8X(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_ENTITY_ID + 2)
    assert(Entity.MAX_ENTITY_ID & 1 == 0)
    assert(Entity.MAX_ENTITY_ID < 255)

    assert8a()
    assert16i()

    and.b   #Entity.MAX_ENTITY_ID
    sep     #$10
i8()
    tay
    rep     #$10
i16()
    ldx.w   Entity.entityIdTable,y
    bmi     {label}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

