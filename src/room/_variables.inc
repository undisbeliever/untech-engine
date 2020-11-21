// resources/_variables.inc
// ========================
//
// Variables used by the resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Room {

// Room data
//
// Holds the decompressed room data
//
// (wram 7E data block)
allocate(roomData, wram7e, Project.ROOM_DATA_SIZE)
constant roomData.size = Project.ROOM_DATA_SIZE

assert(roomData.size >= 1024)


namespace Entrances {
    // Word address of the room entrances data
    //
    // (word addr)
    allocate(dataAddr, wram7e, 2)
}


namespace EntityGroups {
    constant N_ENTITY_GROUPS = 8

    // Word address of each entity group
    //
    // If the value is zero then there is no entity group
    //
    // (word pointer array)
    allocate(groupAddrs, wram7e, 2 * N_ENTITY_GROUPS)



    // Bitfield of entity groups to activate
    //
    // (byte bitfield)
    allocate(toActivateBitfield, wram7e, 1)

    // Bitfield of entity groups that have been spawned
    //
    // (byte bitfield)
    allocate(spawnedGroupsBitfield, wram7e, 1)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

