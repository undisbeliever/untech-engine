
scope Entity {

// Initializes the entity pool
//
// This MUST be called before using this module
//
// This module will also initialize:
//   * DMA module
//   * MetaSprite module
//   * Camera module
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
a16()
i16()
code()
scope Init: {
    jsr     Dma.Init
    jsr     MetaSprite.Init
    jsr     Camera.Init

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
    ldx.w   #lists.LAST - lists.deactivated
    -
        stz.w   lists.deactivated,x
        dex
        dex
        bpl     -

    rts
}



// Clear the TryToActivateAgain entity list
//
// Should be called at the start of each level
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// MODIFIES: DP
a16()
i16()
code()
scope ClearTryToActivateAgainList: {
    lda.w   lists.tryToActivateAgain
    beq     Return

    Loop:
        tcd

        ldx.b   BaseEntity.romPtr
        lda.l   BaseEntityData.functionTable,x
        tax
        jsr     (BaseEntityFunctionTable.Destructor,x)

        lda.b   BaseEntity.next
        bne     Loop


    // Move the tryToActivateAgain list into the free list

    lda.w   lists.free
    sta.b   BaseEntity.next

    lda.w   lists.tryToActivateAgain
    sta.w   lists.free

    stz.w   lists.tryToActivateAgain

Return:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

