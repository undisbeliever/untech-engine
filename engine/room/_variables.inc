// resources/_variables.inc
// ========================
//
// Variables used by the resources subsystem.
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Room {

// Room data
//
// Holds the decompressed room data
//
// (wram 7E data block)
allocate(roomData, wram7e, Project.ROOM_DATA_SIZE)
constant roomData.size = Project.ROOM_DATA_SIZE

assert(roomData.size >= 1024)


// Byte flag to request a room change
//
// MSB set if the script or engine requests the game loop changes the room.
//
// Project MUST check this flag regularly and call `Room.LoadRoom` when the MSB flag is set.
//
// (byte flag)
allocate(roomChangeRequested, wram7e, 1)

constant roomChangeRequested.LOAD_NEW_ROOM_FLAG = 0x80


namespace Entrances {
    // Word address of the room entrances data
    //
    // (word addr)
    allocate(dataAddr, wram7e, 2)
}


namespace EntityGroups {
    // Bitfield of entity groups that have been spawned
    //
    // (byte bitfield)
    allocate(spawnedGroupsBitfield, wram7e, 1)


    // Word address of the room entity list
    //
    // (word addr)
    allocate(roomEntitiesAddr, wram7e, 2)
}

namespace ScriptTriggers {
    constant N_SCRIPT_TRIGGERS = 16

    // The index of the last item in the Script Triggers SoA.
    //
    // This value is negative if there are no triggers in the room
    //
    // (byte index)
    allocate(lastTriggerIndex, wram7e, 1)


    // The index of the script trigger encountered in the previous frame.
    //
    // This value is negative if there is no previous script trigger.
    //
    // (byte index into Script Triggers SOA)
    allocate(prevTriggerIndex, wram7e, 1)


    // The tile index (within `room.mapData`) of the script-trigger tile
    // the player touched.
    //
    // If this value is negative then the player has not touched a room
    // script-trigger tile.
    //
    // This value should not be set if it is positive.
    //
    // This value is reset to a negative value on every `Room.Process()` call.
    //
    // (word index into `room.mapData`)
    allocate(playerTileIndex, wram7e, 2)


    // Calculated values within the Script Triggers SoA
    namespace Triggers {
        // The top y-tile position of the script trigger.
        //
        // This array is calculated at room load.
        //
        // (byte SoA element)
        allocate(top, wram7e, 2 * N_SCRIPT_TRIGGERS)

        // The bottom y-tile position of the script trigger.
        //
        // Equal to script trigger `aabb.y + aabb.height`.
        //
        // This array is calculated at room load.
        //
        // (byte SoA element)
        constant bottom = top + 1

        assertOnePage(top, 2 * N_SCRIPT_TRIGGERS)
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

