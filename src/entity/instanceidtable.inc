// entity/instanceidtable.inc
// ==========================
//
// Routines for accessing the instanceIdTable
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {
namespace _InstanceIdTable {

// Test MAX_INSTANCE_ID is valid
assertPowerOfTwo(MAX_INSTANCE_ID + 2)
assert(MAX_INSTANCE_ID & 1 == 0)
assert(MAX_INSTANCE_ID < 255)
assert(MAX_INSTANCE_ID > N_ENTITIES)


// Initializes the instanceIdTable and _unusedEntity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro Init() {
    assert16a()
    assert16i()

    // reset instanceIdTable
    lda.w   #_unusedEntity
    ldx.w   #MAX_INSTANCE_ID
    -
        sta.w   instanceIdTable,x
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


    // set currentInstanceId to MAX_INSTANCE_ID + 2
    // set minimumInstanceId to 2
    assert(MAX_INSTANCE_ID < 256)
    assert(currentInstanceId + 1 == minimumInstanceId)
    lda.w   #((MAX_INSTANCE_ID + 2) & 0xff) | (2 << 8)
    sta.w   currentInstanceId
}



// Assigns a new instanceId to the entity.
//
// ASSUMES: the entity does not have an instanceId.
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
macro AssignInstanceId() {
    assert16a()
    assert8i()

    //  id = currentInstanceId
    //
    //  repeat:
    //      id-- // (2 bytes)
    //      if id < minimumInstanceId:
    //          id = MAX_INSTANCE_ID
    //  until instanceIdTable[id] < 0x8000:
    //
    //  instanceIdTable[id] = dp
    //  currentInstanceId = id

    ldx.w   currentInstanceId

    Loop:
        dex
        dex
        cpx.w   minimumInstanceId
        bcs     NoWrapping
            ldx.b   #MAX_INSTANCE_ID
    NoWrapping:

        assert(_unusedEntity > 0x8000)
        lda.w   instanceIdTable,x
        bpl     Loop

    // X = unused instanceId
    tdc
    sta.w   instanceIdTable,x
    stx.b   BaseEntity.instanceId

    stx.w   currentInstanceId
}



// Remove an entity from the InstanceIdTable
//
// NOTE: Also clears listId
//
// REQUITES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
// BRANCH: InvalidInstanceId if the instanceId is invalid
macro RemoveEntity() {
    assert16a()
    assert8i()

    ldx.b   BaseEntity.instanceId
    beq     SkipRemoveEntity

        tdc
        cmp.w   instanceIdTable,x
        bne     InvalidInstanceId

        lda.w   #_unusedEntity
        sta.w   instanceIdTable,x

        assert(BaseEntity.instanceId + 1 == BaseEntity.listId)
        stz.b   BaseEntity.instanceId

SkipRemoveEntity:
}

}


// Public API
// ==========


// Get the address of the entity with the given instanceId,
// branching to *label* if there is no entity with the
// given instanceId.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: A = instanceId (only the first n bits are used)
// OUTPUT: Y = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_Y(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_INSTANCE_ID + 2)
    assert(Entity.MAX_INSTANCE_ID & 1 == 0)

    assert16a()
    assert16i()

    and.w   #Entity.MAX_INSTANCE_ID
    tax
    ldy.w   Entity.instanceIdTable,x
    bmi     {label}
}


// Get the address of the entity with the given instanceId,
// branching to *label* if there is no entity with the
// given instanceId.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: A = instanceId (only the first n bits are used)
// OUTPUT: X = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_X(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_INSTANCE_ID + 2)
    assert(Entity.MAX_INSTANCE_ID & 1 == 0)

    assert16a()
    assert16i()

    and.w   #Entity.MAX_INSTANCE_ID
    tay
    ldx.w   Entity.instanceIdTable,y
    bmi     {label}
}



// Get the address of the entity with the given instanceId,
// branching to *label* if there is no entity with the
// given instanceId.
//
// REQUIRES: 16 bit Index, DB = 0x7e
// INPUT: A = instanceId
// OUTPUT: Y = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_A8Y(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_INSTANCE_ID + 2)
    assert(Entity.MAX_INSTANCE_ID & 1 == 0)
    assert(Entity.MAX_INSTANCE_ID < 255)

    assert8a()
    assert16i()

    and.b   #Entity.MAX_INSTANCE_ID
    sep     #$10
i8()
    tax
    rep     #$10
i16()
    ldy.w   Entity.instanceIdTable,x
    bmi     {label}
}



// Get the address of the entity with the given instanceId,
// branching to *label* if there is no entity with the
// given instanceId.
//
// REQUIRES: 16 bit Index, DB = 0x7e
// INPUT: A = instanceId
// OUTPUT: X = entity address
// BRANCH: *label* if there is no entity with the id
macro GetEntityWithIdOrBranchIfEmpty_A8X(label) {
    assert(Entity._unusedEntity > 0x8000)
    assertPowerOfTwo(Entity.MAX_INSTANCE_ID + 2)
    assert(Entity.MAX_INSTANCE_ID & 1 == 0)
    assert(Entity.MAX_INSTANCE_ID < 255)

    assert8a()
    assert16i()

    and.b   #Entity.MAX_INSTANCE_ID
    sep     #$10
i8()
    tay
    rep     #$10
i16()
    ldx.w   Entity.instanceIdTable,y
    bmi     {label}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

