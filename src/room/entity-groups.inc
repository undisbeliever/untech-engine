// room/entity-groups.inc
// ======================
//
// Entity Group routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Room {
namespace EntityGroups {


// Initialize the Room Entity Groups subsystem
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
macro _Init() {
    assert16a()
    assert16i()

    assert(EntityGroups.spawnedGroupsBitfield + 1 == EntityGroups.groupAddrs)
    stz.w   EntityGroups.spawnedGroupsBitfield
}



// Load and verify the EntityGroup data
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X = address of the entity group data
// OUTPUT: X = address after entity group data
macro _Load() {
    assert16a()
    assert16i()

    // Confirm this is an Entity Group data block
    lda.w   0,x
    and.w   #0xff
    cmp.w   #Format.EntityGroup.IDENTIFIER
    beq     +
    MissingEntityGroupIdentifier:
        break(INVALID_ROOM_DATA)
    +
    inx


    stz.b   _count

    // Y holds nGroups
    ldy.w   #0

    Loop:
        // Read nEntities
        // MUST NOT SET CARRY
        assert(Format.EntityGroup.nEntities == 0)
        lda.w   Format.EntityGroup.nEntities,x
        and.w   #0xff
        beq     EndLoop


        // Advance entity count
        // carry clear
        adc.b   _count
        sta.b   _count


        // Store entity Group address
        txa
        sta.w   EntityGroups.groupAddrs,y
        iny
        iny
        cpy.w   #MAX_ENTITY_GROUPS * 2 - 1
        bcc     +
        TooManyEntityGroups:
            break(INVALID_ROOM_DATA)
        +


        // Advance pointer
        // X += nEntities * Format.EntityGroup.Entities.size + Format.EntityGroup.HEADER_SIZE
        stx.b   _tmpX

        assert(Format.EntityGroup.HEADER_SIZE == 1)
        assert((1 << 2) + 1 == Format.EntityGroup.Entities.size)
        lda.w   Format.EntityGroup.nEntities,x
        and.w   #0xff
        sta.b   _tmp
        asl
        asl
        // carry clear
        adc.b   _tmp
        sec         // + 1
        adc.b   _tmpX
        tax


        bra     Loop
    EndLoop:

    assert(Format.EntityGroup.HEADER_SIZE)
    inx


    // Set address of unused groups to 0
    lda.w   #0
    FillLoop:
        cpy.w   #MAX_ENTITY_GROUPS * 2
        bcs     EndFillLoop

        sta.w   EntityGroups.groupAddrs,y
        iny
        iny
        bra     FillLoop
    EndFillLoop:


    // Set minimum entity InstanceId for non room spawned entities
    assert(Entity.N_ENTITIES > 0)
    lda.b   _count
    cmp.w   #Entity.N_ENTITIES + 1
    bcc     +
    InvalidEntityCount:
        break(INVALID_ROOM_DATA)
    +
    Entity._InstanceIdTable._SetNumberOfRoomEntities()
}



// Spawn_Entity_Group scripting background task
namespace Spawn_Entity_Group__BgTask {
    // Number of entities to spawn
    // (byte counter)
    constant _counter                   = Scripting.BackgroundTasks.state + 0

    // The Instance Id of the next entity to spawn
    // (byte)
    constant _currentEntityInstanceId   = Scripting.BackgroundTasks.state + 1

    // word address into Room.Format.EntityGroup.Entities
    // (only set if _counter > 0)
    constant _entityPos                 = Scripting.BackgroundTasks.state + 2

    assert(Scripting.BackgroundTasks.state.size > 4)



    // BG Task setup routine
    //
    // REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
    // INPUT: Y - script program counter before argument
    // OUTPUT: Y - script program counter after argument
    a8()
    i16()
    code()
    function SetupBgTask___EntityGroup {
        constant _loopCounter = utDpTmp0

        // First argument: entity group
        lda.w   Scripting.scriptData,y
        iny
        sty.b   Scripting.programCounter

        // If _counter is zero then no entities will be spawned
        stz.w   _counter


        cmp.b   #Room.MAX_ENTITY_GROUPS
        bcs     Return

        sta.b   _loopCounter


        // Do not spawn entity group if it has already been spawned
        // Also mark entity group as spawned
        tax
        lda.l   Scripting.Bytecode.SetFlagBits,x
        tsb.w   Room.EntityGroups.spawnedGroupsBitfield
        bne     Return


        // The first entity MUST NOT have an instanceId of 0
        lda.b   #Entity.INSTANCE_ID_INCREMENT
        sta.w   _currentEntityInstanceId

        ldx.w   #0

        // Increment _currentEntityInstanceId
        // ::TODO optimize later::
        Loop:
            dec.b   _loopCounter
            bmi     FoundGroup

            ldy.w   Room.EntityGroups.groupAddrs,x
            beq     EmptyGroup

                // Group exists, advance _currentEntityInstanceId
                assert(Entity.INSTANCE_ID_INCREMENT == 2)
                lda.w   Room.Format.EntityGroup.nEntities,y
                asl
                clc
                adc.w   _currentEntityInstanceId
                sta.w   _currentEntityInstanceId

        EmptyGroup:
            inx
            inx
            cpx.w   #Room.EntityGroups.N_ENTITY_GROUPS * 2
            bcc     Loop


    FoundGroup:
        // X = index into groupAddrs for the selected entity group
        ldy.w   Room.EntityGroups.groupAddrs,x
        sty.w   _entityPos

        lda.w   Room.Format.EntityGroup.nEntities,y
        sta.w   _counter

    Return:
        ldy.b   Scripting.programCounter
        rts
    }


    code()
    namespace ProcessBgTask {
        au()
        iu()
        EntityPoolEmpty:
            rep     #$30
            sep     #$20
        a16()
        i16()
            Warnings.Warn(ENTITY_POOL_EMPTY)

        NoEntitiesToSpawn:
            // End task
            sec
            rts
    }


    // BG Task process routine
    //
    // REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e
    // OUTPUT: carry set if the task has completed
    a8()
    i16()
    code()
    function ProcessBgTask {
        lda.w   _counter
        beq     NoEntitiesToSpawn


        rep     #$30
    a16()

        ldy.w   _entityPos
        Entity._SpawnRoomEntity()
        // NOTE: Entity._SpawnRoomEntity() will increment _currentEntityInstanceId

        // DP = entity spawned
        // ::TODO move into _SpawnRoomEntity::
        lda.w   #0
        tcd


        lda.w   _entityPos
        clc
        adc.w   #Room.Format.EntityGroup.Entities.size
        sta.w   _entityPos

        sep     #$30
    a8()

        dec.w   _counter
        beq     TaskComplete

        clc
        rts


    TaskComplete:
        sec
        rts
    }
}


}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

