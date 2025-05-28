// entity/allocation.inc
// =====================
//
// Entity pool initialization routines.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


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

    jsl     ForceBlank__far

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
    stx.w   specialLists.free


    ldy.w   #InvalidEntityFunctionTable
    lda.w   #entityPool + ENTITY_SIZE

    Loop:
        sta.b   BaseEntity.next,x
        stz.b   BaseEntity.instanceId,x
        stz.b   BaseEntity.MetaSprite.status,x

        // Y = #InvalidEntityFunctionTable
        sty.b   BaseEntity.functionTable,x

        tax

        clc
        adc.w   #ENTITY_SIZE
        cmp.w   #entityPool + ENTITY_SIZE * N_ENTITIES
        bcc     Loop

    stz.w   BaseEntity.next + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)
    stz.w   BaseEntity.instanceId + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)
    stz.w   BaseEntity.MetaSprite.status + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)

    sty.w   BaseEntity.functionTable + entityPool + ENTITY_SIZE * (N_ENTITIES - 1)


    // set lastFreeEntity to the last entity in the free list
    lda.w   #entityPool + ENTITY_SIZE * (N_ENTITIES - 1)
    sta.w   lastFreeEntity


    // clear the other lists
    // free list is the last one

    // Ensure free list is the last one
    assert(specialLists.deactivated < specialLists.free)
    assert(specialLists.activateNextFrame < specialLists.free)
    assert(lists.LAST < specialLists.free)

    ldx.w   #specialLists.FREE_INDEX - 2
    -
        stz.w   lists.FIRST,x
        dex
        dex
        bpl     -


    Player._Init()

    _InstanceIdTable._Init()

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

