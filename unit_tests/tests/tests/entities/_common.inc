
scope EntitiesTest {

// OUT: A = number of entities in free list
a16()
i16()
code()
scope _FreeListLength: {
    ldx.w   Entities.lists.free
    bra     __ListLength
}

// OUT: A = number of entities in player list
a16()
i16()
code()
scope _FirstListLength: {
    ldx.w   Entities.lists.FIRST
    bra     __ListLength
}

// IN: Y = list id
// OUT: A = number of entities in player list
a16()
i16()
code()
scope _ListLength_Y: {
    tya
    asl
    tay
    ldx.w   Entities.lists.FIRST,y
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

