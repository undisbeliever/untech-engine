// entity/allocation.inc
// =====================
//
// Entity pool initialization routines.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {

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
function Init {
    jsr     Dma.Init
    jsr     MetaSprite.Init
    jsr     Camera.Init
    jsr     _UpdateActiveWindows

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
    // free list is the last one

    // Ensure free list is the last one
    assert(lists.deactivated < lists.free)
    assert(lists.activateNextFrame < lists.free)
    assert(lists.LAST < lists.free)

    lda.w   lists.free
    ldx.w   #lists.free - lists.FIRST - 2
    -
        stz.w   lists.FIRST,x
        dex
        dex
        bpl     -

    rts
}



// Clear the activateNextFrame entity list
//
// Should be called at the start of each level
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function ClearActivateNextFrameList {
    lda.w   lists.activateNextFrame
    beq     Return

    Loop:
        tcd

        ldx.b   BaseEntity.romPtr
        lda.l   BaseEntityData.functionTable,x
        tax
        jsr     (BaseEntityFunctionTable.Destructor,x)

        lda.b   BaseEntity.next
        bne     Loop


    // Move the activateNextFrame list into the free list

    lda.w   lists.free
    sta.b   BaseEntity.next

    lda.w   lists.activateNextFrame
    sta.w   lists.free

    stz.w   lists.activateNextFrame


    lda.w   #0
    tcd

Return:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

