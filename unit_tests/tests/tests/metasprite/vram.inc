
// Don't test that the correct tiles are uploaded into dma buffer
// That is easily tested by using the engine but hard to code for.

// These tests work because a dynamic tileset has vram allocated
// even if it has no current frame.

scope MSFS {
    constant FST_OneRow(Woman)
    constant FST_OneRowFixed(SpikedWalker)
    constant FST_TwoRowsFixed(Man)
}

scope MetaSpriteTest {
scope Vram {

macro _ActivateFixedTest(FrameSet, nSlots) {
    code()
    a16()
    i16()
    Test.add("MetaSprite.Vram.Activate ({FrameSet})")
    scope Activate_{FrameSet}: {
        jsr     _Init_MetaSpriteModule

        lda.w   #Entities.entity0
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // Test vramSet set and dynamic clear
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.vramSet
        bne     Fail

        ldx.w   Dma.Tile16.tableIndex
        cpx.w   #{nSlots} * 2
        bne     Fail


        // Second entity
        lda.w   #Entities.entity1
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // Test that no extra uploads were done
        ldx.w   Dma.Tile16.tableIndex
        cpx.w   #{nSlots} * 2
        bne     Fail

        // Test vramSet set and dynamic clear
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.vramSet
        bne     Fail

        // Test that both entities use same charAttr
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        cmp.w   Entities.entity0 + BaseEntity.MetaSprite.blockOneCharAttrOffset
        bne     Fail

        if {nSlots} == 2 {
            lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset
            cmp.w   Entities.entity0 + BaseEntity.MetaSprite.blockTwoCharAttrOffset
            bne     Fail
        }

        sec
        rts

    Fail:
        clc
        rts
    }
}

_ActivateFixedTest(FST_OneTileFixed, 1)
_ActivateFixedTest(FST_TwoTilesFixed, 2)
_ActivateFixedTest(FST_OneRowFixed, 1)
_ActivateFixedTest(FST_TwoRowsFixed, 2)


macro _ActivateDynamicTest(FrameSet, nSlots) {
    code()
    a16()
    i16()
    Test.add("MetaSprite.Vram.Activate ({FrameSet})")
    scope Activate_{FrameSet}: {
        jsr     _Init_MetaSpriteModule

        lda.w   #Entities.entity0
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0Palette

        lda.w   #0
        jsr     MetaSprite.SetFrame
        bcc     Fail

        jsr     MetaSprite.Activate
        bcc     Fail

        // Test vramSet and dynamic set
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        bne     Fail

        ldx.w   Dma.Tile16.tableIndex
        cpx.w   #{nSlots} * 2
        bne     Fail


        // Second entity
        // NULL frame upload
        lda.w   #Entities.entity1
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcc     Fail

        // Test that no upload was done (because current frame == NULL)
        ldx.w   Dma.Tile16.tableIndex
        cpx.w   #{nSlots} * 2
        bne     Fail

        // Test vramSet and dynamic set
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        bne     Fail

        // Test that both entities use different charAttr
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        cmp.w   Entities.entity0 + BaseEntity.MetaSprite.blockOneCharAttrOffset
        beq     Fail

        if {nSlots} == 2 {
            lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset
            cmp.w   Entities.entity0 + BaseEntity.MetaSprite.blockTwoCharAttrOffset
            beq     Fail
        }

        sec
        rts

    Fail:
        clc
        rts
    }
}

_ActivateDynamicTest(FST_OneTile, 1)
_ActivateDynamicTest(FST_TwoTiles, 2)
_ActivateDynamicTest(FST_OneRow, 1)
_ActivateDynamicTest(FST_TwoRows, 2)



macro _DeallocateTest(type) {
    code()
    a16()
    i16()
    Test.add("MetaSprite.Vram.Deallocate ({type})")
    scope Deallocate_{type}: {
        jsr     _Init_Fill{type}
        bcc     Fail

        // Try to create a new entity, even though the slots are full
        // This should fail


        lda.w   #Entities.entity10
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcs     Fail

        // Now if I remove the double fixed tile the activate will succeed
        lda.w   #Entities.entity1
        tcd
        jsr     MetaSprite.Deactivate

        lda.w   #Entities.entity10
        tcd
        jsr     MetaSprite.Activate
        bcs     _Pass
        Fail:
            clc
            rts
        _Pass:


        // Now there is only one free slot
        // Trying to activate a dual slot will fail

        lda.w   #Entities.entity11
        tcd
        lda.w   #MSFS.FST_Two{type}s
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcs     Fail


        // Now if I remove a single dynamic tile the activate will succeed
        lda.w   #Entities.entity2
        tcd
        jsr     MetaSprite.Deactivate

        lda.w   #Entities.entity11
        tcd
        jsr     MetaSprite.Activate
        bcc     Fail


        // Now there is no free slots

        lda.w   #Entities.entity12
        tcd
        lda.w   #MSFS.FST_Two{type}s
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcs     Fail


        // Deactivate single fixed tileset
        // And try a double free
        lda.w   #Entities.entity0
        tcd
        jsr     MetaSprite.Deactivate
        jsr     MetaSprite.Deactivate


        // One tile free - still fail
        lda.w   #Entities.entity12
        tcd
        jsr     MetaSprite.Activate
        bcs     Fail


        // Finally deactivate a double dynamic
        lda.w   #Entities.entity6
        tcd
        jsr     MetaSprite.Deactivate
        jsr     MetaSprite.Deactivate


        // Finally create single slot
        lda.w   #Entities.entity12
        tcd
        jsr     MetaSprite.Activate
        bcc     Fail


        // Fill the other one
        lda.w   #Entities.entity13
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail


        // We are full again
        // This will fail
        lda.w   #Entities.entity14
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     Fail

        sec
        rts
    }
}
_DeallocateTest(Tile)
_DeallocateTest(Row)


macro _ReferenceCountTest(type) {
    code()
    a16()
    i16()
    Test.add("MetaSprite.Vram - ReferenceCount ({type})")
    scope ReferenceCount_{type}: {
        jsr     _Init_Fill{type}
        bcc     Fail

        // entity 1: single fixed, count = 1

        lda.w   #Entities.entity10
        tcd
        lda.w   #MSFS.FST_One{type}Fixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // entity 0: single fixed, count = 2

        lda.w   #Entities.entity11
        tcd
        lda.w   #MSFS.FST_One{type}Fixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // entity 0: single fixed, count = 3

        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 2

        lda.w   #Entities.entity10
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 1


        // Try and fail to create a dynamic entity (we're full)
        lda.w   #Entities.entity14
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     Fail

        lda.w   #Entities.entity0
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 0

        // Can now create entity
        lda.w   #Entities.entity14
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail


        // entity 1: double fixed, count = 1

        lda.w   #Entities.entity10
        tcd
        lda.w   #MSFS.FST_Two{type}sFixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // entity 0: single fixed, count = 2

        // Try and fail to create a dynamic entity (we're full)
        lda.w   #Entities.entity13
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     Fail


        lda.w   #Entities.entity10
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 1

        lda.w   #Entities.entity1
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 0

        // Can now create entity
        lda.w   #Entities.entity13
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail


        sec
        rts
    Fail:
        clc
        rts
    }
}
_ReferenceCountTest(Tile)
_ReferenceCountTest(Row)


// initializes the metasprite engine and fills all the tile slots.
//
// Entity 0 uses a single fixed slots
// Entity 1 uses two fixed slots
// Entity 2 uses a single dynamic slot
// Entities 3-(8/9) uses two dynamic slots
macro __Init_Fill(type, nSlots) {
    code()
    a16()
    i16()
    scope _Init_Fill{type}: {
        jsr     _Init_MetaSpriteModule

        lda.w   #Entities.entity0
        tcd
        lda.w   #MSFS.FST_One{type}Fixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #Entities.entity1
        tcd
        lda.w   #MSFS.FST_Two{type}sFixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #Entities.entity2
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #Entities.entity3
        Loop:
            tcd

            lda.w   #MSFS.FST_Two{type}s
            jsr     _InitEntity0PaletteAndActivate
            bcc     Fail

            tdc
            clc
            adc.w   #Entities.ENTITY_SIZE
            cmp.w   #Entities.entity3 + Entities.ENTITY_SIZE * ({nSlots} - 4) / 2
            bcc     Loop

        sec
        rts

    Fail:
        clc
        rts
    }
}
__Init_Fill(Tile, MetaSprite.Vram.VRAM_TILE_SLOTS)
__Init_Fill(Row, MetaSprite.Vram.VRAM_ROW_SLOTS)

}
}

// vim: ft=asm ts=4 sw=4 et:

