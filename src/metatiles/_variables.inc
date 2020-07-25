// metatiles/_variables.inc
// ========================
//
// Variables used by the MetaTiles module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
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


        // X component of an index within `map.data`
        // (uint16)
        allocate(dataIndexXmask, wram7e, 2)

        // Y component of an index within `map.data`
        // (uint16)
        allocate(dataIndexYmask, wram7e, 2)

        // Y position mask used for converting a position to a tile index/address
        // Equal to either Y_POSITION_MASK_SMALL or Y_POSITION_MASK_LARGE
        // (uint16)
        allocate(yPositionMask, wram7e, 2)
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

    // If `drawMapCellBufferIndex` is >= `END` value then the buffer is full
    constant drawMapCellBufferIndex.END = drawMapCellBuffer.BUFFER_SIZE * 2



    namespace CrumblingTiles {
        // Time used by the crumbling blocks
        //
        // This value is not in sync with `frameCounter`, it will be
        // incremented on every `CrumblingTiles.Process` and may be
        // decremented if overflowing.
        //
        // (uint16 - time in frames)
        allocate(time, wram7e, 2)


        inline __AllocateVariables(ns) {
            // The tile to turn into on the first, second and third transitions
            // (3x byte)
            constant firstTileId  = tilesetFooter.CrumblingTiles{ns}.firstTileId
            constant secondTileId = tilesetFooter.CrumblingTiles{ns}.secondTileId
            constant thirdTileId  = tilesetFooter.CrumblingTiles{ns}.thirdTileId

            // Time to wait between the first and second tile transition in display frames.
            // (uint16 - display frames)
            allocate(firstWait, wram7e, 2)

            // Time to wait between the second and third tile transition in display frames.
            //
            // If this value is 0xffff then there is no third tile transition
            //
            // (uint16 - display frames)
            allocate(secondWait, wram7e, 2)


            // Crumbling tiles queue.
            //
            // SoA circular buffer.
            //
            // The buffer positions start at the end and increment downwards.
			//
			// The buffer is considered full if the `index after writerPos` == `thirdPos`.
			// This greatly simplifies the code but only allows only allows `N_ELEMENTS - 1`
			// crumbling tiles to be active per queue (with a cost of 3 bytes of wasted memory
			// per queue).
            namespace Queue {
                constant N_ELEMENTS = 32

				// ::TODO make these variables page aligned::

                // The time the next transition occurs at.
                //
                // This value is compared with `CrumblingTiles.time`, not `frameCounter`.
                //
                // This value is not used by the first queue.
                //
                // (uint16)
                allocate(time, wram7e, 2 * N_ELEMENTS)


                // The index within `map.data` for the crumbling tile.
                //
                // (uint16 index into `map.data`)
                allocate(mapDataIndex, wram7e, 2 * N_ELEMENTS)


                constant INDEX_INCREMENT = 2

                // The maximum allowed index value for a queue position index
                constant LAST_INDEX = INDEX_INCREMENT * (N_ELEMENTS - 1)

                // Negative indexes are used to test if the position index underflowed
                assert(LAST_INDEX < 0x80)


                // Position of next free element in the circular buffer.
                //
                // This variable is also used to mark the end of the first queue.
				//
				// If this variable is equal to equal to the item before `thirdPos`
				// then the circular buffer is full.
                //
                // (byte index)
                allocate(writerPos, wram7e, 1)

                // Position of first queue in the circular buffer
                //
                // This variable is also used to mark the end of the second queue.
                //
                // Tiles in this queue will be processed on the next
				// `MetaTiles.CrumblingTiles.Process` call.
                //
                // `Queue.time` is ignored by this queue.
                //
                // (byte index)
                allocate(firstPos, wram7e, 1)

                // Position of the second queue in the circular buffer
                //
                // This variable is also used to mark the end of the third queue.
                //
                // (byte index)
                allocate(secondPos, wram7e, 1)

                // Position of the third queue in the circular buffer
                //
                // This variable is also used to mark the end of the writable area in the circular buffer.
                //
                // This queue will always be empty if `secondWait` is negative.
                //
                // (byte index)
                allocate(thirdPos, wram7e, 1)
            }
        }
        namespace ChainA {
            __AllocateVariables(ChainA)
        }
        namespace ChainB {
            __AllocateVariables(ChainB)
        }
        assert(tilesetFooter.N_CRUMBLING_BLOCK_CHAINS == 2)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

