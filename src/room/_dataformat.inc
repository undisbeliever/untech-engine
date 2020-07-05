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


// The maximum number of entity groups in a room
constant MAX_ENTITY_GROUPS = 8
assert(EntityGroups.N_ENTITY_GROUPS == MAX_ENTITY_GROUPS)



namespace Format {
    if Room.ROOM_FORMAT_VERSION != 2 {
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


    // Entity Group data
    //
    // Each Entity Group contains a list of entities.  When an entity group is
    // activated, all entities within the group will be spawned.
    namespace EntityGroup {
        // Byte used to identify the start of an entity group block
        constant IDENTIFIER = 0x45 // "E"

        struct()
            // Number of entities in the group
            //
            // If zero then there is no more entity groups in the room
            //
            // (byte)
            field(nEntities, 1)

            constant HEADER_SIZE = __STRUCT__.size

            namespace Entities {
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
            }
        endstruct()

        constant Entities.size = EntityGroup.size - HEADER_SIZE
    }



    // Room footer
    //
    // Used to verify all blocks in a room have been read successfully
    constant FOOTER = 0x21444e45 // "END!"
    constant FOOTER_SIZE = 4
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

