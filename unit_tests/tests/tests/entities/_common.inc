
scope EntitiesTest {

// OUT: A = number of entities in free list
a16()
i16()
code()
scope _FreeListLength: {
    ldx.w   Entities.lists.free
    bra     __ListLength
}

// OUT: A = number of entities in deactivated list
a16()
i16()
code()
scope _DeactivatedListLength: {
    ldx.w   Entities.lists.deactivated
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



// OUT: C set if deactivated lists have no metasprite data
//               AND all other lists have metasprite daata
a16()
i16()
code()
scope _TestCorrectListsActive: {
allocateTmpWord(tmp_index)

    // test deactivated entity is not active
    lda.w   Entities.lists.deactivated
    InactiveLoop:
        tcd

        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
        bne     Fail

        lda.b   BaseEntity.next
        bne     InactiveLoop


    // test all other entities are active
    scope ActiveListTest: {
        ldx.w   #Entities.lists.FIRST
        Loop:
            stx.w   tmp_index

            lda.w   0,x
            beq     Skip
            EntityLoop:
                tcd

                lda.b   BaseEntity.MetaSprite.status
                and.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
                cmp.w   #MetaSprite.StatusFlags.paletteSet | MetaSprite.StatusFlags.vramSet
                bne     Fail

                lda.b   BaseEntity.next
                bne     EntityLoop
            Skip:

            ldx.w   tmp_index
            inx
            inx
            cpx.w   #Entities.lists.LAST + 1
            bcc     Loop
    }

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

