
scope MetaSprite {

    // The MetaSprite entity data
    // MUST BE called inside the BaseEntity struct
    macro EntityData() {
        scope MetaSprite {
            // The frameSet that the metasprite belongs to
            field(frameSet, 2)

            // The state of the MetaSprite's VRAM/Palette allocations
            //
            // 	    %pdiiiiiv
            //
            //      p: palette set
            //      d: dynamic tileset active (MUST NOT be set if vram is clear)
            //      iiiii: vram slot table number (index / 2)
            //      v: vram set
            field(status, 1)

            // Palette id
            field(palette, 1)

            // Frame address
            field(currentFrame, 2)

            // Offset between frame object data and the OAM charattr data
            field(blockOneCharAttrOffset, 2)

            // Offset between frame object data and the OAM charattr data (second block)
            field(blockTwoCharAttrOffset, 2)
        }
    }

    scope StatusFlags {
        constant paletteSet(0x80)
        constant dynamicTileset(0x40)
        constant vramSlotMask(0x3e)
        constant vramSet(0x01)
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
}

// vim: ft=asm ts=4 sw=4 et:

