// entity/instanceidtable.inc
// ==========================
//
// Routines for accessing the instanceIdTable
//
//
// SPDX-FileCopyrightText: © 2019 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2019 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


Warnings.Register(DUPLICATE_INSTANCE_ID, 16, 8, 31)

namespace Entity {
namespace _InstanceIdTable {

// Test MAX_INSTANCE_ID is valid
assertPowerOfTwo(MAX_INSTANCE_ID + 2)
assert(MAX_INSTANCE_ID & 1 == 0)
assert(MAX_INSTANCE_ID < 255)
assert(MAX_INSTANCE_ID / INSTANCE_ID_INCREMENT - 1 > N_ENTITIES)


// Initializes the instanceIdTable and _unusedEntity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro _Init() {
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
    lda.w   #((MAX_INSTANCE_ID + 2) & 0xff) | (FIRST_INSTANCE_ID << 8)
    sta.w   currentInstanceId
}



// Sets the number of room entities
//
// Populates `minimumInstanceId`.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = number of room entities
macro _SetNumberOfRoomEntities() {
    assert16a()
    assert16i()

    // instanceId 0 is always unused
    assert(Entity.FIRST_INSTANCE_ID == 1 * Entity.INSTANCE_ID_INCREMENT)
    inc

    assert(Entity.INSTANCE_ID_INCREMENT == 2)
    asl
    cmp.w   #Entity.MAX_INSTANCE_ID - 1
    bcc     +
        break(INVALID_ROOM_DATA)
    +

    sep     #$20
a8()
    sta.w   Entity.minimumInstanceId

    rep     #$20
a16()
}



// Assigns a new instanceId to the entity.
//
// ASSUMES: the entity does not have an instanceId.
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
macro _AssignNewInstanceId() {
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

        assert(_unusedEntity & 0xffff > 0x8000)
        lda.w   instanceIdTable,x
        bpl     Loop

    // X = unused instanceId
    tdc
    sta.w   instanceIdTable,x
    stx.b   BaseEntity.instanceId

    stx.w   currentInstanceId
}



// Set a given instanceId to the entity
//
// Breaks if the instanceId is already assigned
//
// ASSUMES: the entity does not have an instanceId.
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: X = entityId (must not be 0)
macro _SetInstanceId() {
    assert16a()
    assert8i()

    //  if instanceIdTable[id] >= 0x8000:
    //      Warnings.Warn(DUPLICATE_INSTANCE_ID)
    //      id = 0
    //  else:
    //      instanceIdTable[id] = dp
    //
    //  currentInstanceId = id

    assert(Entity._unusedEntity & 0xffff > 0x8000)
    lda.w   Entity.instanceIdTable,x
    bmi     InstanceIdUnused
        Warnings.Warn(DUPLICATE_INSTANCE_ID)

        ldx.b   #0
        bra     EndIf

    InstanceIdUnused:
        tdc
        sta.w   Entity.instanceIdTable,x

EndIf:
    stx.b   BaseEntity.instanceId
}



// Remove an entity from the InstanceIdTable
//
// NOTE: Also clears listId
//
// REQUITES: 16 bit A, 8 bit Index, DB = 0x7e
// INPUT: dp = entity
// BRANCH: InvalidInstanceId if the instanceId is invalid
macro _RemoveEntity() {
    assert16a()
    assert8i()

    ldx.b   BaseEntity.instanceId
    beq     SkipRemoveEntity

        tdc
        cmp.w   instanceIdTable,x
        bne     InvalidInstanceId

        lda.w   #_unusedEntity
        sta.w   instanceIdTable,x

        // test if safe to do a 16 bit write
        assert(BaseEntity.instanceId + 1 == BaseEntity.projectileId)
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
    assert(Entity._unusedEntity & 0xffff > 0x8000)
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
    assert(Entity._unusedEntity & 0xffff > 0x8000)
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
    assert(Entity._unusedEntity & 0xffff > 0x8000)
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
    assert(Entity._unusedEntity & 0xffff > 0x8000)
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

