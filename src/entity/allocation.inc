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
// MODIFIES: Sets ForceBlank
a16()
i16()
code()
function Init {
    sep     #$20
a8()
    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB = REGISTER_DB

    jsr     ForceBlank

    // Dirty all entity memory
    Dma.ForceBlank.DirtyWram(entityPool, ENTITY_SIZE * N_ENTITIES)

    plb
    // DB = 0x7e

    rep     #$30
a16()

    jsr     Dma.Init
    jsr     MetaSprite.Init
    jsr     Camera.Init
    jsr     _UpdateActiveWindows


    lda.w   #0
    tcd

    // create and chain the free linked list

    assert(N_ENTITIES > 2)

    ldx.w   #entityPool
    stx.w   lists.free

    lda.w   #entityPool + ENTITY_SIZE

    Loop:
        sta.b   BaseEntity.next,x

        stz.b   BaseEntity.functionTable,x
        stz.b   BaseEntity.MetaSprite.status,x

        tax

        clc
        adc.w   #ENTITY_SIZE
        cmp.w   #entityPool + ENTITY_SIZE * N_ENTITIES
        bcc     Loop

    stz.w   BaseEntity.next + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)
    stz.w   BaseEntity.functionTable + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)
    stz.w   BaseEntity.MetaSprite.status + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)


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

        jsr     __EntityDestructor

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

