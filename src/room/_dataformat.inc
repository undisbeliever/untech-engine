// room/_dataformat.inc
// ====================
//
// Data format of the room subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Room {


// List of rooms
// (Long Addr Table)
constant RoomList = Project.RoomList
constant RoomList.count = Project.RoomList.count


// Maximum number of scripts per room.
constant N_ROOM_SCRIPTS = 16

// The maximum number of entity groups in a room
constant N_ENTITY_GROUPS = 8

// The maximum number of script triggers in a room
constant N_SCRIPT_TRIGGERS = 16



namespace Format {
    if Room.ROOM_FORMAT_VERSION != 6 {
        error "Invalid untech-compiler Room Format"
    }

    namespace Header {
        // Stored in RAM
        struct(roomData)
            // The width of the map in tiles
            // (byte)
            field(width, 1)

            // The height of the map in tiles
            //
            // `bytesPerColumn` is set depending on this value:
            //
            //      if height <= MAP_HEIGHT_SMALL:
            //          bytesPerColumn = MAP_HEIGHT_SMALL
            //      else:
            //          bytesPerColumn = MAP_HEIGHT_LARGE
            //
            // (byte)
            field(height, 1)

            // The sceneId for this room
            // (byte)
            field(sceneId, 1)

            // Number of Room Entrances for this room
            field(nEntrances, 1)

            // The program counter of the room's startup script.
            //
            // This script will be started on room load and the gameloop will not start
            // until this script has finished execution.
            //
            // This value MUST NOT be null (0).
            //
            // (word index into `roomData`)
            field(startupScript, 2)

            // The initial program counter of each script in the map.
            // Each entry contains a index (within `roomData`) to the start of script.
            // If there is no script, the value is 0.
            //
            // (N_ROOM_SCRIPTS word indexes)
            field(scripts, 2 * N_ROOM_SCRIPTS)

            // The end-index within the `RoomEntity` struct for each entity group.
            //
            // Each value in this array is the one-past-the-end index within
            // the `RoomEntity` struct for each entity group.
            //
            // If the entity group is empty then the value is 0.
            //
            // The last non-zero value in this array is used to determine the
            // number of room-entities in the room.
            //
            // (byte array)
            field(entityGroups, N_ENTITY_GROUPS)


            // Room Script Triggers
            //
            // Structure of Arrays
            namespace ScriptTriggers {
                constant ARRAY_SIZE = 2 * N_SCRIPT_TRIGGERS

                constant LAST_INDEX = (N_SCRIPT_TRIGGERS - 1) * 2
                constant INDEX_INCREMENT = 2

                // Top-Left `mapData` tile index for the script trigger
                // (word index)
                field(topLeft, ARRAY_SIZE)

                // The byte after the tile index of the bottom-most right-most tile in the script trigger.
                //
                // If the script trigger exists, this value is equal to
                // `tilePos(x + width - 1, y + height - 1) + 1`.
                //
                // If this value is negative then the scriptTrigger does not exist.
                //
                // (word index)
                field(bottomRight, ARRAY_SIZE)

                // The script to activate when the trigger is triggered.
                //
                //   osssssss
                //      o = activate script once flag
                //      s = script id
                //
                // This value will be set to an invalid script id after a "once" trigger has been activated.
                //
                // (byte with 1 byte padding)
                field(script, ARRAY_SIZE)

                constant script.ONCE_FLAG        = 0b10000000
                constant script.SCRIPT_ID_MASK   = 0b01111111


                // ::TODO put some data in the second byte::


                assertOnePage(topLeft,     ARRAY_SIZE)
                assertOnePage(bottomRight, ARRAY_SIZE)
                assertOnePage(script,      ARRAY_SIZE)
            }
        endstruct()
    }

    // Immediately following the header is the map data.


    // The map data
    //
    // (byte grid array column-major order, exactly width * bytesPerColumn bytes in size)
    constant mapData = roomData + Header.size


    // Room Entrance data
    //
    // Repeated `Header.nEntrances` times
    namespace RoomEntrance {
        constant IDENTIFIER = 0x4e // "N"

        struct()
            // The position of the player
            //
            // xxxxxxxx xxxxyyyy yyyyyyyy
            //
            //      xxxxxxxxxxxx = Player X position (offset by MetaTiles.map.LEFT)
            //      yyyyyyyyyyyy = Player Y position (offset by MetaTiles.map.TOP)
            //
            // (2x 12 bit position)
            field(position, 3)

            constant POSITION_MASK  = 0x0fff
            constant POSITION_SHIFT = 12

            // The orientation of the player
            // (Orientation enum byte)
            field(orientation, 1)
        endstruct()


        // The orientation of the player at the start of the room
        namespace Orientation {
            createEnum()
                enum(DOWN_RIGHT)
                enum(DOWN_LEFT)
                enum(UP_RIGHT)
                enum(UP_LEFT)
            endEnum()

        }
        assertPowerOfTwo(Orientation.__ENUM__.count)
        constant ORIENTATION_MASK = Orientation.__ENUM__.count - 1
    }


    // Room entity data.
    //
    // The entity-group that each room entity belongs to is determined
    // by the `Header.entityGroups` array in the room header.
    //
    // The number of entities in this data block is equal to the last
    // non-zero value in the `Header.entityGroups` array.
    namespace RoomEntity {
        // Byte used to identify the start of the room-entities block
        constant IDENTIFIER = 0x45 // "E"

        struct()
            // Entity Data Id
            // (byte)
            field(entityDataId, 1)

            // The position of the entity
            //
            // xxxxxxxx xxxxyyyy yyyyyyyy
            //
            //      xxxxxxxxxxxx = Entity X position (offset by POSITION_X_OFFSET)
            //      yyyyyyyyyyyy = Entity Y position (offset by POSITION_Y_OFFSET)
            //
            // (2x 12 bit position)
            field(position, 3)

            constant POSITION_MASK  = 0x0fff
            constant POSITION_SHIFT = 12

            constant POSITION_X_OFFSET = MetaTiles.map.LEFT
            constant POSITION_Y_OFFSET = MetaTiles.map.TOP  - 256

            // Entity spawn parameter
            // (byte)
            field(parameter, 1)

        endstruct()
    }


    // Room Script data
    //
    // Immediately following this struct is the bytecode script data.
    namespace RoomScripts {
        // Byte used to identify the start of an room script data block
        constant IDENTIFIER = 0x53 // "S"

        struct()
            // Size of the room script data in bytes
            // (word)
            field(blockSize, 2)
        endstruct()
    }


    // Room footer
    //
    // Used to verify all blocks in a room have been read successfully
    constant FOOTER = 0x21444e45 // "END!"
    constant FOOTER_SIZE = 4
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

