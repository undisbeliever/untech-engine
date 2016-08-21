
scope EntitiesTest {

a16()
i16()
code()
Test.add("Entities.GameLoop (ChangeList)")
scope GameLoop_ChangeList: {
constant tmp_index(Test.tmp)
constant tmp_test(Test.tmp + 2)

    // Spawns many entities that will just change their
    // list to the particle list, processes one Entity
    // Game Loop frame and verifies that the list sizes
    // are correct.

    jsr     Entities.Init

    lda.w   #0x1000
    sta.w   Entities.Spawn.xPos
    sta.w   Entities.Spawn.yPos

    ldx.w   #0
    SpawnLoop:
        stx.w   tmp_index

        lda.l   SpawnList,x
        ldy.w   #1
        jsr     Entities.Spawn
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
        jsr     Entities.ProcessGameLoop

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
scope SpawnList: {
    define C(ChangeToParticleEntity.EntityId_)
    define B(BlankEntity.EntityId_)

    dw  {C}0, {C}0, {B}0, {B}0, {B}0
    dw  {B}1, {C}1, {C}1, {B}1, {B}1
    dw  {B}2, {C}2, {B}2, {C}2, {B}2
    dw  {B}3, {B}3, {B}3, {C}3, {C}3
    dw  {C}4, {C}4, {C}4, {C}4, {C}4
    dw  {B}5, {B}5, {B}5, {B}5, {B}5
    dw  {C}6, {C}6, {B}6, {C}6, {C}6
    dw  {B}7, {B}7, {C}7, {B}7, {C}7

    constant size(pc() - SpawnList)
    assert(size/2 <= Entities.N_ENTITIES)
}

scope ExpectedSizes: {
    dw  3
    dw  3
    dw  3
    dw  3
    dw  0
    dw  5
    dw  1
    dw  40 - (3 * 4 + 5 + 1)

    constant size(pc() - ExpectedSizes)
    assert(size / 2 == Entities.lists.N_LISTS)
}
}



a16()
i16()
code()
Test.add("Entities.GameLoop (Delete)")
scope GameLoop_Delete: {
constant tmp(Test.tmp)

    jsr     Entities.Init

    assert(Entities.lists.N_LISTS * 3 <= Entities.N_ENTITIES)

    lda.w   #0x1000
    sta.w   Entities.Spawn.xPos
    sta.w   Entities.Spawn.yPos

    // spawn 3 entities per list
    // these entities will delete themselves after Y frames
    lda.w   #DeleteAfterDelayEntity.EntityId_0
    Loop:
        sta.w   tmp

        ldy.w   #1
        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #4
        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #2
        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #2
        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #DeleteAfterDelayEntity.EntityId_0 + Entities.lists.N_LISTS
        bcc     Loop

    // Test list lengths

    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 4
    bne     Fail

    // frame 1 - 1 list of entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 3
    bne     Fail

    // frame 2 - 2 lists of entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 1
    bne     Fail

    // frame 3 - no entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 1
    bne     Fail

    // frame 4 - 1 list of entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 0
    bne     Fail

    // frame 5 - no entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 0
    bne     Fail

    jmp     _TestEntityListsValid

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.GameLoop.TryToActivateAgain")
scope GameLoop_TryToActivateAgain: {
constant tmp(Test.tmp)

    jsr     Entities.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft

    scope CreateEntities: {
        lda.w   #0x1000
        sta.w   Entities.Spawn.xPos
        sta.w   Entities.Spawn.yPos

        assert(Entities.N_ENTITIES >= Entities.lists.N_LISTS)

        lda.w   #DeleteAfterDelayEntity.EntityId_0
        Loop:
            sta.w   tmp

            ldy.w   #2
            jsr     Entities.Spawn
            bcc     Fail

            lda.w   tmp
            inc
            cmp.w   #DeleteAfterDelayEntity.EntityId_0 + Entities.lists.N_LISTS
            bcc     Loop


        jsr     _TryToActivateAgainListLength
        cmp.w   #Entities.lists.N_LISTS
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
            cpy.w   #Entities.lists.N_LISTS
            bcc     ListLoop

        jsr     _TestEntityListsValid
        bcc     Fail
    }

    // Test that entities are allocated in the next frame
    scope TestActivateNextFrame: {
        jsr     Dma.Init
        jsr     WaitFrame

        jsr     Entities.ProcessGameLoop

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
            cpy.w   #Entities.lists.N_LISTS
            bcc     Loop

        jsr     _TestEntityListsValid
        bcc     Fail
    }

    // After the second frame the entities should be deleted
    // Tests that they were actually processed
    scope TestNextFrame: {
        jsr     Entities.ProcessGameLoop

        jsr     _FreeListLength
        cmp.w   #Entities.N_ENTITIES
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
scope GameLoop_MoveCamera: {
constant tmp_index(Test.tmp)
constant tmp_listId(Test.tmp + 2)

    jsr     Entities.Init

    // Spawn 4 entities at different point on the screen
    assert(Entities.N_ENTITIES >= Entities.lists.N_LISTS * 4)

    lda.w   #BlankEntity.EntityId_0
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
        spawn(0x2000 + 40, 0x1000 + Camera.DISPLAY_HEIGHT + Camera.ACTIVE_WINDOW_EXTEND_VERTICAL + 1)

        lda.w   tmp_listId
        inc
        cmp.w   #BlankEntity.EntityId_0 + Entities.lists.N_LISTS
        bcc     Loop


    // Test Initial Entity counts
    jsr     Entities.ProcessGameLoop

    ldx.w   #InitialExpectedSizes
    jsr     _TestListLengthTable
    bcc     Fail

    jsr     _TestEntityListsValid
    bcc     Fail


    // Test move camera
    // Only test NPCs and Entities
    // (as they are the ones that deactivate)
    scope TestMoveCamera: {
        ldx.w   #0
        Loop:
            stx.w   tmp_index

            lda.l   MoveCameraTable + 0,x
            sta.w   Camera.xPos

            lda.l   MoveCameraTable + 2,x
            sta.w   Camera.yPos

            jsr     Entities.ProcessGameLoop


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

scope SpawnEntity_XY: {
    stx.w   Entities.Spawn.xPos
    sty.w   Entities.Spawn.yPos
    lda.w   tmp_listId
    jmp     Entities.Spawn
}

rodata(rom0)
scope InitialExpectedSizes: {
    dw  4
    dw  4
    dw  1
    dw  1
    dw  1
    dw  1
    dw  1
    dw  4

    constant size(pc() - InitialExpectedSizes)
    assert(size / 2 == Entities.lists.N_LISTS)
}

rodata(rom0)
scope MoveCameraTable: {
    macro _row(xPos, yPos, e) {
        dw  {xPos}, {yPos}
        dw  4, 4, {e}, 0, 0, {e}, {e}, 4
    }

    _row(0x1000, 0x2000, 1)
    _row(0x2000, 0x2000, 0)
    _row(0x2000, 0x1000, 1)
    _row(0x2000, 0x1010, 2)
    _row(0x2000, 0x1000, 2) // Still inside inactive window
    _row(0x3000, 0x3000, 0)

    constant rowSize((2 + 8) * 2)
    constant size(pc() - MoveCameraTable)
}
}

}

// vim: ft=asm ts=4 sw=4 et:

