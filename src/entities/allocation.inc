
scope Entities {

// Initializes the Entities pool
//
// This MUST be called before using this module
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
a16()
i16()
code()
scope Init: {
    // create and chain the free linked list

    assert(N_ENTITIES > 2)

    ldx.w   #entityPool
    stx.w   lists.free

    lda.w   #entityPool + ENTITY_SIZE

    Loop:
        sta.w   BaseEntity.next,x
        stz.w   BaseEntity.functionTable,x

        tax

        clc
        adc.w   #ENTITY_SIZE
        cmp.w   #entityPool + ENTITY_SIZE * N_ENTITIES
        bcc     Loop

    stz.w   BaseEntity.next + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)


    // clear the other lists
    ldx.w   #lists._LAST - lists._FIRST - 2
    -
        stz.w   lists._LAST,x
        dex
        dex
        bpl     -

    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

