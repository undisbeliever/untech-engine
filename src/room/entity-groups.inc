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

    assert(EntityGroups.spawnedGroupsBitfield + 1 == EntityGroups.roomEntitiesAddr)
    stz.w   EntityGroups.spawnedGroupsBitfield
}



// Load and verify the Entity Group data
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X = address of the entity group data
// OUTPUT: X = address after entity group data
macro _Load() {
    assert16a()
    assert16i()

    sep     #$20
a8()

    // Confirm this is an Room Entity data block
    lda.w   0,x
    cmp.b   #Format.RoomEntity.IDENTIFIER
    beq     +
    MissingEntityGroupIdentifier:
        break(INVALID_ROOM_DATA)
    +
    inx

    stx.w   EntityGroups.roomEntitiesAddr


    sep     #$30
a8()
i8()

    // Check entityGroups array is valid and find the last entity group

    // X holds index into entityGroups
    // Y = previous entityGroups value
    ldy.b   #0
    ldx.b   #0

    Loop:
        lda.w   Format.Header.entityGroups,x
        beq     EmptyEntityGroup

        // ::SHOULDO add MAX_N_ROOM_ENTITIES constant::
        cmp.b   #Entity.N_ENTITIES
        bcc     +
        InvalidEntityGroup:
            break(INVALID_ROOM_DATA)
        +

        tay

        inx
        cpx.b   #N_ENTITY_GROUPS
        bcc     Loop


    // All entity groups exist, do not check for empty groups
    bra     SkipEmptyCheck


    EmptyEntityGroup:
        // There is at least one empty entity group
        // Confirm the rest of the entity groups are empty
        CheckEmptyLoop:
            lda.w   Format.Header.entityGroups,x
            beq     +
            ExpectedEmptyEntityGroup:
                break(INVALID_ROOM_DATA)
            +

            inx
            cpx.b   #N_ENTITY_GROUPS
            bcc     CheckEmptyLoop


SkipEmptyCheck:
    // Y = number of entities in the entity list

    rep     #$30
a16()
i16()
    tya
    // Ah = 0
    sta.b   _tmp

    Entity._InstanceIdTable._SetNumberOfRoomEntities()


    // Calculate the end address of the room entities list
    // X = numberRoomEntities + sizeof(Entities) + roomEntitiesAddr
    assert(Room.Format.RoomEntity.size == 5)
    lda.b   _tmp
    asl             // * 2
    asl             // * 4
    clc
    adc.b   _tmp    // + 1
    clc
    adc.w   Room.EntityGroups.roomEntitiesAddr
    tax
}



// Spawn_Entity_Group scripting background task
namespace Spawn_Entity_Group__BgTask {
    // Word address into the Room.Format.RoomEntity struct
    // (Set to 0 if entity group already spawned or entity group invalid)
    // (word addr)
    constant _entityAddr                 = Scripting.BackgroundTasks.state + 0

    // Entity instance Id of the next entity to spawn.
    // MUST NOT BE 0, the first instanceId is `Entity.FIRST_INSTANCE_ID`.
    // (byte)
    constant _currentEntityInstanceId   = Scripting.BackgroundTasks.state + 2

    // The one-past-the-end entity instance id for the entity group.
    // Used to determine when to stop spawning entities.
    // (byte)
    constant _entityInstanceIdEnd       = Scripting.BackgroundTasks.state + 3

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
    constant _tmp = utDpTmp0

        // First argument: entity group
        lda.w   Scripting.scriptData,y
        iny
        sty.b   Scripting.programCounter

        cmp.b   #Room.N_ENTITY_GROUPS
        bcs     DoNotSpawn


        // Do not spawn entity group if it has already been spawned
        // Also mark entity group as spawned
        assert(Room.N_ENTITY_GROUPS == 8)
        assert(Room.EntityGroups.spawnedGroupsBitfield + 1 == Room.EntityGroups.roomEntitiesAddr)
        // Ah = 0
        tax
        lda.l   Scripting.Bytecode.SetFlagBits,x
        tsb.w   Room.EntityGroups.spawnedGroupsBitfield
        bne     DoNotSpawn


        // Check entityGroup exists
        lda.w   Room.Format.Header.entityGroups,x
        beq     DoNotSpawn

            // _entityInstanceIdEnd = (end_index + 1) * INSTANCE_ID_INCREMENT
            assert(Entity.INSTANCE_ID_INCREMENT == 2)
            assert(Entity.FIRST_INSTANCE_ID == Entity.INSTANCE_ID_INCREMENT)
            inc
            asl
            sta.w   _entityInstanceIdEnd


            // Get the end-entity index of the previous entity group
            // (set A to 0 if entity group == 0)
            txa
            beq     +
                lda.w   Room.Format.Header.entityGroups - 1,x
            +
            sta.b   _tmp


            // _currentEntityInstanceId = (start_index + 1) * INSTANCE_ID_INCREMENT
            assert(Entity.INSTANCE_ID_INCREMENT == 2)
            assert(Entity.FIRST_INSTANCE_ID == Entity.INSTANCE_ID_INCREMENT)
            inc
            asl
            sta.w   _currentEntityInstanceId


            // _entityAddr = start_index * sizeof(Entities) + roomEntitiesAddr
            assert(Room.Format.RoomEntity.size == 5)
            lda.b   _tmp
            asl             // * 2
            asl             // * 4
            clc
            adc.b   _tmp    // + 1

            rep     #$31
        a16()
            // carry clear
            adc.w   Room.EntityGroups.roomEntitiesAddr
            sta.w   _entityAddr


            sep     #$20
        a8()

            ldy.b   Scripting.programCounter

            rts


    a8()
    DoNotSpawn:
        ldx.w   #0
        stx.w   _entityAddr

        ldy.b   Scripting.programCounter

        rts
    }



    code()
    namespace ProcessBgTask {
        a16()
        i16()
        EntityPoolEmpty:
            sep     #$20
        a8()
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
        ldy.w   _entityAddr
        beq     NoEntitiesToSpawn

        rep     #$30
    a16()

        Entity._SpawnRoomEntity()

        lda.w   _entityAddr
        clc
        adc.w   #Room.Format.RoomEntity.size
        sta.w   _entityAddr


        sep     #$21
    a8()
        lda.w   _currentEntityInstanceId
        // carry set
        adc.b   #Entity.INSTANCE_ID_INCREMENT - 1
        sta.w   _currentEntityInstanceId


        // BG Task is completed if _currentEntityInstanceId >= _entityInstanceIdEnd
        cmp.w   _entityInstanceIdEnd
        rts
    }
}


}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

