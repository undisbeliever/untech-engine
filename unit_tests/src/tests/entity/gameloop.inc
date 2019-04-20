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

    assert(EntityLists.Particles == 0)
    assert(Entity.N_ENTITY_LISTS == 6)

    evaluate currentList = 0
    inline row(listName, suffix, a, b, c, d, e) {
        assert(EntityLists.{listName} == {currentList})
        evaluate currentList = {currentList} + 1

        dw {a}{suffix}, {b}{suffix}, {c}{suffix}, {d}{suffix}, {e}{suffix}
    }

    row(Particles,          Particle,           {C}, {C}, {B}, {B}, {B})
    row(PlayerProjectiles,  PlayerProjectile,   {B}, {C}, {C}, {B}, {B}) 
    row(EnemyProjectiles,   EnemyProjectile,    {B}, {C}, {B}, {C}, {B})
    row(Platforms,          Platform,           {B}, {B}, {B}, {C}, {C})
    row(Players,            Player,             {C}, {C}, {C}, {C}, {C})    // change all
    row(Enemies,            Enemy,              {B}, {B}, {B}, {B}, {B})    // change none

    constant SpawnList.size = pc() - SpawnList
    assert(SpawnList.size/2 <= Entity.N_ENTITIES)
}

ExpectedSizes: {
    dw  (5 * 5) - (3 * 3)   // particles
    dw  3
    dw  3
    dw  3
    dw  0                   // Change all to particles
    dw  5                   // Change none

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

assert(Entity.N_ENTITY_LISTS * 3 <= Entity.N_ENTITIES)

assert(EntityLists.Particles == 0)
constant FIRST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_Particle
assert(EntityLists.Enemies == Entity.N_ENTITY_LISTS - 1)
constant LAST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_Enemy

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
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS * 4
    bne     Fail

    // frame 1 - 1 list of entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS * 3
    bne     Fail

    // frame 2 - 2 lists of entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS * 1
    bne     Fail

    // frame 3 - no entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS * 1
    bne     Fail

    // frame 4 - 1 list of entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS * 0
    bne     Fail

    // frame 5 - no entities deleted
    jsr     ProcessEntityGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.N_ENTITY_LISTS * 0
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

assert(EntityLists.Particles == 0)
constant FIRST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_Particle
assert(EntityLists.Enemies == Entity.N_ENTITY_LISTS - 1)
constant LAST_ENTITY_ID = Project.EntityIds.DeleteAfterDelay_Enemy

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
        cmp.w   #Entity.N_ENTITY_LISTS
        bne     Fail

        // Test each list has no entities in them (not active)
        lda.w   #0
        ListLoop:
            pha
                jsr     Entity.CountEntitiesInList
            ply

            cmp.w   #0
            bne     Fail

            tya
            inc
            cmp.w   #Entity.N_ENTITY_LISTS
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
        lda.w   #0
        Loop:
            pha
                jsr     Entity.CountEntitiesInList
            ply

            cmp.w   #1
            bne     Fail

            tya
            inc
            cmp.w   #Entity.N_ENTITY_LISTS
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

    lda.w   #Entity.N_ENTITY_LISTS - 1
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
    db  Project.EntityIds.Blank_Particle
    db  Project.EntityIds.DeleteIfOutside_PlayerProjectile
    db  Project.EntityIds.DeleteIfOutside_EnemyProjectile
    db  Project.EntityIds.DeactivateIfOutside_Platform
    db  Project.EntityIds.Blank_Player
    db  Project.EntityIds.DeactivateIfOutside_Enemy

constant SpawnTable.size = pc() - SpawnTable
assert(SpawnTable.size == Entity.N_ENTITY_LISTS)

InitialExpectedSizes: {
    dw  4   // particles
    dw  1   // player projectiles
    dw  1   // enemy projectiles
    dw  1   // platforms
    dw  4   // players
    dw  1   // enemies

    assert((pc() - InitialExpectedSizes) / 2 == Entity.N_ENTITY_LISTS)
}

rodata(rom0)
MoveCameraTable: {
    macro _row(xPos, yPos, e, activated, deactivated) {
        dw  {xPos}, {yPos}
        dw  {activated} * 2
        dw  {deactivated} * 2

        dw  4   // particles
        dw  0   // player projectiles
        dw  0   // enemy projectiles
        dw  {e} // platforms
        dw  4   // players
        dw  {e} // enemies
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

