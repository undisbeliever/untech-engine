// metatiles/_dataformat.inc
// ==========================
//
// MetaTile Data Format.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
    if MetaTiles.TILESET_FORMAT_VERSION != 8 {
        error "Invalid untech-compiler MetaTile Tileset Format"
    }

    if MetaTiles.INTERACTIVE_TILES_FORMAT_VERSION != 1 {
        error "Invalid untech-compiler Interactive Tiles Format"
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
    // NOTE: Each entry MUST MATCH a TileCollisionType value
    //
    allocate(collisionMap, wram7e, N_META_TILES)
    constant collisionMap.size = N_META_TILES


    // Interactive tiles function table index for each MetaTile
    //
    // Format:
    //      00ttttt0
    //          ttttt = interactive tile id
    //
    // See `InteractiveTilesFunctionTables` for details about the function tables.
    allocate(interactiveTilesMap, wram7e, N_META_TILES)
    namespace interactiveTilesMap {
        constant size = N_META_TILES

        // Number of functions inside the interactive tiles function table.
        constant N_FUNCTIONS = 32

        constant MASK = (N_FUNCTIONS - 1) << 1
    }


    // Tile Collision type used by the collision map.
    //
    // Each value in this enum is combined with the tile X-pixel position to
    // create a `TopHeightTable`/`BottomHeightTable` index using the following
    // formula:
    //
    //      height_table_index = tileCollisonType | (xPos & 0xf);
    //
    //
    // ::KUDOS Christopher Hebert for the slope names::
    // ::: Reconstructing Cave Story: Slope Theory::
    // ::: https://www.youtube.com/watch?v=ny14i0GxGZw ::
    namespace TileCollisionType {
        createEnum(0, 16)
            enum(SOLID)                     // Most common check
            enum(DOWN_RIGHT_SLOPE)          // right and down sides are the biggest, fall down to collide, walk up to ascend
            enum(DOWN_LEFT_SLOPE)
            enum(DOWN_RIGHT_SHORT_SLOPE)
            enum(DOWN_RIGHT_TALL_SLOPE)
            enum(DOWN_LEFT_TALL_SLOPE)
            enum(DOWN_LEFT_SHORT_SLOPE)
            enum(UP_PLATFORM)
            enum(EMPTY)
            enum(DOWN_PLATFORM)
            enum(UP_RIGHT_SLOPE)
            enum(UP_LEFT_SLOPE)
            enum(UP_RIGHT_SHORT_SLOPE)
            enum(UP_RIGHT_TALL_SLOPE)
            enum(UP_LEFT_TALL_SLOPE)
            enum(UP_LEFT_SHORT_SLOPE)

            // No collision in X-axis.
            // Solid collision for bottom-center or top-center tile in Y-axis.
            constant END_SLOPE = SOLID | 0x08
        endEnum()

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
            constant size = tileMap.size + collisionMap.size + interactiveTilesMap.size

            // Ensure `tileMap`, `collisionMap`, and `interactiveTilesMap` are continuous
            assert(tileMap + tileMap.size == collisionMap)
            assert(collisionMap + collisionMap.size == interactiveTilesMap)

            // Ensure the variables are page aligned
            assert(tileMap & 0xff == 0)
            assert(collisionMap & 0xff == 0)
            assert(interactiveTilesMap & 0xff == 0)
        }
    }



    namespace InteractiveTiles {
        // Called when a tile-interacting entity has a Y-axis collision with
        // the tile.
        //
        // This function is called on the first tile that collided the entity's
        // tile hitbox.  For instance, if there was a y-axis collision on the
        // tile at the bottom-center of the hitbox, then the entity will not
        // interact with the tiles at the hitbox's bottom-left or bottom-right.
        //
        // NOTE: X-axis collisions DO NOT call this function.
        //
        // NOTE: This function is only called on entities that call
        //       `MoveEntityWithTileCollisionsAndInteractiveTiles`.
        //
        // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp = entity
        // INPUT: Y = map data index of collided tile
        constant EntityCollision_FunctionTable = Project.InteractiveTiles.EntityCollision_FunctionTable

        // Called when a tile-interacting entity hovers over the tile.
        //
        // Only the map tile at the entity's position is checked, all other map
        // tiles inside the entity's tile hitbox will be ignored.
        //
        // NOTE: This function is only called on entities that call
        //       `MoveEntityWithTileCollisionsAndInteractiveTiles`.
        //
        // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp = entity
        // INPUT: Y = map data index of the tile at the entity's position
        constant EntityAirCollision_FunctionTable = Project.InteractiveTiles.EntityAirCollision_FunctionTable

        // Called when a tile-interacting player has a Y-axis collision and
        // the map tile is in the same column as the player's X-position.
        //
        // This function will not be called if there is no collision in the map
        // tile at the player's x-position.
        //
        // NOTE: X-axis collisions DO NOT call this function.
        //
        // NOTE: This function is only called on entities that call
        //       `MovePlayerWithTileCollisionsAndInteractiveTiles`.
        //
        // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp = entity address of player
        // INPUT: Y = map data index of collided tile
        constant PlayerOriginCollision_FunctionTable = Project.InteractiveTiles.PlayerOriginCollision_FunctionTable

        // Called when a tile-interacting player has a Y-axis collision and
        // the map tile is not in the same column as the player's X-position.
        //
        // This function will be called whenever the tile-interacting player
        // has a Y-axis collision.  It can be called between 0 and 4 times per
        // player depending on the size of the player's tile hitbox.
        //
        // NOTE: This function MUST BE blank if the tile is not SOLID.
        //       ::TODO add this check to the editor::
        //
        // NOTE: X-axis collisions DO NOT call this function.
        //
        // NOTE: This function is only called on entities that call
        //       `MovePlayerWithTileCollisionsAndInteractiveTiles`.
        //
        // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp = entity address of player
        // INPUT: Y = map data index
        constant PlayerLeftRightCollision_FunctionTable = Project.InteractiveTiles.PlayerLeftRightCollision_FunctionTable

        // Called when a tile-interacting player hovers over the tile.
        //
        // Only the map tiles that are in the same column as the player's
        // X-position are invoked.  All tiles to the left or right of the
        // player's x-position are ignored.
        //
        // NOTE: This function is only called on entities that call
        //       `MovePlayerWithTileCollisionsAndInteractiveTiles`.
        //
        // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
        // INPUT: dp = entity
        // INPUT: Y = map data index
        constant PlayerAirCollision_FunctionTable = Project.InteractiveTiles.PlayerAirCollision_FunctionTable


        assert(EntityAirCollision_FunctionTable - EntityCollision_FunctionTable == interactiveTilesMap.N_FUNCTIONS * 2)
        assert(PlayerOriginCollision_FunctionTable - EntityAirCollision_FunctionTable == interactiveTilesMap.N_FUNCTIONS * 2)
        assert(PlayerLeftRightCollision_FunctionTable - PlayerOriginCollision_FunctionTable == interactiveTilesMap.N_FUNCTIONS * 2)
        assert(PlayerAirCollision_FunctionTable - PlayerLeftRightCollision_FunctionTable == interactiveTilesMap.N_FUNCTIONS * 2)
        assert(Project.InteractiveTiles.EndFunctionTables - PlayerAirCollision_FunctionTable == interactiveTilesMap.N_FUNCTIONS * 2)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

