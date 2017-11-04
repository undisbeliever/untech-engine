// metatiles/_variables.inc
// ========================
//
// Variables used by the MetaTiles module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
    assert(MAP_SIZE >= 1024)
    assert(N_META_TILES >= 64)

    constant METATILE_SIZE = 2
    constant METATILE_SIZE_PX = 16
    constant METATILE_DISPLAY_WIDTH = 16
    constant METATILE_DISPLAY_HEIGHT = 14

    namespace map {
        // Map Width in metatiles
        // (uint8)
        allocate(width, wram7e, 1)

        // Map Height in metatiles
        //
        // Must be >= 12
        //
        // (uint8)
        allocate(height, wram7e, 1)

        // Y coordinate of the map
        // (uint16)
        allocate(top, wram7e, 2)

        // X coordinate of the map
        // (uint16)
        allocate(left, wram7e, 2)

        // Map data
        // (array of indexes to tileMap AoS)
        allocate(data, wram7e, MAP_SIZE * 2)


        // Calculated values
        // =================

        // The number of bytes in the data
        // (Equal to width * height * 2)
        // (uint16)
        allocate(dataSize, wram7e, 2)

        // The bottom-most y coordinate of the map
        // (uint16)
        allocate(bottom, wram7e, 2)

        // The right-most x coordinate of the map
        // (uint16)
        allocate(right, wram7e, 2)

        // Number of bytes per map row
        //
        // Equal to map.width * 2.
        // (uint16)
        allocate(bytesPerRow, wram7e, 2)

        // Number of bytes per display height
        //
        // Equal to map.width * METATILE_DISPLAY_HEIGHT * 2
        // (uint16)
        allocate(bytesPerDisplayHeight, wram7e, 2)
    }


    // SNES Tile Map data for each MetaTile
    //
    // (structure of arrays -> Tilemap)
    namespace tileMap {
        allocate(topLeft, wram7e, N_META_TILES * 2)
        allocate(topRight, wram7e, N_META_TILES * 2)
        allocate(bottomLeft, wram7e, N_META_TILES * 2)
        allocate(bottomRight, wram7e, N_META_TILES * 2)
    }
    constant tileMap = tileMap.topLeft
    constant tileMap.size = N_META_TILES * 2 * 4


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

