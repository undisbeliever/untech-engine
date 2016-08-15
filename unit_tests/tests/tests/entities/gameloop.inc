
scope EntitiesTest {

a16()
i16()
code()
Test.add("Entities.GameLoop (ChangeList)")
scope GameLoop_ChangeList: {
constant tmp_index(Test.tmp)
constant tmp_listId(Test.tmp + 2)
constant tmp_test(Test.tmp + 4)

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

        ldx.w   #0
        ldy.w   #0
        ListSizeLoop:
            stx.w   tmp_index
            sty.w   tmp_listId

            jsr     _ListLength_Y

            ldx.w   tmp_index
            cmp.l   ExpectedSizes,x
            bne     Fail

            ldy.w   tmp_listId
            iny

            inx
            inx
            cpx.w   #ExpectedSizes.size
            bcc     ListSizeLoop

        dec.w   tmp_test
        bne     TestLoop

    sec
    rts

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
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 4
    bne     Fail

    // frame 1 - 1 list of entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 3
    bne     Fail

    // frame 2 - 2 lists of entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 1
    bne     Fail

    // frame 3 - no entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 1
    bne     Fail

    // frame 4 - 1 list of entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 0
    bne     Fail

    // frame 5 - no entities deleted
    jsr     Entities.ProcessGameLoop
    jsr     _FreeListLength
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 0
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

