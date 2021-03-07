// metatiles/_variables.inc
// ========================
//
// Variables used by the MetaTiles module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {
    constant METATILE_SIZE = 2
    constant METATILE_SIZE_PX = 16
    constant METATILE_DISPLAY_WIDTH = 16
    constant METATILE_DISPLAY_HEIGHT = 14

    constant MAP_HEIGHT_SMALL = 64
    constant MAP_HEIGHT_LARGE = 128

    constant MAP_MAX_WIDTH = 0xff



    namespace CrumblingTiles {
        // Allocate a continuous buffer to store the queue arrays.
        // This allows me to rollback time in both queues using a single loop
        constant N_ELEMENTS_IN_BOTH_QUEUES = 32 * 2
        allocate(_timeBuffer,         wram7e, N_ELEMENTS_IN_BOTH_QUEUES * 2)
        allocate(_mapDataIndexBuffer, wram7e, N_ELEMENTS_IN_BOTH_QUEUES * 2)

        assert(_timeBuffer & 0xff == 0)
    }

    // Location (in memory) to store the MetaTile Tileset
    constant TILESET_DATA_SIZE = 256 * 10 + 14
    allocate(_tilesetData, wram7e, TILESET_DATA_SIZE)



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

        // The map.data index offset between the top-left and the bottom-right of the screen.
        //
        // Equal to: `bytesPerColumn * METATILE_DISPLAY_WIDTH + METATILE_DISPLAY_HEIGHT`
        //
        // (uint16)
        allocate(dataIndexScreenOffset, wram7e, 2)

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



    namespace Render {
        // Address of the top-left most visible cell
        // (word index)
        allocate(visibleMapPtr, wram7e, 2)


        // Offset between the Camera position and the display offset
        // Reset on every DrawFullScreen_ForceBlank
        // (2x sint16)
        allocate(cameraXoffset, wram7e, 2)
        allocate(cameraYoffset, wram7e, 2)
    }



    // Horizontal SNES Tilemap Buffer
    // A single tile map 64 * 2 tiles in size
    namespace horizontalBuffer {
        constant buffer.size = 64 * 2 * 2
        allocate(buffer, wram7e, buffer.size)
        // No need to test if buffer is on a single page - we never read from it with index addressing

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
        // No need to test if buffer is on a single page - we never read from it with index addressing

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


    // Horizontal and vertical buffer states
    //
    // Used by `Dma.MetaTiles` to determine if horizontalBuffer or verticalBuffer is to be transferred to VRAM.
    //
    // (byte flags)
    allocate(bufferState, shadow, 1)
    namespace bufferState {
        constant VERTICAL = 0x80
        constant HORIZONTAL = 0x7f
    }


    // DrawMapCell VBlank buffer
    // (Structure of arrays)
    namespace drawMapCellBuffer {
        constant BUFFER_SIZE = 4

        // Vram word address to transfer the MetaTile to
        // (word address - shadow)
        allocate(vramWaddr, shadow, BUFFER_SIZE * 2)

        // The four map tiles to transfer to VRAM
        // (Tilemap word * 4)
        allocate(topLeft, shadow, BUFFER_SIZE * 2)
        allocate(topRight, shadow, BUFFER_SIZE * 2)
        allocate(bottomLeft, shadow, BUFFER_SIZE * 2)
        allocate(bottomRight, shadow, BUFFER_SIZE * 2)


        assertOnePage(vramWaddr,    BUFFER_SIZE * 2)
        assertOnePage(topLeft,      BUFFER_SIZE * 2)
        assertOnePage(topRight,     BUFFER_SIZE * 2)
        assertOnePage(bottomRight,  BUFFER_SIZE * 2)
        assertOnePage(topLeft,      BUFFER_SIZE * 2)
    }
    // Index to the current position on the drawMapCellBuffer
    // (word index)
    allocate(drawMapCellBufferIndex, shadow, 2)

    // If `drawMapCellBufferIndex` is >= `END` value then the buffer is full
    constant drawMapCellBufferIndex.END = drawMapCellBuffer.BUFFER_SIZE * 2



    namespace SetMapCellQueue {
        constant N_ELEMENTS = 32

        // Current position within the queue.
        //
        // Incremented by `SetMapCell`, decremented after drawing map cell.
        //
        // (byte index into `mapDataIndex)
        allocate(currentIndex, wram7e, 1)
        constant INDEX_INCREMENT = 2
        constant LAST_INDEX = (N_ELEMENTS - 1) * INDEX_INCREMENT


        // Pending `SetMapCell` cells that have yet to be drawn to screen.
        //
        // (array of word indexes within `map.data`)
        allocate(mapDataIndex, wram7e, N_ELEMENTS * 2)
        assertOnePage(mapDataIndex, N_ELEMENTS * 2)
    }



    namespace CrumblingTiles {
        // Time used by the crumbling blocks
        //
        // This value is not in sync with `frameCounter`.
        //
        // `time` will be incremented on every `MetaTile.Process` call and will
        // be rolled back when it is >= 0x8000.
        //
        // (uint16 - time in frames)
        allocate(time, wram7e, 2)


        variable __bufferOffset = 0
        inline __AllocateVariables(ns, evaluate nElements) {
            // The tile to turn into on the first, second and third transitions
            // (3x byte)
            constant firstTileId  = Tileset.Footer.CrumblingTiles{ns}.firstTileId
            constant secondTileId = Tileset.Footer.CrumblingTiles{ns}.secondTileId
            constant thirdTileId  = Tileset.Footer.CrumblingTiles{ns}.thirdTileId

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
                constant N_ELEMENTS = {nElements}

                constant INDEX_INCREMENT = 2

                // The maximum allowed index value for a queue position index
                constant LAST_INDEX = INDEX_INCREMENT * (N_ELEMENTS - 1)

                // Negative indexes are used to test if the position index underflowed
                assert(LAST_INDEX < 0x80)


                // The time the next transition occurs at.
                //
                // This value is compared with `CrumblingTiles.time`, not `frameCounter`.
                //
                // This value is not used by the first queue.
                //
                // (uint16)
                constant time = _timeBuffer + __bufferOffset * 2
                assertOnePage(time, N_ELEMENTS * 2)

                // The index within `map.data` for the crumbling tile.
                //
                // (uint16 index into `map.data`)
                constant mapDataIndex = _mapDataIndexBuffer + __bufferOffset * 2
                assertOnePage(mapDataIndex, N_ELEMENTS * 2)

                __bufferOffset = __bufferOffset + N_ELEMENTS



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
            __AllocateVariables(ChainA, 32)
        }
        namespace ChainB {
            __AllocateVariables(ChainB, 32)
        }
        assert(__bufferOffset == N_ELEMENTS_IN_BOTH_QUEUES)

        constant N_CRUMBLING_BLOCK_CHAINS = 2
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

