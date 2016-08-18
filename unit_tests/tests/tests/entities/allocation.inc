
scope EntitiesTest {

a16()
i16()
code()
Test.add("Entities.Init")
scope Init: {
    jsr     Entities.Init

    jsr     _FreeListLength
    cmp     #Entities.N_ENTITIES
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
    cmp     #Entities.N_ENTITIES - 1
    bne     Fail

    // Test first list length
    jsr     _FirstListLength
    cmp     #1
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
    cmp     #Entities.N_ENTITIES
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
    cmp     #Entities.N_ENTITIES - Entities.lists.N_LISTS * 2
    bne     Fail

    // Test each list length
    ldy     #0
    ListLoop:
        phy
            jsr     _ListLength_Y
        ply

        cmp     #2
        bne     Fail

        iny
        cpy.w   #Entities.lists.N_LISTS
        bcc     ListLoop

    jmp     _TestCorrectListsActive

Fail:
    clc
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

