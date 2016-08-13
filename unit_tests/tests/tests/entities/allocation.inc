
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

    lda.w   BlankEntity.EntityId
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

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("Entities.Spawn Overflow")
scope Spawn_Overflow: {
    jsr     Entities.Init

    stz.w   BlankEntity.parameterSum

    ldx.w   #Entities.N_ENTITIES

    Loop:
        phx

        lda.w   BlankEntity.EntityId
        ldy.w   #5
        jsr     Entities.Spawn
        bcc     Fail

        plx
        dex
        bne     Loop


    // Test cannot make new entity
    lda.w   BlankEntity.EntityId
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

    sec
    rts

Fail:
    clc
    rts
}


// OUT: A = number of entities in free list
scope _FreeListLength: {
    ldx.w   Entities.lists.free
    bra     __ListLength
}

// OUT: A = number of entities in player list
scope _FirstListLength: {
    ldx.w   Entities.lists.FIRST
    bra     __ListLength
}

// IN: X = first entity address
// OUT: A = number of entities in list
a16()
i16()
code()
scope __ListLength: {
    lda.w   #0

    cpx.w   #0
    beq     EndLoop

    Loop:
        inc

        txy
        ldx.w   BaseEntity.next,y
        bne     Loop

EndLoop:
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

