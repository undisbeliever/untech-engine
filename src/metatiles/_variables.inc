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

    constant N_TILES = 512

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

        // Number of bytes per map row
        //
        // Equal to map.width * 2.
        // (word index)
        allocate(bytesPerRow, wram7e, 2)
    }


    // SNES Tile Map data for each MetaTile
    //
    // (structure of arrays -> Tilemap)
    namespace tileMap {
        allocate(topLeft, wram7e, N_TILES * 2)
        allocate(topRight, wram7e, N_TILES * 2)
        allocate(bottomLeft, wram7e, N_TILES * 2)
        allocate(bottomRight, wram7e, N_TILES * 2)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

