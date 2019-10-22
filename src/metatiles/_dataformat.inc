// metatiles/_dataformat.inc
// ==========================
//
// MetaTile Data Format.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
    if MetaTiles.TILESET_FORMAT_VERSION != 7 {
        error "Invalid untech-compiler MetaTile Tileset Format"
    }

    constant N_META_TILES = 256


    // SNES Tile Map data for each MetaTile
    //
    // (structure of arrays -> Tilemap)
    namespace tileMap {
        allocate(topLeft,       wram7e, N_META_TILES * 2)
        allocate(topRight,      wram7e, N_META_TILES * 2)
        allocate(bottomLeft,    wram7e, N_META_TILES * 2)
        allocate(bottomRight,   wram7e, N_META_TILES * 2)
    }
    constant tileMap = tileMap.topLeft
    constant tileMap.size = N_META_TILES * 2 * 4


    // Collision data for each MetaTile
    //
    // NOTE: Each entry MUST MATCH a HeightTableIndex value
    //
    allocate(collisionMap, wram7e, N_META_TILES)
    constant collisionMap.size = N_META_TILES


    // The height table used by the collision map
    //
    // ::KUDOS Christopher Hebert for the slope names::
    // ::: Reconstructing Cave Story: Slope Theory::
    // ::: https://www.youtube.com/watch?v=ny14i0GxGZw ::
    namespace HeightTableIndex {
        createEnum(0, 16)
            enum(EMPTY)
            enum(SOLID)
            enum(UP_PLATFORM)
            enum(DOWN_PLATFORM)
            enum(DOWN_RIGHT_SLOPE)          // right and down sides are the biggest, fall down to collide, walk up to ascend
            enum(DOWN_LEFT_SLOPE)
            enum(DOWN_RIGHT_SHORT_SLOPE)
            enum(DOWN_RIGHT_TALL_SLOPE)
            enum(DOWN_LEFT_TALL_SLOPE)
            enum(DOWN_LEFT_SHORT_SLOPE)
            enum(UP_RIGHT_SLOPE)
            enum(UP_LEFT_SLOPE)
            enum(UP_RIGHT_SHORT_SLOPE)
            enum(UP_RIGHT_TALL_SLOPE)
            enum(UP_LEFT_TALL_SLOPE)
            enum(UP_LEFT_SHORT_SLOPE)
        endEnum()

        // No collision in X-axis.
        // Solid collision for bottom-center or top-center tile in Y-axis.
        constant END_SLOPE = SOLID | 0x08

        assert(__ENUM__.last <= %11110000)
    }


    // List of MetaTile Tilesets
    // (Long Addr Table)
    constant TilesetList = Project.MetaTileTilesetList
    constant TilesetList.count = Project.MetaTileTilesetList.count


    namespace Format {

        // MetaTile Tileset data format
        //  Two lz4 compressed data blocks, one after each other
        //      * tile data block
        //          - lz4 compressed block
        //          - contains tileMap + collisionMap, copied straight to WRAM
        //      * Animated Tileset block
        //          - see Resources.Format.AnimatedTilesetHeader for data format
        namespace Tileset {
            constant wramLocation = MetaTiles.tileMap
            constant size = tileMap.size + collisionMap.size

            // Ensure `tileMap` and `collisionMap` are continuous
            assert(tileMap + tileMap.size == collisionMap)

            // Ensure `tileMap` and `collisionMap` is page aligned
            assert(tileMap & 0xff == 0)
            assert(collisionMap & 0xff == 0)
        }
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

