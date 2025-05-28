// SPDX-FileCopyrightText: © 2019 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2019 Marcus Rowe <undisbeliever@gmail.com>
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

namespace Entities {

namespace SpawnAndChangeList {
    buildFunctionTable(BaseEntityFunctionTable, SpawnAndChangeList)

    constant DELETE_DELAY = 2


    // IN: A = entity parameter = entity list id
    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        sep     #$20
    a8()
        sta.b   BaseEntity.listId

        rep     #$30
    a16()

        // always activate
        sec
        rts
    }


    a16()
    i16()
    code()
    function Process {
        // This entity spawns 3 entities:
        //  * DeleteAfterDelayEntity with listId = listId
        //  * DeleteAfterDelayEntity with lidtId = listId - 1
        //  * DeleteAfterDelayEntity with listId = listId + 1
        //
        // then changes its entity list to the previous list.

        lda.b   BaseEntity.listId
        and.w   #0xff
        jsr     SpawnDeleteEntity


        lda.b   BaseEntity.listId
        and.w   #0xff
        dec
        jsr     SpawnDeleteEntity


        lda.b   BaseEntity.listId
        and.w   #0xff
        inc
        jsr     SpawnDeleteEntity


        lda.b   BaseEntity.listId
        and.w   #0xff
        dec
        bpl     +
            lda.w   #Entity.N_ENTITY_LISTS - 1
        +
        jmp     Entity.ChangeEntityListIdAndGotoNextEntity
    }


    // IN: A = listId to spawn DeleteAfterDelayEntity in.
    a16()
    i16()
    code()
    function SpawnDeleteEntity {
        // ensure entity is spawned
        ldx.w   Camera.xPos
        stx.w   Entity.Spawn.xPos
        ldx.w   Camera.yPos
        stx.w   Entity.Spawn.yPos

        // wrap list id if necessary
        cmp.w   #EntityLists.EntityList_0
        bcs     +
            // A < EntityList_0
            lda.w   #EntityLists.EntityList_9
        +
        cmp.w   #EntityLists.EntityList_9
        bcc     +
            // A >= EntityList_9
            lda.w   #EntityLists.EntityList_0
        +

        clc
        adc.w   #Project.EntityIds.DeleteAfterDelay_0
        ldy.w   #DELETE_DELAY
        jmp     Entity.Spawn
    }


    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    HitboxCollision:
    HurtboxCollision:
    ShieldCollision:
    function NULL {
        rts
    }
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

