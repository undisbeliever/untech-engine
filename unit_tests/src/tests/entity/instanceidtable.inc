// SPDX-FileCopyrightText: © 2018 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2018 Marcus Rowe <undisbeliever@gmail.com>
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

namespace EntityTest {

a16()
i16()
code()
Test.add("Entity.GetEntityWithId...")
function GetEntityWithId {
constant oldCurrent = Test.tmp
constant entityAddr = Test.tmp + 2

    jsr     Entity.Init

    lda.w   Entity.currentInstanceId
    sta.w   oldCurrent

    lda.w   #Project.EntityIds.Blank_0
    jsr     Entity.Spawn
    bcc     Fail

    sty.w   entityAddr

    // set high bits of A
    lda.w   #0xffff

    sep     #$20
a8()

    // Test currentInstanceId changed
    lda.w   Entity.currentInstanceId
    cmp.w   oldCurrent
    beq     Fail

    rep     #$30
a16()

    // Test GetEntityWithIdOrBranchIfEmpty_Y returns this entity
    // Y = entityAddr
    lda.w   BaseEntity.instanceId,y
    Entity.GetEntityWithIdOrBranchIfEmpty_Y(Fail)
    cpy.w   entityAddr
    bne     Fail

    // Test GetEntityWithIdOrBranchIfEmpty_X returns this entity
    ldy.w   entityAddr
    lda.w   BaseEntity.instanceId,y
    Entity.GetEntityWithIdOrBranchIfEmpty_X(Fail)
    cpx.w   entityAddr
    bne     Fail


    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entity.GetEntityWithId_A8")
function GetEntityWithId_A8 {
constant oldCurrent = Test.tmp
constant entityAddr = Test.tmp + 2

    jsr     Entity.Init

    lda.w   Entity.currentInstanceId
    sta.w   oldCurrent

    lda.w   #Project.EntityIds.Blank_0
    jsr     Entity.Spawn
    bcc     Fail

    sty.w   entityAddr

    // set high bits of A
    lda.w   #0xffff

    sep     #$20
a8()

    // Test currentInstanceId changed
    lda.w   Entity.currentInstanceId
    cmp.w   oldCurrent
    beq     Fail

    // Test GetEntityWithIdOrBranchIfEmpty_A8Y returns this entity
    // Y = entityAddr
    lda.w   BaseEntity.instanceId,y
    Entity.GetEntityWithIdOrBranchIfEmpty_A8Y(Fail)
    cpy.w   entityAddr
    bne     Fail

    // Test GetEntityWithIdOrBranchIfEmpty_A8X returns this entity
    ldy.w   entityAddr
    lda.w   BaseEntity.instanceId,y
    Entity.GetEntityWithIdOrBranchIfEmpty_A8X(Fail)
    cpx.w   entityAddr
    bne     Fail

    rep     #$30
a16()
    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



// Spawn a few hundred entities, testing that the
//   * instanceId is valid
//   * instanceIdTable is valid
//   * The Entity.ValidateEntityLists does not raise an error
a16()
i16()
code()
Test.add("Entity.InstanceId loop")
function InstanceIdLoopTest {
constant outerLoopCounter = Test.tmp
constant spawnLoopCounter = Test.tmp + 2
constant entityAddr       = Test.tmp + 4
constant deleteDelay      = Test.tmp + 6

constant N_BLANK_ENTITIES = 12
constant ENTITIES_PER_GAMELOOP = 12
constant N_GAMELOOPS = 60

assert(N_GAMELOOPS * ENTITIES_PER_GAMELOOP > 4 * (Entity.MAX_INSTANCE_ID >> 1))
assert(N_GAMELOOPS * ENTITIES_PER_GAMELOOP > 10 * Entity.N_ENTITIES)
assert(ENTITIES_PER_GAMELOOP * 4 + N_BLANK_ENTITIES < Entity.N_ENTITIES)


    jsr     Entity.Init

_AfterInit:
    lda.w   #N_GAMELOOPS - 1
    sta.w   outerLoopCounter

    lda.w   #5
    sta.w   deleteDelay


    // Spawn 10 Blank entities
    // (This tests the system skips over used instanceIds)

    lda.w   #N_BLANK_ENTITIES
    sta.w   spawnLoopCounter

    BlankEntitySpawnLoop:
        // Delete entity after deleteDelay frames
        // Y = deleteDelay
        lda.w   #Project.EntityIds.DeleteAfterDelay_0
        jsr     Entity.Spawn
        bcc     Fail

        dec.w   spawnLoopCounter
        bpl     BlankEntitySpawnLoop



    OuterLoop:
        lda.w   #ENTITIES_PER_GAMELOOP - 1
        sta.w   spawnLoopCounter

        SpawnLoop:
            // adjust deleteDelay on every spawn
            // This will (badly) shuffle the entityPool and instanceIds
            ldy.w   deleteDelay
            dey
            cpy.w   #2
            bcs     +
                ldy.w   #5      // a prime number
            +
            sty.w   deleteDelay

            // Delete entity after deleteDelay frames
            // Y = deleteDelay
            lda.w   #Project.EntityIds.DeleteAfterDelay_0
            jsr     Entity.Spawn
            bcc     Fail

            // Y = newly spawned entity
            sty.w   entityAddr


            // Test instanceId not zero and >= Entity.minimumInstanceId
            sep     #$20
        a8()
            lda.w   BaseEntity.instanceId,y
            beq     Fail

            cmp.w   Entity.minimumInstanceId
            bcc     Fail

            rep     #$30
        a16()


            // Test instanceIdTable value is correct
            Entity.GetEntityWithIdOrBranchIfEmpty_Y(Fail)
            cpy.w   entityAddr
            bne     Fail


            dec.w   spawnLoopCounter
            bpl     SpawnLoop


        // will also call ValidateEntityLists
        jsr     ProcessEntityGameLoop


        dec.w   outerLoopCounter
        bpl     OuterLoop

    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}

// Run the InstanceIdLoopTest again, this time with `minimumInstanceId` set
a16()
i16()
code()
Test.add("Entity.InstanceId loop (min)")
function InstanceIdLoopTest_Min {
constant MIN_INSTANCE_ID = Entity.MAX_INSTANCE_ID - Entity.N_ENTITIES * 2
assert(MIN_INSTANCE_ID & 1 == 0)

    jsr     Entity.Init

    lda.w   #MIN_INSTANCE_ID
    sta.w   Entity.minimumInstanceId

    bra     InstanceIdLoopTest._AfterInit
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

