// metasprite/_variables.inc
// =========================
//
// Variables used by the MetaSprite module.


scope MetaSprite {

    // The MetaSprite entity data
    // MUST BE called inside the BaseEntity struct
    macro EntityData() {
        // The frameSet that the metasprite belongs to
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

        // Frame address
        field(MetaSprite.currentFrame, 2)

        // Offset between frame object data and the OAM charattr data
        field(MetaSprite.blockOneCharAttrOffset, 2)

        // Offset between frame object data and the OAM charattr data (second block)
        field(MetaSprite.blockTwoCharAttrOffset, 2)

        // Current animation Id
        field(MetaSprite.Animation.id, 1)

        // The animation bytecode program counter.
        //
        // If NULL (0) then there is no animation.
        //
        // (word addr)
        field(MetaSprite.Animation.pc, 2)

        // Delay state for processing next animation bytecode
        //
        //      fnnnnnnn nnnnnnnn
        //
        //  f = If true then MS Frame has not been set.
        //      Set MS Frame on next animation process.
        //  n = current delay value.
        //      Value depends on the animation instruction being processed.
        field(MetaSprite.Animation.delay, 2)
    }

    scope StatusFlags {
        constant paletteSet(0x80)
        constant dynamicTileset(0x40)
        constant vramSlotMask(0x3e)
        constant vramSet(0x01)
    }

    scope Render {
        allocate(oamBuffer, wram7e, 128 * 4)
        scope oamBuffer {
            constant xPos(oamBuffer + OamFormat.xPos)
            constant yPos(oamBuffer + OamFormat.yPos)
            constant charAttr(oamBuffer + OamFormat.charAttr)
        }

        allocate(oamHiBuffer, wram7e, 128 / 4)

        // Buffer the oam's x8/size bits
        // bits 10-15 of each 4 byte block MUST BE CLEAR
        //
        // bits 16-31 are unused by the renderer and can be used elsewhere
        allocate(oamXposBuffer, wram7e, 128 * 4)

        scope {
            // Mark the unused word of `oamXposBuffer` as unused.
            evaluate n(127)
            while ({n} >= 0) {
                markTmpWord(oamXposBuffer + {n} * 4 + 2)
                evaluate n({n} - 1)
            }
        }

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


    scope Vram {
        constant VRAM_TILE_SLOTS(16)
        constant VRAM_ROW_SLOTS(14)

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
        scope slots {
            constant N_ELEMENTS(VRAM_TILE_SLOTS + VRAM_ROW_SLOTS)
            constant ARRAY_INCREMENT(2)
            constant _sectionSize(N_ELEMENTS * ARRAY_INCREMENT)

            // All indexes at or above this value are Row slots
            constant ROW_SLOTS_MIN_INDEX(ARRAY_INCREMENT * VRAM_TILE_SLOTS)


            // This assumption allows the MSB of the index to mean NULL
            assert(N_ELEMENTS * ARRAY_INCREMENT <= 128)

            // Index of the next slot.
            // This is NOT set if the slot is dynamic or the second half of a dual slot
            // (byte index, if >= 0x80 then NULL)
            allocate(next, wram7e, N_ELEMENTS * 2)

            // Index of the previous slot.
            // This is ONLY set if in a fixed tileset
            // (byte index, if >= 0x80 then NULL)
            constant prev(next + 1)

            // Address of the tileset in the `DMA_Tile16Data` block
            // (word address)
            allocate(tileset, wram7e, N_ELEMENTS * 2)

            // Number of times the slot is used
            // (uint8)
            allocate(count, wram7e, N_ELEMENTS * 2)

            // Index of the second slot (if in a two slot type)
            // (byte index, if >= 0x80 then NULL
            constant pair(count + 1)
        }

        scope slotPool {
            // List of slots using one fixed tile
            // (byte index, if >= 0x80 the NULL)
            allocateTmpWord(oneTile)

            // List of slots using two fixed tiles
            // (byte index, if >= 0x80 the NULL)
            constant twoTiles(oneTile + 1)

            // List of slots using one fixed row
            // (byte index, if >= 0x80 the NULL)
            allocateTmpWord(oneRow)

            // List of slots using two fixed rows
            // (byte index, if >= 0x80 the NULL)
            constant twoRows(oneRow + 1)

            // List of unallocated vram tile slots
            // (byte index, if >= 0x80 the NULL)
            allocateTmpWord(freeTiles)

            // List of unallocated vram row slots
            // (byte index, if >= 0x80 the NULL)
            allocateTmpWord(freeRows)
        }
    }

    scope Palette {
        constant N_PALETTES(8)

        // Table that contains the address/reference count of each palette
        // used by the MetaSprite engine.
        //
        // Converting from slot index to useful values:
        //
        //      OAM Palette ID = slot index / 2
        //      Palette Buffer Offset = slot index * 16
        //
        // Double Linked List Array of structures
        scope slots {
            constant N_ELEMENTS(N_PALETTES)
            constant ARRAY_INCREMENT(2)

            // This assumption allows the MSB of the index to mean NULL
            assert(N_ELEMENTS * 2 <= 128)

            // next item in the list
            // (byte index, >= $80 is null)
            allocate(next, wram7e, N_ELEMENTS * 2)

            // previous item in the list
            // NOT set when in free list
            // (byte index, >= $80 is NULL)
            constant prev(next + 1)

            // address of the palette in `MS_PaletteData` block
            // (word address)
            allocate(paletteAddress, wram7e, N_ELEMENTS * 2)

            // Number of times the palette is used
            // (byte)
            allocate(count, wram7e, N_ELEMENTS * 2)
        }

        scope slotPool {
            // The used list
            // (byte index, >= $80 is null)
            allocateTmpWord(used)

            // The freeList
            constant free(used + 1)
        }
    }

    scope Animation {
        // The speed of the animations in Wait Timer mode
        //
        // This variable is set to 5 (NTSC) or 6 (PAL)
        // (word)
        allocateTmpWord(timerSpeed)
    }

    scope ActionPoint {
        // The address of the last action Point that was processed.
        //
        // Exposed so an Entity Init routine can determine location of
        // action point (ie, for direction information)
        //
        // (index, MUST NOT be modified outside ActionPoint routines)
        allocateTmpWord(address)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

