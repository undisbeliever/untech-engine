
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


// OUT: A = number of entities in free list
scope _FreeListLength: {
    ldx.w   Entities.lists.free
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

