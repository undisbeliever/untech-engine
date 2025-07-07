// metatiles/_dataformat.inc
// ==========================
//
// MetaTile Data Format.
//
// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
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


namespace MetaTiles {
    if MetaTiles.TILESET_FORMAT_VERSION != 9 {
        error "Invalid untech-compiler MetaTile Tileset Format"
    }

    if MetaTiles.INTERACTIVE_TILES_FORMAT_VERSION != 1 {
        error "Invalid untech-compiler Interactive Tiles Format"
    }

    constant N_META_TILES = 256


    // MetaTile Tileset data format
    namespace Tileset {
        struct(_tilesetData)
        struct_maxsize(TILESET_DATA_SIZE)

        // SNES Tile Map data for each MetaTile
        //
        // (structure of arrays -> Tilemap)
        namespace tileMap {
            field(topLeft,      N_META_TILES * 2)
            field(topRight,     N_META_TILES * 2)
            field(bottomLeft,   N_META_TILES * 2)
            field(bottomRight,  N_META_TILES * 2)

            assert(topLeft & 0xff == 0)
        }
        constant tileMap = tileMap.topLeft
        constant tileMap.size = N_META_TILES * 2 * 4


        // Collision data for each MetaTile
        //
        // NOTE: Each entry MUST MATCH a TileCollisionType value
        //
        field(collisionMap, N_META_TILES)
        constant collisionMap.size = N_META_TILES
        assertOnePage(collisionMap)


        // Interactive tiles function table index for each MetaTile
        //
        // Format:
        //      00ttttt0
        //          ttttt = interactive tile id
        //
        // See `InteractiveTilesFunctionTables` for details about the function tables.
        field(interactiveTilesMap, N_META_TILES)
        namespace interactiveTilesMap {
            constant size = N_META_TILES

            // Number of functions inside the interactive tiles function table.
            constant N_FUNCTIONS = 32

            constant MASK = (N_FUNCTIONS - 1) << 1
        }
        assertOnePage(interactiveTilesMap)


        // This data is at the end because we want the tileset-map variables to be page aligned.
        // Crumbling tile settings for the MetaTile Tileset.
        namespace Footer {
            inline __CrumblingTilesChain() {
                // The first tile in the chain
                //
                // The crumble tile will change to this tileId when the crumbling tile is activated.
                //
                // MUST NOT be the tileId of the crumble tile
                //
                // (byte)
                field(firstTileId, 1)

                // The second tile in the chain
                //
                // The crumble tile will change to this tileId after `firstDelay` ticks.
                //
                // (byte)
                field(secondTileId, 1)

                // The third tile in the chain
                //
                // The crumble tile will change to this tileId after `firstDelay` + `secondDelay` ticks.
                //
                // NOTE: This tileId will not be used if `secondDelay` is `0xffff`.
                //
                // (byte)
                field(thirdTileId, 1)

                // The delay between the first and second tile transition in animation ticks.
                // (uint16 - ticks)
                field(firstDelay, 2)

                // The delay between the second and third tile transition in animation ticks.
                //
                // If this value is `0xffff`, then there is no third transition on this chain.
                //
                // (uint16 - ticks)
                field(secondDelay, 2)
            }
            namespace CrumblingTilesChainA {
                __CrumblingTilesChain()
            }
            namespace CrumblingTilesChainB {
                __CrumblingTilesChain()
            }
            assert(MetaTiles.CrumblingTiles.N_CRUMBLING_BLOCK_CHAINS == 2)
        }

        endstruct()
        assert(size == TILESET_DATA_SIZE)
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
        //      * MetaTile tiles block
        //          - lz4 compressed block
        //          - contains `MetaTiles.Tileset` struct, copied straight to WRAM
        //      * Animated Tileset block
        //          - see Resources.Format.AnimatedTilesetHeader for data format
        namespace Tileset {
            constant wramLocation = MetaTiles._tilesetData
            constant size = MetaTiles.Tileset.size

            // Ensure the variables are page aligned
            assert(wramLocation & 0xff == 0)
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


        assert(EntityAirCollision_FunctionTable - EntityCollision_FunctionTable == Tileset.interactiveTilesMap.N_FUNCTIONS * 2)
        assert(PlayerOriginCollision_FunctionTable - EntityAirCollision_FunctionTable == Tileset.interactiveTilesMap.N_FUNCTIONS * 2)
        assert(PlayerLeftRightCollision_FunctionTable - PlayerOriginCollision_FunctionTable == Tileset.interactiveTilesMap.N_FUNCTIONS * 2)
        assert(PlayerAirCollision_FunctionTable - PlayerLeftRightCollision_FunctionTable == Tileset.interactiveTilesMap.N_FUNCTIONS * 2)
        assert(Project.InteractiveTiles.EndFunctionTables - PlayerAirCollision_FunctionTable == Tileset.interactiveTilesMap.N_FUNCTIONS * 2)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

