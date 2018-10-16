// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
        jsr     Entity.ProcessGameLoop

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
    define C = Entities.ChangeToParticleEntity.EntityId_
    define B = Entities.BlankEntity.EntityId_

    dw  {C}0, {C}0, {B}0, {B}0, {B}0    // player
    dw  {B}1, {C}1, {C}1, {B}1, {B}1    // boss
    dw  {B}2, {C}2, {B}2, {C}2, {B}2    // enemy
    dw  {B}3, {B}3, {B}3, {C}3, {C}3    // npc
    dw  {C}4, {C}4, {C}4, {C}4, {C}4    // playform
    dw  {B}5, {B}5, {B}5, {B}5, {B}5    // particle

    constant SpawnList.size = pc() - SpawnList
    assert(SpawnList.size/2 <= Entity.N_ENTITIES)
}

ExpectedSizes: {
    dw  3                   // player
    dw  3                   // boss
    dw  3                   // enemy
    dw  3                   // npc
    dw  0                   // platform
    dw  (6 * 5) - (3 * 4)   // particle

    constant ExpectedSizes.size = pc() - ExpectedSizes
    assert(ExpectedSizes.size / 2 == Entity.lists.N_LISTS)
}
}



a16()
i16()
code()
Test.add("Entity.GameLoop (Delete)")
function GameLoop_Delete {
constant tmp = Test.tmp

    jsr     Entity.Init

    assert(Entity.lists.N_LISTS * 3 <= Entity.N_ENTITIES)

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    // spawn 3 entities per list
    // these entities will delete themselves after Y frames
    lda.w   #Entities.DeleteAfterDelayEntity.EntityId_0
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
        cmp.w   #Entities.DeleteAfterDelayEntity.EntityId_0 + Entity.lists.N_LISTS
        bcc     Loop

    // Test list lengths

    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 4
    bne     Fail

    // frame 1 - 1 list of entities deleted
    jsr     Entity.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 3
    bne     Fail

    // frame 2 - 2 lists of entities deleted
    jsr     Entity.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 1
    bne     Fail

    // frame 3 - no entities deleted
    jsr     Entity.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 1
    bne     Fail

    // frame 4 - 1 list of entities deleted
    jsr     Entity.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 0
    bne     Fail

    // frame 5 - no entities deleted
    jsr     Entity.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entity.N_ENTITIES - Entity.lists.N_LISTS * 0
    bne     Fail

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entity.GameLoop.TryToActivateAgain")
function GameLoop_TryToActivateAgain {
constant tmp = Test.tmp

    jsr     Entity.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft

    function CreateEntities {
        lda.w   #0x1000
        sta.w   Entity.Spawn.xPos
        sta.w   Entity.Spawn.yPos

        assert(Entity.N_ENTITIES >= Entity.lists.N_LISTS)

        lda.w   #Entities.DeleteAfterDelayEntity.EntityId_0
        Loop:
            sta.w   tmp

            ldy.w   #2
            jsr     Entity.Spawn
            bcc     Fail

            lda.w   tmp
            inc
            cmp.w   #Entities.DeleteAfterDelayEntity.EntityId_0 + Entity.lists.N_LISTS
            bcc     Loop


        jsr     _TryToActivateAgainListLength
        cmp.w   #Entity.lists.N_LISTS
        bne     Fail

        // Test each list has no entities in them (not active)
        ldy.w   #0
        ListLoop:
            phy
                jsr     _ListLength_Y
            ply

            cmp.w   #0
            bne     Fail

            iny
            cpy.w   #Entity.lists.N_LISTS
            bcc     ListLoop

        jsr     _TestEntityListsValid
        bcc     Fail
    }

    // Test that entities are allocated in the next frame
    function TestActivateNextFrame {
        jsr     Dma.Init
        jsr     WaitFrame

        jsr     Entity.ProcessGameLoop

        jsr     _TryToActivateAgainListLength
        cmp.w   #0
        bne     Fail

        // Test each list has no entities in them (not active)
        ldy.w   #0
        Loop:
            phy
                jsr     _ListLength_Y
            ply

            cmp.w   #1
            bne     Fail

            iny
            cpy.w   #Entity.lists.N_LISTS
            bcc     Loop

        jsr     _TestEntityListsValid
        bcc     Fail
    }

    // After the second frame the entities should be deleted
    // Tests that they were actually processed
    function TestNextFrame {
        jsr     Entity.ProcessGameLoop

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
Test.add("GameLoop (Move Camera)")
function GameLoop_MoveCamera {
constant tmp_index = Test.tmp
constant tmp_listId = Test.tmp + 2

    jsr     Entity.Init

    // Spawn 4 entities at different point on the screen
    assert(Entity.N_ENTITIES >= Entity.lists.N_LISTS * 4)

    lda.w   #OldBehaviourTestEntities.firstId
    Loop:
        sta.w   tmp_listId

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

        lda.w   tmp_listId
        inc
        cmp.w   #OldBehaviourTestEntities.firstId + Entity.lists.N_LISTS
        bcc     Loop


    // Test Initial Entity counts
    jsr     Entity.ProcessGameLoop

    ldx.w   #InitialExpectedSizes
    jsr     _TestListLengthTable
    bcc     Fail

    jsr     _TestEntityListsValid
    bcc     Fail


    // Test move camera
    // Only test NPCs and Entities
    // (as they are the ones that deactivate)
    function TestMoveCamera {
        ldx.w   #0
        Loop:
            stx.w   tmp_index

            lda.l   MoveCameraTable + 0,x
            sta.w   Camera.xPos

            lda.l   MoveCameraTable + 2,x
            sta.w   Camera.yPos

            jsr     Entity.ProcessGameLoop


            // Test list lengths
            lda.w   tmp_index
            clc
            adc.w   #MoveCameraTable + 4
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

Fail:
    clc
    rts

function SpawnEntity_XY {
    stx.w   Entity.Spawn.xPos
    sty.w   Entity.Spawn.yPos
    lda.w   tmp_listId
    jmp     Entity.Spawn
}

namespace OldBehaviourTestEntities {
    assert(Entity.lists.N_LISTS == 6)

    rodata(EN_EntityList)
    constant firstId = (pc() - Entity.Data.EntityList) / 2
        dw  Entities.BlankEntity.RomData0               // player list
        dw  Entities.BlankEntity.RomData1               // boss list
        dw  Entities.DeactivateIfOutsideEntity.RomData2 // enemy list
        dw  Entities.DeactivateIfOutsideEntity.RomData3 // npc list
        dw  Entities.DeactivateIfOutsideEntity.RomData4 // platform list
        dw  Entities.BlankEntity.RomData5               // particle list
}

rodata(rom0)
InitialExpectedSizes: {
    dw  4   // player
    dw  4   // boss
    dw  1   // enemy
    dw  1   // npc
    dw  1   // platform
    dw  4   // particle

    assert((pc() - InitialExpectedSizes) / 2 == Entity.lists.N_LISTS)
}

rodata(rom0)
MoveCameraTable: {
    macro _row(xPos, yPos, e) {
        dw  {xPos}, {yPos}
        dw  4   // player
        dw  4   // boss
        dw  {e} // enemy
        dw  {e} // npc
        dw  {e} // platform
        dw  4   // particle
    }

    _row(0x1000, 0x2000, 1)
    _row(0x2000, 0x2000, 0)
    _row(0x2000, 0x1000, 1)
    _row(0x2000, 0x1010, 2)
    _row(0x2000, 0x1000, 2) // Still inside inactive window
    _row(0x3000, 0x3000, 0)

    constant MoveCameraTable.rowSize = (2 + Entity.lists.N_LISTS) * 2
    constant MoveCameraTable.size = pc() - MoveCameraTable
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

