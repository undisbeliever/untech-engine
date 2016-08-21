
scope EntitiesTest {

// OUT: A = number of entities in free list
// MODIFIES: DP
a16()
i16()
code()
scope _FreeListLength: {
    ldx.w   Entities.lists.free
    bra     __ListLength
}

// OUT: A = number of entities in deactivated list
// MODIFIES: DP
a16()
i16()
code()
scope _DeactivatedListLength: {
    ldx.w   Entities.lists.deactivated
    bra     __ListLength
}

// OUT: A = number of entities in the tryToActivateAgain list
// MODIFIES: DP
a16()
i16()
code()
scope _TryToActivateAgainListLength: {
    ldx.w   Entities.lists.tryToActivateAgain
    bra     __ListLength
}

// OUT: A = number of entities in player list
// MODIFIES: DP
a16()
i16()
code()
scope _FirstListLength: {
    ldx.w   Entities.lists.FIRST
    bra     __ListLength
}

// IN: Y = list id
// OUT: A = number of entities in player list
// MODIFIES: DP
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
// MODIFIES: DP
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
//               AND all other lists have metasprite data
// MODIFIES: DP
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


// Tests the entity list lengths against a
// list table.
// IN: X - address of list table (list of words, in rom0 data block)
// OUT: C set if entity list == table
// MODIFIES: DP
a16()
i16()
code()
scope _TestListLengthTable: {
allocateTmpWord(tmp_index)
allocateTmpWord(tmp_listId)

    ldy.w   #0
    ListSizeLoop:
        stx.w   tmp_index
        sty.w   tmp_listId

        jsr     _ListLength_Y

        ldx.w   tmp_index
        cmp.l   Table,x
        bne     Fail

        inx
        inx

        ldy.w   tmp_listId
        iny
        cpy.w   #Entities.lists.N_LISTS
        bcc     ListSizeLoop

    sec
    rts

Fail:
    clc
    rts

rodata(rom0)
    constant Table(pc() & 0xff0000)
}

}

// vim: ft=asm ts=4 sw=4 et:

