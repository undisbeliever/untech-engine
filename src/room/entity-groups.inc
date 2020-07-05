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

    // Clear toActivateBitfield and spawnedGroupsBitfield
    assert(EntityGroups.toActivateBitfield + 1 == EntityGroups.spawnedGroupsBitfield)
    stz.w   EntityGroups.toActivateBitfield
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



// Spawns all unspawned entity groups.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function SpawnEntityGroups {
allocateTmpWord(_count)
allocateTmpWord(_currentEntityInstanceId)
constant _groupsToProcess = _currentEntityInstanceId + 1

    // NOTE: _SpawnRoomEntity modifies DP, must not use DP here

    // Confirm the `toActivateBitfield` and `unspawnedGroupsBitfield` are 8 bit
    assert(MAX_ENTITY_GROUPS == 8)
    assert(EntityGroups.toActivateBitfield + 1 == EntityGroups.spawnedGroupsBitfield)

    sep     #$20
a8()

    // Find groups to spawn
    lda.w   spawnedGroupsBitfield
    eor.b   #0xff
    and.w   toActivateBitfield
    bne     +
        jmp     Return
    +

    sta.w   _groupsToProcess

    // Mark _groupsToProcess groups as spawned
    ora.w   spawnedGroupsBitfield
    sta.w   spawnedGroupsBitfield


    lda.b   #Entity.FIRST_INSTANCE_ID
    sta.w   _currentEntityInstanceId

    ldx.w   #-2

    // X = groupAddrs index
    a8()
    i16()
    GroupLoop:
        inx
        inx

        lsr.w   _groupsToProcess
        bcs     SpawnGroup
        beq     Return

        a8()
        SkipGroup:
            ldy.w   groupAddrs,x
            beq     GroupLoop

            // Group exists
            // advance _currentEntityInstanceId

            assert(Entity.INSTANCE_ID_INCREMENT == 2)
            lda.w   Format.EntityGroup.nEntities,y
            asl
            clc
            adc.w   _currentEntityInstanceId
            sta.w   _currentEntityInstanceId

            bra     GroupLoop


        EntityPoolEmpty:
            Warnings.Warn(ENTITY_POOL_EMPTY)

        au()
        Return:
            rep     #$30
        a16()
            lda.w   #0
            tcd
            rts


        a8()
        SpawnGroup:
            // Spawn all entities in group

            // Check group exists
            ldy.w   groupAddrs,x
            beq     GroupLoop

            // Y = entity group address
            rep     #$30
        a16()

            lda.w   Room.Format.EntityGroup.nEntities,y
            and.w   #0xff
            beq     GroupLoop
            sta.w   _count

            phx

            InnerLoop:
                phy
                    Entity._SpawnRoomEntity()

                    // NOTE: Entity._SpawnRoomEntity() will increment _currentEntityInstanceId

                    // DP = entity spawned
                pla
                clc
                adc.w   #Format.EntityGroup.Entities.size
                tay

                dec.w   _count
                beq     +
                    jmp     InnerLoop
                +

            sep     #$20
        a8()
            plx
            jmp     GroupLoop
}


}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

