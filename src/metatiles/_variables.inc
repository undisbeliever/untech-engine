// metatiles/_variables.inc
// ========================
//
// Variables used by the MetaTiles module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
    assert(MAP_SIZE >= 1024)

    constant METATILE_SIZE = 2
    constant METATILE_SIZE_PX = 16
    constant METATILE_DISPLAY_WIDTH = 16
    constant METATILE_DISPLAY_HEIGHT = 14

    constant MAP_HEIGHT_SMALL = 64
    constant MAP_HEIGHT_LARGE = 128

    namespace map {
        // Map width in metatiles
        // (uint8)
        constant width = Room.Format.Header.width

        // Map data
        //
        // (byte grid array column-major order)
        constant data = Room.Format.mapData


        // Calculated values
        // =================

        // Number of bytes per column (AKA map height in metaTiles)
        //
        // Used to determine if the map has a small (64) or large (128)
        // height, see `BranchIfMapIsSmall` macro for more details.
        //
        // Used to move tileIndex left or right.
        //
        // MUST be equal to MAP_HEIGHT_SMALL (64) or MAP_HEIGHT_LARGE (128)
        //
        // (uint16, MUST be equal to 64 or 128)
        allocate(bytesPerColumn, wram7e, 2)

        // The number of bytes in the data
        // (Equal to map.width * map.bytesPerColumn)
        // (uint16)
        allocate(dataSize, wram7e, 2)

        // Top-left of map is not at (0, 0) to eliminate signed comparison when collision testing.
        constant TOP = 0x1000
        constant LEFT = 0x1000

        // The bottom-most y coordinate of the map
        // (uint16)
        allocate(bottom, wram7e, 2)

        // The right-most x coordinate of the map
        // (uint16)
        allocate(right, wram7e, 2)
    }

    // A bit-mask used when converting a position to a tile index/address
    constant X_POSITION_MASK = 0xff * METATILE_SIZE_PX
    constant Y_POSITION_MASK_SMALL = (MAP_HEIGHT_SMALL - 1) * METATILE_SIZE_PX
    constant Y_POSITION_MASK_LARGE = (MAP_HEIGHT_LARGE - 1) * METATILE_SIZE_PX
    assertPowerOfTwo(MAP_HEIGHT_SMALL)
    assertPowerOfTwo(MAP_HEIGHT_LARGE)
    assert(X_POSITION_MASK < map.LEFT)
    assert(Y_POSITION_MASK_SMALL < map.TOP)
    assert(Y_POSITION_MASK_LARGE < map.TOP)

    // Horizontal SNES Tilemap Buffer
    // A single tile map 64 * 2 tiles in size
    namespace horizontalBuffer {
        constant buffer.size = 64 * 2 * 2
        allocate(buffer, wram7e, buffer.size)

        // Vram word address to transfer the buffer to
        // (word address - shadow)
        allocate(vramWaddr, shadow, 2)

        // Index of the leftmost visible tile in the buffer
        // (word index)
        allocate(cursorIndex, wram7e, 2)

        // Y coordinate of the topmost visible tile in the buffer
        // Used to determine when to update the buffer
        // (uint16)
        allocate(cursorYpos, wram7e, 2)
    }


    // Vertical SNES Tilemap Buffer
    // Two tile maps, each 1 * 32 tiles in size
    namespace verticalBuffer {
        constant buffer.size = 32 * 2 * 2
        allocate(buffer, wram7e, buffer.size)

        // Vram word address to transfer the buffer to
        // (word address - shadow)
        allocate(vramWaddr, shadow, 2)

        // Index of the topmost visible tile in the buffer
        // (word index)
        allocate(cursorIndex, wram7e, 2)

        // X coordinate of the topmost visible tile in the buffer
        // Used to determine when to update the buffer
        // (uint16)
        allocate(cursorXpos, wram7e, 2)
    }


    // Buffer state
    allocate(bufferState, shadow, 1)
    namespace bufferState {
        constant VERTICAL = 0x80
        constant HORIZONTAL = 0x7f
    }


    // DrawMapCell VBlank buffer
    // (Structure of arrays)
    namespace drawMapCellBuffer {
        constant BUFFER_SIZE = 3

        // Vram word address to transfer the MetaTile to
        // (word address - shadow)
        allocate(vramWaddr, shadow, BUFFER_SIZE * 2)

        // The four map tiles to transfer to VRAM
        // (Tilemap word * 4)
        allocate(topLeft, shadow, BUFFER_SIZE * 2)
        allocate(topRight, shadow, BUFFER_SIZE * 2)
        allocate(bottomLeft, shadow, BUFFER_SIZE * 2)
        allocate(bottomRight, shadow, BUFFER_SIZE * 2)
    }
    // Index to the current position on the drawMapCellBuffer
    // (word index)
    allocate(drawMapCellBufferIndex, shadow, 2)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

