
scope Entities {

// Initializes the Entities pool
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


// Spawns a new entity and inserts it into its default list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: A - Entity Id
//        Y - entity parameter (passed to entity init function)
//        Create.xPos - entity x Position (word)
//        Create.yPos - entity y Position (word)
//
// OUTPUT: Y - address of new entity
//         C - set if entity created successfully
code()
a16()
i16()
scope Spawn: {

allocateTmpWord(xPos)
allocateTmpWord(yPos)

    // ::TODO limit entity Id
    asl
    tax

    lda.w   lists.free
    beq     ReturnFalse

    phd
        phy

        // Remove Entity from free list

        tcd
        lda.b   BaseEntity.next
        sta.w   lists.free


        // Get Entity ROM Data

        lda.l   Entities.Data.List,x
        tax
        stx.b   BaseEntity.romPtr

        // X = entity ROM data address

        // Insert Entity into initial List

        assert(lists.N_LISTS & (lists.N_LISTS - 1) == 0)
        lda.l   BaseEntityData.initialList,x
        and.w   #(lists.N_LISTS - 1) * 2
        tay

        lda.w   lists.FIRST,y
        sta.b   BaseEntity.next
        tdc
        sta.w   lists.FIRST,y


        // Setup BaseEntity

        lda.l   BaseEntityData.functionTable,x
        sta.b   BaseEntity.functionTable

        lda.w   xPos
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos

        lda.w   yPos
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos

        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2
        stz.b   BaseEntity.xVecl + 4
        stz.b   BaseEntity.xVecl + 6


        lda.l   BaseEntityData.defaultPalette,x
        tay
        lda.l   BaseEntityData.frameSetId,x

        MetaSprite._InitEntity()


        // Call Entity Init function
        ldx.b   BaseEntity.functionTable
        pla
        jsr     (BaseEntityFunctionTable.Init,x)


        // ::TODO activate if inside active window::

        tdc
        tay
    pld

    sec
    rts

ReturnFalse:
    clc
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

