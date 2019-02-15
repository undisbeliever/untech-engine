// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

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
        tay
        bpl     +
            // A < 0
            lda.w   #Entity.N_ENTITY_LISTS - 1
        +
        cmp.w   #Entity.N_ENTITY_LISTS
        bcc     +
            lda.w   #0
        +

        clc
        adc.w   #Entities.DeleteAfterDelay.EntityDataId_0
        ldy.w   #DELETE_DELAY
        jmp     Entity.Spawn
    }


    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
    function NULL {
        rts
    }


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  0               // initialListId
        db  0               // defaultPalette
        dw  0               // frameSetId

    rodata(EN_EntityDataList)
    constant EntityDataId = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

