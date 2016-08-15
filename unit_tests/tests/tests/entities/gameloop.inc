
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

}

// vim: ft=asm ts=4 sw=4 et:

