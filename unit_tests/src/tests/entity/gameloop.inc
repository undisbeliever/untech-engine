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
    define C = Entities.ChangeToParticleEntity.EntityDataId_
    define B = Entities.BlankEntity.EntityDataId_

    dw  {C}0, {C}0, {B}0, {B}0, {B}0    // player
    dw  {B}1, {C}1, {C}1, {B}1, {B}1    // boss
    dw  {B}2, {C}2, {B}2, {C}2, {B}2    // enemy
    dw  {B}3, {B}3, {B}3, {C}3, {C}3    // npc
    dw  {C}4, {C}4, {C}4, {C}4, {C}4    // platform
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
    dw  0                   // playerprojectiles
    dw  0                   // enemyprojectiles

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

    jsr     Entity.Init

    assert(Entity.N_ENTITY_LISTS * 3 <= Entity.N_ENTITIES)

    lda.w   #0x1000
    sta.w   Entity.Spawn.xPos
    sta.w   Entity.Spawn.yPos

    // spawn 3 entities per list
    // these entities will delete themselves after Y frames
    lda.w   #Entities.DeleteAfterDelayEntity.EntityDataId_0
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
        cmp.w   #Entities.DeleteAfterDelayEntity.EntityDataId_0 + Entity.N_ENTITY_LISTS
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

    jsr     Entity.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft

    function CreateEntities {
        lda.w   #0x1000
        sta.w   Entity.Spawn.xPos
        sta.w   Entity.Spawn.yPos

        assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS)

        lda.w   #Entities.DeleteAfterDelayEntity.EntityDataId_0
        Loop:
            sta.w   tmp

            ldy.w   #2
            jsr     Entity.Spawn
            bcc     Fail

            lda.w   tmp
            inc
            cmp.w   #Entities.DeleteAfterDelayEntity.EntityDataId_0 + Entity.N_ENTITY_LISTS
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

    // Spawn a DeleteIfAnimationEndsEntity
    // This entity has a single animation
    //   which draws frame1 for two display frames
    //   then draws frame2 for two display frames
    //   then deletes itself on the fifth frame

    lda.w   #Entities.DeleteIfAnimationEndsEntity.EntityDataId
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
constant tmp_listId = Test.tmp + 2

    jsr     Entity.Init

    // Spawn 4 entities at different point on the screen
    assert(Entity.N_ENTITIES >= Entity.N_ENTITY_LISTS * 4)

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
        cmp.w   #OldBehaviourTestEntities.firstId + Entity.N_ENTITY_LISTS
        bcc     Loop


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


            stz.w   Entities.DeactivateIfOutsideEntity.activatedCount
            stz.w   Entities.DeactivateIfOutsideEntity.deactivatedCount

            jsr     ProcessEntityGameLoop


            // test activatedCount and deactivatedCount
            ldx.w   tmp_index

            lda.w   Entities.DeactivateIfOutsideEntity.activatedCount
            cmp.l   MoveCameraTable + 4,x
            bne     Fail

            lda.w   Entities.DeactivateIfOutsideEntity.deactivatedCount
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
    lda.w   tmp_listId
    jmp     Entity.Spawn
}

namespace OldBehaviourTestEntities {
    assert(Entity.N_ENTITY_LISTS == 8)

    rodata(EN_EntityDataList)
    constant firstId = (pc() - Entity.Data.EntityDataList) / 2
        dw  Entities.BlankEntity.RomData0               // player list
        dw  Entities.BlankEntity.RomData1               // boss list
        dw  Entities.DeactivateIfOutsideEntity.RomData2 // enemy list
        dw  Entities.DeactivateIfOutsideEntity.RomData3 // npc list
        dw  Entities.DeactivateIfOutsideEntity.RomData4 // platform list
        dw  Entities.BlankEntity.RomData5               // particle list
        dw  Entities.DeleteIfOutsideEntity.RomData6     // playerProjectile list
        dw  Entities.DeleteIfOutsideEntity.RomData7     // enemyProjectile list
}

rodata(rom0)
InitialExpectedSizes: {
    dw  4   // player
    dw  4   // boss
    dw  1   // enemy
    dw  1   // npc
    dw  1   // platform
    dw  4   // particle
    dw  1   // player projectile
    dw  1   // enemy projectile

    assert((pc() - InitialExpectedSizes) / 2 == Entity.N_ENTITY_LISTS)
}

rodata(rom0)
MoveCameraTable: {
    macro _row(xPos, yPos, e, activated, deactivated) {
        dw  {xPos}, {yPos}
        dw  {activated} * 3
        dw  {deactivated} * 3

        dw  4   // player
        dw  4   // boss
        dw  {e} // enemy
        dw  {e} // npc
        dw  {e} // platform
        dw  4   // particle
        dw  0   // player projectile
        dw  0   // enemy projectile
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
            lda.w   #SpawnAndChangeListTestEntity.EntityDataId

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


namespace SpawnAndChangeListTestEntity {
    buildFunctionTable(BaseEntityFunctionTable, SpawnAndChangeListTestEntity)

    constant DELETE_DELAY = 2


    // IN: A = entity parameter = entity list id
    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        sep     #$20
    a8()
        sta.b   BaseEntity.listId

        rep     #$30
    a16()

        // always activate
        sec
        rts
    }


    a16()
    i16()
    code()
    function Process {
        // This entity spawns 3 entities:
        //  * DeleteAfterDelayEntity with listId = listId
        //  * DeleteAfterDelayEntity with lidtId = listId - 1
        //  * DeleteAfterDelayEntity with listId = listId + 1
        //
        // then changes its entity list to the previous list.

        lda.b   BaseEntity.listId
        and.w   #0xff
        jsr     SpawnDeleteEntity


        lda.b   BaseEntity.listId
        and.w   #0xff
        dec
        jsr     SpawnDeleteEntity


        lda.b   BaseEntity.listId
        and.w   #0xff
        inc
        jsr     SpawnDeleteEntity


        lda.b   BaseEntity.listId
        and.w   #0xff
        dec
        bpl     +
            lda.w   #Entity.N_ENTITY_LISTS - 1
        +
        jmp     Entity.ChangeEntityListIdAndGotoNextEntity
    }


    // IN: A = listId to spawn DeleteAfterDelayEntity in.
    a16()
    i16()
    code()
    function SpawnDeleteEntity {
        // ensure entity is spawned
        ldx.w   Camera.xPos
        stx.w   Entity.Spawn.xPos
        ldx.w   Camera.yPos
        stx.w   Entity.Spawn.yPos

        // wrap list id if necessary
        tay
        bpl     +
            // A < 0
            lda.w   #Entity.N_ENTITY_LISTS - 1
        +
        cmp.w   #Entity.N_ENTITY_LISTS
        bcc     +
            lda.w   #0
        +

        clc
        adc.w   #Entities.DeleteAfterDelayEntity.EntityDataId_0
        ldy.w   #DELETE_DELAY
        jmp     Entity.Spawn
    }


    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
    function NULL {
        rts
    }


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  0               // initialListId
        db  0               // defaultPalette
        dw  0               // frameSetId

    rodata(EN_EntityDataList)
    constant EntityDataId = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

