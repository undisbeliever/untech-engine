// metasprite/_variables.inc
// =========================
//
// Variables used by the MetaSprite module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {

    // The MetaSprite entity data
    // MUST BE called inside the BaseEntity struct
    inline EntityData() {
        // The frameSet that the metasprite belongs to
        // MUST NOT BE NULL
        // (word index)
        field(MetaSprite.frameSet, 2)

        // The state of the MetaSprite's VRAM/Palette allocations
        //
        // 	    %pdiiiiiv
        //
        //      p: palette set
        //      d: dynamic tileset active (MUST NOT be set if vram is clear)
        //      iiiii: vram slot table number (equal to vram slot index)
        //      v: vram set
        field(MetaSprite.status, 1)

        // Palette id
        field(MetaSprite.palette, 1)

        // Current frame to display/process
        // MUST NOT BE NULL
        // (word index)
        field(MetaSprite.currentFrame, 2)

        // Offset between frame object data and the OAM charattr data
        field(MetaSprite.blockOneCharAttrOffset, 2)

        // Offset between frame object data and the OAM charattr data (second block)
        field(MetaSprite.blockTwoCharAttrOffset, 2)

        // The AnimationDurationFormat for the current animation frame
        // (see MetaSprite.Format.AnimationDurationFormat)
        field(MetaSprite.Animation.durationFormat, 1)

        // Current animation id
        // If 0xff then the animation is disabled
        field(MetaSprite.Animation.id, 1)

        // The current position within the animation frame table
        // (one byte index)
        field(MetaSprite.Animation.pos, 1)

        // Time until the next animation frame is processed.
        //
        // If negative then the current animation frame has not been processed
        // and will be loaded on the next `MetaSprite.Animation.Process` call.
        //
        // (sint16)
        field(MetaSprite.Animation.nextFrameTime, 2)
    }

    namespace StatusFlags {
        constant paletteSet = 0x80
        constant dynamicTileset = 0x40
        constant vramSlotMask = 0x3e

        // The entity has a VRAM slot assigned to it
        // (NOTE: does not signify the entity's tiles are uploaded to VRAM)
        constant vramSet = 0x01
    }

    namespace Render {
        allocate(oamBuffer, wram7e, 128 * 4)
        namespace oamBuffer {
            constant xPos = oamBuffer + OamFormat.xPos
            constant yPos = oamBuffer + OamFormat.yPos
            constant charAttr = oamBuffer + OamFormat.charAttr
        }

        allocate(oamHiBuffer, wram7e, 128 / 4)

        // Buffer the oam's x8/size bits
        // bits 10-15 of each 4 byte block MUST BE CLEAR
        //
        // bits 16-31 are unused by the renderer and can be used elsewhere,
        // they are marked as unused words at the bottom of this file.
        allocate(oamXposBuffer, wram7e, 128 * 4)

        // The index of the buffer
        // (word index)
        allocateTmpWord(bufferPos)

        // The index of the buffer in the previous frame
        // (word index)
        allocateTmpWord(previousBufferPos)


        // argument for `MetaSprite.Render functions`
        // (word value - INT_MS8_OFFSET)
        allocateTmpWord(xPos)

        // argument for `MetaSprite.Render functions`
        // (word value - INT_MS8_OFFSET)
        allocateTmpWord(yPos)
    }

    namespace Collision {
        // Collision bits
        // See (THIS_* and OTHER_* constants)
        // (uint8_t, stored as two bytes for efficiency)
        allocate(bits, wram7e, 2)

        // The entity that the object collided with
        // (word addr)
        allocate(otherEntity, dp, 2)

        // The centre position of the collision
        // (2x uint16)
        allocate(xPos, dp, 2)
        allocate(yPos, dp, 2)
    }


    namespace Vram {
        constant VRAM_TILE_SLOTS = 16
        constant VRAM_ROW_SLOTS = 14

        assert(VRAM_TILE_SLOTS + VRAM_ROW_SLOTS * 8 <= 8 * 16)

        // State data for the VRAM table
        //
        // Slots 0 - (VRAM_TILE_SLOTS - 1) are VRAM tile slots
        // Slots `VRAM_TILE_SLOTS` and onwards are VRAM row slots
        //
        // This data exists in one of 4 states:
        //	1) A single linked list holding the free slots
        //	2) Not part of a linked list (used in dynamic tilesets)
        //	3) A double linked list for fixed tilesets (it makes removing easier)
        //	4) Second half of a dual tileset. (not part of a linked list, see `pair`)
        //
        // Data is a SOA linked list.
        //
        //	    slot index = status & METASPRITE_VRAM_TILE_SLOTS
        //
        namespace slots {
            constant N_ELEMENTS = VRAM_TILE_SLOTS + VRAM_ROW_SLOTS
            constant ARRAY_INCREMENT = 2
            constant _sectionSize = N_ELEMENTS * ARRAY_INCREMENT

            // All indexes at or above this value are Row slots
            constant ROW_SLOTS_MIN_INDEX = ARRAY_INCREMENT * VRAM_TILE_SLOTS


            // This assumption allows the MSB of the index to mean NULL
            assert(N_ELEMENTS * ARRAY_INCREMENT <= 128)

            // Index of the next slot.
            // This is NOT set if the slot is dynamic or the second half of a dual slot
            // (byte index, if >= 0x80 then NULL)
            allocate(next, wram7e, N_ELEMENTS * 2)

            // Index of the previous slot.
            // This is ONLY set if in a fixed tileset
            // (byte index, if >= 0x80 then NULL)
            constant prev = next + 1

            // Address of the tileset in the `DMA_Tile16Data` block
            // (word address)
            allocate(tileset, wram7e, N_ELEMENTS * 2)

            // Number of times the slot is used
            // (uint8)
            allocate(count, wram7e, N_ELEMENTS * 2)

            // Index of the second slot (if in a two slot type)
            // (byte index, if >= 0x80 then NULL
            constant pair = count + 1
        }

        namespace slotPool {
            // List of slots using one fixed tile
            // (byte index, if >= 0x80 the NULL)
            allocate(oneTile, wram7e, 1)

            // List of slots using two fixed tiles
            // (byte index, if >= 0x80 the NULL)
            allocate(twoTiles, wram7e, 1)

            // List of slots using one fixed row
            // (byte index, if >= 0x80 the NULL)
            allocate(oneRow, wram7e, 1)

            // List of slots using two fixed rows
            // (byte index, if >= 0x80 the NULL)
            allocate(twoRows, wram7e, 1)

            // List of unallocated vram tile slots
            // (byte index, if >= 0x80 the NULL)
            allocate(freeTiles, wram7e, 1)

            // List of unallocated vram row slots
            // (byte index, if >= 0x80 the NULL)
            allocate(freeRows, wram7e, 1)
        }
    }

    namespace Palette {
        constant N_PALETTES = 8

        // Table that contains the address/reference count of each palette
        // used by the MetaSprite engine.
        //
        // Converting from slot index to useful values:
        //
        //      OAM Palette ID = slot index / 2
        //      Palette Buffer Offset = slot index * 16
        //
        // Double Linked List Array of structures
        namespace slots {
            constant N_ELEMENTS = N_PALETTES
            constant ARRAY_INCREMENT = 2

            // This assumption allows the MSB of the index to mean NULL
            assert(N_ELEMENTS * 2 <= 128)

            // next item in the list
            // (byte index, >= $80 is null)
            allocate(next, wram7e, N_ELEMENTS * 2)

            // previous item in the list
            // NOT set when in free list
            // (byte index, >= $80 is NULL)
            constant prev = next + 1

            // address of the palette in `MS_PaletteData` block
            // (word address)
            allocate(paletteAddress, wram7e, N_ELEMENTS * 2)

            // Number of times the palette is used
            // (byte)
            allocate(count, wram7e, N_ELEMENTS * 2)
        }

        namespace slotPool {
            // The used list
            // (byte index, >= $80 is null)
            allocateTmpWord(used)

            // The freeList
            constant free = used + 1
        }
    }

    namespace Animation {
        // The speed of the animations in Wait Timer mode
        //
        // This variable is set to 5  = NTSC) or 6 (PAL
        // (word)
        allocateTmpWord(timerSpeed)
    }

    namespace ActionPoint {
        // The position of the current action point.
        //
        // Exposed so an Entity Init routine can determine location of
        // action point (ie, for direction information)
        //
        // The third byte is always zero to allow for efficient retrieval of the
        // yPos in 16 bit accumulator mode.
        // For example (16 bit A):
        //  * xPos: `lda.w MetaSprite.ActionPoint.positionPair.xPos ; and.w #0xff`
        //  * yPos: `lda.w MetaSprite.ActionPoint.positionPair.yPos`
        //
        // NOTE: This variable is only set when an entity or projectile
        //       is spawned at an action point.
        //
        // (int_ms8 pair)
        allocate(positionPair, wram7e, 3)

        // (int_ms8 byte)
        constant positionPair.xPos = positionPair
        // (int_ms8 word)
        constant positionPair.yPos = positionPair + 1

        assert(MetaSprite.Format.ActionPoints.yPos == MetaSprite.Format.ActionPoints.xPos + 1)
    }
}

// Mark the unused word of `MetaSprite.Render.oamXposBuffer` as unused.
evaluate n = 127
while ({n} >= 0) {
    markTmpWord(MetaSprite.Render.oamXposBuffer + {n} * 4 + 2)
    evaluate n = {n} - 1
}

// vim: ft=bass-65816 ts=4 sw=4 et:

