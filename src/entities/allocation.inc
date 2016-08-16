
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

scope Spawn {
ReturnFalse:
    clc
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


        // Insert entity into the appropriate list

        scope ActiveTest {
            // entity is active if:
            //   entity list only contains active entities
            //   OR entity is inside active window

            // get initial list index
            assert(lists.N_LISTS & (lists.N_LISTS - 1) == 0)
            ldx.b   BaseEntity.romPtr
            lda.l   BaseEntityData.initialList,x
            and.w   #(lists.N_LISTS - 1) * 2
            tax

            lda.l   EntityAlwaysActiveList,x
            bne     IsActive

            Camera.BranchEntityOutsideActiveWindow(IsInactive)

            IsActive:
                // entity is activate.
                // Insert into its active list

                phx
                    jsr     MetaSprite.Activate
                plx

                // x = list index
                lda.w   lists.FIRST,x
                sta.b   BaseEntity.next
                tdc
                sta.w   lists.FIRST,x

                bra     EndIf

            IsInactive:
                // entity is inactivate.
                // Insert into deactivated list, remembering the list
                // address for when it is reactived

                // x = list index
                txa
                clc
                adc.w   #lists.FIRST
                sta.b   BaseEntity.functionTable

                lda.w   lists.deactivated
                sta.b   BaseEntity.next
                tdc
                sta.w   lists.deactivated

            EndIf:

            // A = DP
        }
        tay
    pld

    sec
    rts

rodata(rom0)
// one if the entity list is always active
// zero if entity list can be deactivated
scope EntityAlwaysActiveList: {
    dw  1       // players
    dw  1       // bosses
    dw  0       // enemies
    dw  1       // playerProjectiles
    dw  1       // enemyProjectiles
    dw  0       // npcs
    dw  0       // platforms
    dw  1       // particles

    assert((pc() - EntityAlwaysActiveList) == lists.N_LISTS * 2)
}

code()
}

}

// vim: ft=asm ts=4 sw=4 et:

