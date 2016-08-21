
scope EntitiesTest {

a16()
i16()
code()
Test.add("Entities.Init")
scope Init: {
    jsr     Entities.Init

    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

a16()
i16()
code()
Test.add("Entities.Spawn")
scope Spawn: {
    jsr     Entities.Init

    stz.w   BlankEntity.parameterSum

    lda.w   #0x1234
    sta.w   Entities.Spawn.xPos
    lda.w   #0x5678
    sta.w   Entities.Spawn.yPos

    lda.w   BlankEntity.EntityId_0
    ldy.w   #0x1337
    jsr     Entities.Spawn
    bcc     Fail

    // test x/y pos
    lda.w   BaseEntity.xPos.px,y
    cmp.w   #0x1234
    bne     Fail

    lda.w   BaseEntity.yPos.px,y
    cmp.w   #0x5678
    bne     Fail

    // test parameter worked
    lda.w   BlankEntity.parameterSum
    cmp.w   #0x1337
    bne     Fail

    // test list decreased by one
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - 1
    bne     Fail

    // Test first list length
    jsr     _FirstListLength
    cmp.w   #1
    bne     Fail

    jmp     _TestCorrectListsActive

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Entities.Spawn (Overflow)")
scope Spawn_Overflow: {
constant tmp(Test.tmp)

    jsr     Entities.Init

    stz.w   BlankEntity.parameterSum

    ldx.w   #Entities.N_ENTITIES

    Loop:
        stx.w   tmp

        lda.w   BlankEntity.EntityId_0
        ldy.w   #5
        jsr     Entities.Spawn
        bcc     Fail

        ldx.w   tmp
        dex
        bne     Loop


    // Test cannot make new entity
    lda.w   BlankEntity.EntityId_0
    jsr     Entities.Spawn
    bcs     Fail

    // Test free list is 0
    jsr     _FreeListLength
    bne     Fail

    // Test parameter Sum is correct value
    lda.w   BlankEntity.parameterSum
    cmp.w   #5 * Entities.N_ENTITIES
    bne     Fail

    // Test first list length
    jsr     _FirstListLength
    cmp.w   #Entities.N_ENTITIES
    bne     Fail

    jmp     _TestCorrectListsActive

Fail:
    clc
    rts
}

a16()
i16()
code()
Test.add("Entities.Spawn (Multiple Lists)")
scope Spawn_MultipleLists: {
constant tmp(Test.tmp)

    jsr     Entities.Init

    stz.w   BlankEntity.parameterSum

    lda.w   #0x1000
    sta.w   Entities.Spawn.xPos
    sta.w   Entities.Spawn.yPos

    assert(Entities.N_ENTITIES >= Entities.lists.N_LISTS * 2)

    // spawn 2 entities per list
    lda.w   #BlankEntity.EntityId_0
    Loop:
        sta.w   tmp

        ldy.w   #1
        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        ldy.w   #1
        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #BlankEntity.EntityId_0 + Entities.lists.N_LISTS
        bcc     Loop


    // Test parameter Sum is correct value
    lda.w   BlankEntity.parameterSum
    cmp.w   #2 * Entities.lists.N_LISTS
    bne     Fail


    // test list decreased by N_LISTS
    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES - Entities.lists.N_LISTS * 2
    bne     Fail

    // Test each list length
    ldy.w   #0
    ListLoop:
        phy
            jsr     _ListLength_Y
        ply

        cmp.w   #2
        bne     Fail

        iny
        cpy.w   #Entities.lists.N_LISTS
        bcc     ListLoop

    jmp     _TestCorrectListsActive

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Entities.Spawn (Offscreen)")
scope Spawn_Offscreen: {
constant tmp_listId(Test.tmp)

constant ONSCREEN(0x1080)

    jsr     Entities.Init

    // spawn 2 entities on-screen and 4 entities off-screen per entity list

    assert(Entities.N_ENTITIES >= Entities.lists.N_LISTS * 6)

    lda.w   #BlankEntity.EntityId_0
    Loop:
        sta.w   tmp_listId

        macro spawn(xPos, yPos) {
            ldx.w   #{xPos}
            ldy.w   #{yPos}
            jsr     SpawnEntity_XY
            bcc     Fail
        }

        spawn(ONSCREEN, ONSCREEN)
        spawn(ONSCREEN + 0x200, ONSCREEN)
        spawn(ONSCREEN, ONSCREEN + 0x200)
        spawn(ONSCREEN, ONSCREEN)
        spawn(ONSCREEN - 0x200, ONSCREEN)
        spawn(ONSCREEN, ONSCREEN - 0x200)

        lda.w   tmp_listId
        inc
        cmp.w   #BlankEntity.EntityId_0 + Entities.lists.N_LISTS
        bcc     Loop



    // Verify list sizes

    jsr     _FreeListLength
    cmp.w   #ExpectedSizes.FREE
    bne     Fail

    jsr     _DeactivatedListLength
    cmp.w   #ExpectedSizes.DEACTIVATED
    bne     Fail


    ldx.w   #ExpectedSizes
    jsr     _TestListLengthTable
    bcc     Fail


    jmp     _TestCorrectListsActive

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
scope ExpectedSizes: {
// NOTE: projectiles are always spawned, even if offscreen
    dw  6
    dw  6
    dw  2
    dw  6
    dw  6
    dw  2
    dw  2
    dw  6

    constant DEACTIVATED(4 * 3)
    constant FREE(Entities.N_ENTITIES - 5 * 6 - 3 * 2 - DEACTIVATED)

    constant size(pc() - ExpectedSizes)
    assert(size / 2 == Entities.lists.N_LISTS)
}
}



a16()
i16()
code()
Test.add("Entities.ClearTryToActivateAgainList")
scope ClearTryToActivateAgainList: {
constant tmp(Test.tmp)

    jsr     Entities.Init

    // Prevent the entities from spawning
    jsr     WaitFrame
    stz.w   Dma.transfersLeft

    // Create many try to activate again entities

    lda.w   #0x1000
    sta.w   Entities.Spawn.xPos
    sta.w   Entities.Spawn.yPos

    assert(Entities.N_ENTITIES >= Entities.lists.N_LISTS)

    lda.w   #BlankEntity.EntityId_0
    Loop:
        sta.w   tmp

        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp
        inc
        cmp.w   #BlankEntity.EntityId_0 + Entities.lists.N_LISTS
        bcc     Loop


    jsr     _TryToActivateAgainListLength
    cmp.w   #Entities.lists.N_LISTS
    bne     Fail


    jsr     Entities.ClearTryToActivateAgainList


    jsr     _FreeListLength
    cmp.w   #Entities.N_ENTITIES
    bne     Fail

    jmp     _TestCorrectListsActive


Fail:
    clc
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

