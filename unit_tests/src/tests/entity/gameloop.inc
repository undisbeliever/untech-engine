// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {

a16()
i16()
code()
Test.add("Entity.GameLoop (ChangeList)")
function GameLoop_ChangeList {
constant tmp_index = Test.tmp
constant tmp_test = Test.tmp + 2

    // Spawns many entities that will just change their
    // list to the particle list, processes one Entity
    // Game Loop frame and verifies that the list sizes
    // are correct.

    jsr     Entity.Init

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    ldx.w   #0
    SpawnLoop:
        stx.w   tmp_index

        lda.l   SpawnList,x
        ldy.w   #1
        jsr     Entity.Spawn
        bcc     Fail

        ldx.w   tmp_index
        inx
        inx

        cpx.w   #SpawnList.size
        bcc     SpawnLoop


    // process GameLoop more than once
    lda.w   #3
    sta.w   tmp_test

    TestLoop:
        jsr     ProcessEntityGameLoop

        ldx.w   #ExpectedSizes
        jsr     _TestListLengthTable
        bcc     Fail

        dec.w   tmp_test
        bne     TestLoop

    jmp     _TestEntityListsValid

Fail:
    clc
    rts

rodata(rom0)
SpawnList: {
    define C = Project.EntityIds.ChangeToParticle_
    define B = Project.EntityIds.Blank_

    evaluate currentList = 0
    inline row(suffix, a, b, c, d, e) {
        dw {a}{suffix}, {b}{suffix}, {c}{suffix}, {d}{suffix}, {e}{suffix}

        updateCount({suffix}, {a})
        updateCount({suffix}, {b})
        updateCount({suffix}, {c})
        updateCount({suffix}, {d})
        updateCount({suffix}, {e})
    }

    variable count_Particles = 0
    variable count_PlayerProjectiles = 0
    variable count_EnemyProjectiles = 0
    variable count_Enemies = 0

    evaluate n = 0
    while {n} < 10 {
        variable count_{n} = 0
        evaluate n = {n} + 1
    }

    inline updateCount(suffix, e) {
        if {e}0 == Project.EntityIds.ChangeToParticle_0 {
            // Change to particle
            count_Particles = count_Particles + 1
        } else {
            count_{suffix} = count_{suffix} + 1
        }
    }

    row(0,  {C}, {C}, {B}, {B}, {B})
    row(1,  {B}, {B}, {B}, {C}, {C})
    row(2,  {C}, {B}, {B}, {B}, {B})
    row(3,  {B}, {B}, {C}, {B}, {B})
    row(4,  {C}, {C}, {C}, {C}, {C})    // change all
    row(5,  {C}, {B}, {C}, {B}, {C})
    row(6,  {B}, {C}, {B}, {C}, {B})
    row(6,  {B}, {B}, {B}, {B}, {B})    // change none
    row(8,  {B}, {C}, {C}, {C}, {C})
    row(9,  {C}, {C}, {C}, {C}, {B})

    constant SpawnList.size = pc() - SpawnList
    assert(SpawnList.size/2 <= Entity.N_ENTITIES)
}

ExpectedSizes: {
    dw count_Particles
    dw count_PlayerProjectiles
    dw count_EnemyProjectiles
    dw count_Enemies
    evaluate n = 0
    while {n} < 10 {
        dw count_{n}
        evaluate n = {n} + 1
    }

    constant ExpectedSizes.size = pc() - ExpectedSizes
    assert(ExpectedSizes.size / 2 == Entity.N_ENTITY_LISTS)
}
}



a16()
i16()
code()
Test.add("Entity.GameLoop (Delete)")
function GameLoop_Delete {
constant tmp = Test.tmp

constant FIRST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_0
constant LAST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_9

constant N_SPAWNED_LISTS = LAST_ENTITY_ID - FIRST_ENTITY_ID + 1

assert(N_SPAWNED_LISTS == 10)
assert(N_SPAWNED_LISTS * 4 <= Entity.N_ENTITIES)

    jsr     Entity.Init

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    // spawn 3 entities per list
    // these entities will delete themselves after Y frames
    lda.w   #FIRST_ENTITY_ID
    Loop:
        sta.w   tmp

        ldy.w   #1
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #4
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #2
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #2
        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #LAST_ENTITY_ID + 1
        bcc     Loop

    // Test list lengths

    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - N_SPAWNED_LISTS * 4
    bne     Fail

    // frame 1 - 1 list of entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - N_SPAWNED_LISTS * 3
    bne     Fail

    // frame 2 - 2 lists of entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - N_SPAWNED_LISTS * 1
    bne     Fail

    // frame 3 - no entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - N_SPAWNED_LISTS * 1
    bne     Fail

    // frame 4 - 1 list of entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - 0
    bne     Fail

    // frame 5 - no entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - 0
    bne     Fail

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entity.GameLoop.ActivateNextFrame")
function GameLoop_ActivateNextFrame {
constant tmp = Test.tmp

constant FIRST_ENTITY_LIST_ID = EntityLists.EntityList_0
constant LAST_ENTITY_LIST_ID  = EntityLists.EntityList_9

constant FIRST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_0
constant LAST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_9

constant N_SPAWNED_LISTS = LAST_ENTITY_ID - FIRST_ENTITY_ID + 1
assert(N_SPAWNED_LISTS == 10)

    jsr     Entity.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft

    function CreateEntities {
        lda.w   #0x1000
        sta.w   Entity.Spawn.xPos
        sta.w   Entity.Spawn.yPos

        assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS)

        lda.w   #FIRST_ENTITY_ID
        Loop:
            sta.w   tmp

            ldy.w   #2
            jsr     Entity.Spawn
            bcc     Fail

            lda.w   tmp
            inc
            cmp.w   #LAST_ENTITY_ID + 1
            bcc     Loop


        jsr     _ActivateNextFrameListLength
        cmp.w   #N_SPAWNED_LISTS
        bne     Fail

        // Test each list has no entities in them (not active)
        lda.w   #FIRST_ENTITY_LIST_ID
        ListLoop:
            pha
                jsr     Entity.CountEntitiesInList
            ply

            cmp.w   #0
            bne     Fail

            tya
            inc
            cmp.w   #LAST_ENTITY_LIST_ID + 1
            bcc     ListLoop

        jsr     _TestEntityListsValid
        bcc     Fail
    }

    // Test that entities are allocated in the next frame
    function TestActivateNextFrame {
        jsr     Dma.Init
        jsr     WaitFrame

        jsr     ProcessEntityGameLoop

        jsr     _ActivateNextFrameListLength
        cmp.w   #0
        bne     Fail

        // Test each list has no entities in them (not active)
        lda.w   #FIRST_ENTITY_LIST_ID
        Loop:
            pha
                jsr     Entity.CountEntitiesInList
            ply

            cmp.w   #1
            bne     Fail

            tya
            inc
            cmp.w   #LAST_ENTITY_LIST_ID + 1
            bcc     Loop

        jsr     _TestEntityListsValid
        bcc     Fail
    }

    // After the second frame the entities should be deleted
    // Tests that they were actually processed
    function TestNextFrame {
        jsr     ProcessEntityGameLoop

        jsr     _FreeListLength
        cmp.w   #Entity.N_ENTITIES
        bne     Fail
    }

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entity.GameLoop (Delete Animation Ends)")
function GameLoop_DeleteIfAnimationsEnds {
constant entityAddress  = Test.tmp + 0
constant firstFrame   = Test.tmp + 2

    jsr     Entity.Init

    assert(Entity.N_ENTITY_LISTS * 3 <= Entity.N_ENTITIES)

    // Spawn a DeleteIfAnimationEnds Entity
    // This entity has a single animation
    //   which draws frame1 for two display frames
    //   then draws frame2 for two display frames
    //   then deletes itself on the fifth frame

    lda.w   #Project.EntityIds.DeleteIfAnimationEnds
    jsr     Entity.Spawn
    sty.w   entityAddress


    // display frame 1
    jsr     ProcessEntityGameLoop

    // save currentFrame
    ldy.w   entityAddress
    lda.w   BaseEntity.MetaSprite.currentFrame,y
    sta.w   firstFrame


    // display frame 2
    jsr     ProcessEntityGameLoop

    // test there is one entity in play
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - 1
    bne     Fail

    // Check entity frame changed

    ldy.w   entityAddress
    lda.w   BaseEntity.MetaSprite.currentFrame,y
    cmp.w   firstFrame
    bne     Fail


    // display frame 3
    jsr     ProcessEntityGameLoop

    // test there is one still one entity in play
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - 1
    bne     Fail


    // display frame 4
    jsr     ProcessEntityGameLoop


    // display frame 5
    jsr     ProcessEntityGameLoop

    // test the entity has been deleted
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES
    bne     Fail

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



// Tests:
//   * Entity.DeactivateIfOutsideAndGotoNextEntity
//   * Entity._ProcessActiveWindow
//   * BaseEntityFunctionTable.Deactivated called successfully
a16()
i16()
code()
Test.add("GameLoop (Move Camera)")
function GameLoop_MoveCamera {
constant tmp_index = Test.tmp
constant tmp_spawnIndex = Test.tmp + 2

    jsr     Entity.Init

    // Spawn 4 entities at different point on the screen
    assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS * 4)

    lda.w   #SpawnTable.size - 1
    sta.w   tmp_spawnIndex

    Loop:
        macro spawn(xPos, yPos) {
            ldx.w   #{xPos}
            ldy.w   #{yPos}
            jsr     SpawnEntity_XY
            bcc     Fail
        }

        spawn(0x1000 + 40, 0x1000 - 40)
        spawn(0x1000 - 40, 0x2000 + 40)
        spawn(0x2000 - 40, 0x1000 + 40)
        spawn(0x2000 + 40, 0x1000 + Camera.DISPLAY_HEIGHT + Entity.ACTIVE_WINDOW_EXTEND_VERTICAL + 1)

        dec.w   tmp_spawnIndex
        bpl     Loop


    // Test Initial Entity counts
    jsr     ProcessEntityGameLoop

    ldx.w   #InitialExpectedSizes
    jsr     _TestListLengthTable
    bcc     Fail

    jsr     _TestEntityListsValid
    bcs     +
    Fail:
        clc
        rts
    +


    // Test move camera
    // Only test NPCs and Entities
    // (as they are the ones that deactivate)
    namespace TestMoveCamera {
        ldx.w   #0
        Loop:
            stx.w   tmp_index

            lda.l   MoveCameraTable + 0,x
            sta.w   Camera.xPos

            lda.l   MoveCameraTable + 2,x
            sta.w   Camera.yPos


            stz.w   Entities.DeactivateIfOutside.activatedCount
            stz.w   Entities.DeactivateIfOutside.deactivatedCount

            jsr     ProcessEntityGameLoop


            // test activatedCount and deactivatedCount
            ldx.w   tmp_index

            lda.w   Entities.DeactivateIfOutside.activatedCount
            cmp.l   MoveCameraTable + 4,x
            bne     Fail

            lda.w   Entities.DeactivateIfOutside.deactivatedCount
            cmp.l   MoveCameraTable + 6,x
            bne     Fail


            // Test list lengths
            // X = tmp_index
            txa
            clc
            adc.w   #MoveCameraTable + 8
            tax
            jsr     _TestListLengthTable
            bcc     Fail


            jsr     _TestEntityListsValid
            bcc     Fail


            lda.w   tmp_index
            clc
            adc.w   #MoveCameraTable.rowSize
            tax
            cpx.w   #MoveCameraTable.size
            bcc     Loop
    }

    jmp     _TestEntityListsValid

function SpawnEntity_XY {
    stx.w   Entity.Spawn.xPos
    sty.w   Entity.Spawn.yPos

    ldx.w   tmp_spawnIndex
    lda.l   SpawnTable,x
    and.w   #0xff

    jmp     Entity.Spawn
}

rodata(rom0)

SpawnTable:
    db  Project.EntityIds.Blank_0
    db  Project.EntityIds.DeleteIfOutside_1
    db  Project.EntityIds.DeleteIfOutside_2
    db  Project.EntityIds.DeactivateIfOutside_3
    db  Project.EntityIds.Blank_4
    db  Project.EntityIds.DeactivateIfOutside_5
    db  Project.EntityIds.Blank_6
    db  Project.EntityIds.DeleteIfOutside_7
    db  Project.EntityIds.DeactivateIfOutside_8
    // 9 empty

constant SpawnTable.size = pc() - SpawnTable
assert(SpawnTable.size < Entity.N_ENTITY_LISTS)

InitialExpectedSizes: {
    dw  0
    dw  0
    dw  0
    dw  0
    assert(EntityLists.EntityList_0 == (pc() - InitialExpectedSizes) / 2)

    dw  4   // blank
    dw  1   // delete if outside
    dw  1   // delete if outside
    dw  1   // deactivate if outside
    dw  4   // blank
    dw  1   // deactivate if outside
    dw  4   // blank
    dw  1   // delete if outside
    dw  1   // deactivate if outside
    dw  0   // empty

    assert((pc() - InitialExpectedSizes) / 2 == Entity.N_ENTITY_LISTS)
}

rodata(rom0)
MoveCameraTable: {
    macro _row(xPos, yPos, e, activated, deactivated) {
        dw  {xPos}, {yPos}
        dw  {activated} * 3
        dw  {deactivated} * 3

    ExpectedSizes{#}:
        dw  0, 0, 0, 0
        dw  4, 0, 0, {e}, 4, {e}, 4, 0, {e}, 0

        assert((pc() - ExpectedSizes{#}) / 2 == Entity.N_ENTITY_LISTS)
    }

    //   xPos    yPos    count  activated   deactivated
    _row(0x1000, 0x2000, 1,     1,          1)
    _row(0x2000, 0x2000, 0,     0,          1)
    _row(0x2000, 0x1000, 1,     1,          0)
    _row(0x2000, 0x1010, 2,     1,          0)
    _row(0x2000, 0x1000, 2,     0,          0)
    _row(0x3000, 0x3000, 0,     0,          2)

    constant MoveCameraTable.rowSize = (4 + Entity.N_ENTITY_LISTS) * 2
    constant MoveCameraTable.size = pc() - MoveCameraTable
}
}



// Tests that an entity can spawn a new entity and change its entity
// list at the same time.
a16()
i16()
code()
Test.add("GameLoop (Spawn And Change List)")
function GameLoop_SpawnAndChangeList {
constant _loopsLeft = Test.dpTmp + 0

    jsr     Entity.Init

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    ldy.w   #Entity.N_ENTITY_LISTS - 1
    SpawnLoop:
        phy
            // Y = listId to spawn entity in
            lda.w   #Project.EntityIds.SpawnAndChangeList

            jsr     Entity.Spawn
            bcc     Fail
        ply
        dey
        bpl     SpawnLoop

    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS
    bne     Fail

    lda.w   #20
    sta.b   _loopsLeft

    Loop:
        jsr     ProcessEntityGameLoop

        jsr     _TestEntityListsValid
        bcc     Fail

        dec.b   _loopsLeft
        bne     Loop


    jmp     _TestEntityListsValid


Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

