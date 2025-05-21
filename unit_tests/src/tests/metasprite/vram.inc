// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Don't test that the correct tiles are uploaded into dma buffer
// That is easily tested by using the engine but hard to code for.

// These tests work because a dynamic tileset has vram allocated
// even if it has no current frame.

namespace MSFS {
    constant FST_OneRow = Woman
    constant FST_OneTileFixed = FST_OneTileFixed1
    constant FST_OneRowFixed = FST_OneRowFixed1
    constant FST_TwoTilesFixed = FST_TwoTilesFixed1
    constant FST_TwoRowsFixed = FST_TwoRowsFixed1
}

namespace MetaSpriteTest {
namespace Vram {

allocateTmpWord(tmp0)


inline _ActivateFixedTest(FrameSet, nSlots) {
    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.Activate ({FrameSet})")
    function Activate_{FrameSet} {
        jsr     _Init_MetaSpriteModule

        lda.w   #EntityPool.entity0
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // Test vramSet set and dynamic clear
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.vramSet
        bne     Fail

        ldx.w   Dma.Tile16.transferQueueIndex
        cpx.w   #{nSlots} * 2
        bne     Fail


        // Second entity
        lda.w   #EntityPool.entity1
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // Test that no extra uploads were done
        ldx.w   Dma.Tile16.transferQueueIndex
        cpx.w   #{nSlots} * 2
        bne     Fail

        // Test vramSet set and dynamic clear
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.vramSet
        bne     Fail

        // Test that both entities use same charAttr
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        cmp.w   EntityPool.entity0 + BaseEntity.MetaSprite.blockOneCharAttrOffset
        bne     Fail

        if {nSlots} == 2 {
            lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset
            cmp.w   EntityPool.entity0 + BaseEntity.MetaSprite.blockTwoCharAttrOffset
            bne     Fail
        }

        jsr     _ValidateUsedSlotLists
        bcc     Fail

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


inline _ActivateDynamicTest(FrameSet, nSlots, staticUploads) {
    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.Activate ({FrameSet})")
    function Activate_{FrameSet} {
        jsr     _Init_MetaSpriteModule

        lda.w   #EntityPool.entity0
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

        ldx.w   Dma.Tile16.transferQueueIndex
        cpx.w   #({nSlots} + {staticUploads}) * 2
        bne     Fail

        jsl     WaitFrame__far

        // Second entity
        // also a frame 0 upload
        lda.w   #EntityPool.entity1
        tcd

        lda.w   #MSFS.{FrameSet}
        jsr     _InitEntity0Palette

        lda.w   #0
        jsr     MetaSprite.SetFrame
        bcc     Fail

        jsr     MetaSprite.Activate
        bcc     Fail

        // Test that the tiles were uploaded (if exist)
        ldx.w   Dma.Tile16.transferQueueIndex
        cpx.w   #({nSlots} + {staticUploads}) * 2
        bne     Fail

        // Test vramSet and dynamic set
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        bne     Fail

        // Test that both entities use different charAttr
        lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        cmp.w   EntityPool.entity0 + BaseEntity.MetaSprite.blockOneCharAttrOffset
        beq     Fail

        if {nSlots} == 2 {
            lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset
            cmp.w   EntityPool.entity0 + BaseEntity.MetaSprite.blockTwoCharAttrOffset
            beq     Fail
        }

        jsr     _ValidateUsedSlotLists
        bcc     Fail

        sec
        rts

    Fail:
        clc
        rts
    }
}

_ActivateDynamicTest(FST_OneTile, 1, 0)
_ActivateDynamicTest(FST_TwoTiles, 1, 1)
_ActivateDynamicTest(FST_OneRow, 0, 1)  // frame 0 has no dynamic tiles
_ActivateDynamicTest(FST_TwoRows, 2, 1)



inline _DeallocateTest(type) {
    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.Deallocate ({type})")
    function Deallocate_{type} {
        jsr     _Init_Fill{type}
        bcc     Fail

        // Try to create a new entity, even though the slots are full
        // This should fail


        lda.w   #EntityPool.entity10
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcs     Fail

        // Test NO_MS_VRAM_SLOTS warning was emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current

        // Now if I remove the double fixed tile the activate will succeed
        lda.w   #EntityPool.entity1
        tcd
        jsr     MetaSprite.Deactivate

        lda.w   #EntityPool.entity10
        tcd
        jsr     MetaSprite.Activate
        bcc     Fail


        // Now there is only one free slot
        // Trying to activate a dual slot will fail

        lda.w   #EntityPool.entity11
        tcd
        lda.w   #MSFS.FST_Two{type}s
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcs     Fail

        // Confirm that NO_MS_VRAM_SLOTS warning has been emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current

        // Now if I remove a single dynamic tile the activate will succeed
        lda.w   #EntityPool.entity2
        tcd
        jsr     MetaSprite.Deactivate

        lda.w   #EntityPool.entity11
        tcd
        jsr     MetaSprite.Activate
        bcs     _Pass
        Fail:
            clc
            rts
        _Pass:


        // Now there is no free slots

        lda.w   #EntityPool.entity12
        tcd
        lda.w   #MSFS.FST_Two{type}s
        jsr     _InitEntity0Palette

        jsr     MetaSprite.Activate
        bcs     Fail

        // Test NO_MS_VRAM_SLOTS warning was emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current


        // Deactivate single fixed tileset
        // And try a double free
        lda.w   #EntityPool.entity0
        tcd
        jsr     MetaSprite.Deactivate
        jsr     MetaSprite.Deactivate


        // One tile free - still fail
        lda.w   #EntityPool.entity12
        tcd
        jsr     MetaSprite.Activate
        bcs     Fail

        // Test NO_MS_VRAM_SLOTS warning was emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current


        // Finally deactivate a double dynamic
        lda.w   #EntityPool.entity6
        tcd
        jsr     MetaSprite.Deactivate
        jsr     MetaSprite.Deactivate


        // Must wait one frame to free the DMA slots
        jsl     WaitFrame__far

        // Finally create single slot
        lda.w   #EntityPool.entity12
        tcd
        jsr     MetaSprite.Activate
        bcc     Fail


        // Fill the other one
        lda.w   #EntityPool.entity13
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail


        // We are full again
        // This will fail
        lda.w   #EntityPool.entity14
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     Fail

        // Test NO_MS_VRAM_SLOTS warning was emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current

        jsr     _ValidateUsedSlotLists
        bcc     Fail

        sec
        rts
    }
}
_DeallocateTest(Tile)
_DeallocateTest(Row)



// Delete an entities start/middle/end of the list
// and validate linked list is setup correctly
inline _DeallocateAndTestListTest(type) {
    // IN: A = entity to delete
    a16()
    i16()
    code()
    function _DeallocateAndTestList_{type} {
    constant tmp_entityToRemove = tmp0

        sta.w   tmp_entityToRemove

        jsr     _Init_MetaSpriteModule

        lda.w   #EntityPool.entity0
        tcd
        lda.w   #MSFS.FST_{type}Fixed1
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #EntityPool.entity1
        tcd
        lda.w   #MSFS.FST_{type}Fixed2
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // needed to prevent DMA buffer overflowing
        jsr     Dma.Init

        lda.w   #EntityPool.entity2
        tcd
        lda.w   #MSFS.FST_{type}Fixed3
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   tmp_entityToRemove
        tcd
        jsr     MetaSprite.Deactivate
        jsr     _ValidateUsedSlotLists
        bcc     Fail

        sec
        rts

    Fail:
        clc
        rts
    }

    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.DeallocateAndTestList ({type}, First)")
    function DeallocateAndTestList_{type}_First {
        lda.w   #EntityPool.entity0
        jmp     _DeallocateAndTestList_{type}
    }

    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.DeallocateAndTestList ({type}, Middle)")
    function DeallocateAndTestList_{type}_Middle {
        lda.w   #EntityPool.entity1
        jmp     _DeallocateAndTestList_{type}
    }

    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.DeallocateAndTestList ({type}, Last)")
    function DeallocateAndTestList_{type}_Last {
        lda.w   #EntityPool.entity2
        jmp     _DeallocateAndTestList_{type}
    }
}
_DeallocateAndTestListTest(OneTile)
_DeallocateAndTestListTest(OneRow)
_DeallocateAndTestListTest(TwoTiles)
_DeallocateAndTestListTest(TwoRows)


inline _ReferenceCountTest(type) {
    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram - ReferenceCount ({type})")
    function ReferenceCount_{type} {
        jsr     _Init_Fill{type}
        bcc     Fail

        // entity 1: single fixed, count = 1

        lda.w   #EntityPool.entity10
        tcd
        lda.w   #MSFS.FST_One{type}Fixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // entity 0: single fixed, count = 2

        lda.w   #EntityPool.entity11
        tcd
        lda.w   #MSFS.FST_One{type}Fixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // entity 0: single fixed, count = 3

        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 2

        lda.w   #EntityPool.entity10
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 1


        // Try and fail to create a dynamic entity (we're full)
        lda.w   #EntityPool.entity14
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     Fail

        // Test NO_MS_VRAM_SLOTS warning was emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current


        lda.w   #EntityPool.entity0
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 0

        // Can now create entity
        lda.w   #EntityPool.entity14
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     _Pass
        Fail:
            clc
            rts
        _Pass:


        // entity 1: double fixed, count = 1

        lda.w   #EntityPool.entity10
        tcd
        lda.w   #MSFS.FST_Two{type}sFixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // Prevent dma buffer from overflowing
        jsl     WaitFrame__far

        // entity 0: single fixed, count = 2

        // Try and fail to create a dynamic entity (we're full)
        lda.w   #EntityPool.entity13
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcs     Fail

        // Test NO_MS_VRAM_SLOTS warning was emitted
        lda.w   Warnings.current
        cmp.w   #__WARNINGS__.ids.NO_MS_VRAM_SLOTS
        bne     Fail
        stz.w   Warnings.current


        lda.w   #EntityPool.entity10
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 1

        lda.w   #EntityPool.entity1
        tcd
        jsr     MetaSprite.Deactivate

        // entity 0: single fixed, count = 0

        // Can now create entity
        lda.w   #EntityPool.entity13
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        jsr     _ValidateUsedSlotLists
        bcc     Fail

        sec
        rts
    }
}
_ReferenceCountTest(Tile)
_ReferenceCountTest(Row)


// There is a bug that prevents the third fixed VRAM slot from
// buffering the tiles correctly.
inline _ActivateThirdFixedSlot(name, FrameSet1, FrameSet2, FrameSet3) {
    a16()
    i16()
    code()
    Test.add("MetaSprite.Vram.ActivateThirdSlot ({name})")
    function ActivateThirdFixedSlot_{name} {
        jsr     _Init_MetaSpriteModule

        lda.w   #EntityPool.entity0
        tcd

        lda.w   #MSFS.{FrameSet1}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #EntityPool.entity1
        tcd

        lda.w   #MSFS.{FrameSet2}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        // tests that the tiles of the third slot are buffered
        jsr     Dma.Init
        ldx.w   Dma.Tile16.transferQueueIndex
        phx
            lda.w   #EntityPool.entity2
            tcd

            lda.w   #MSFS.{FrameSet3}
            jsr     _InitEntity0PaletteAndActivate
            bcc     Fail
        plx

        // Test vramSet set and dynamic clear
        lda.b   BaseEntity.MetaSprite.status
        and.w   #MetaSprite.StatusFlags.dynamicTileset | MetaSprite.StatusFlags.vramSet
        cmp.w   #MetaSprite.StatusFlags.vramSet
        bne     Fail

        cpx.w   Dma.Tile16.transferQueueIndex
        beq     Fail

        jsr     _ValidateUsedSlotLists
        bcc     Fail

        sec
        rts

    Fail:
        clc
        rts
    }
}
_ActivateThirdFixedSlot(OneTile, FST_OneTileFixed, FST_OneTileFixed2, FST_OneTileFixed3)
_ActivateThirdFixedSlot(TwoTiles, FST_TwoTilesFixed, FST_TwoTilesFixed2, FST_TwoTilesFixed3)
_ActivateThirdFixedSlot(OneRow, FST_OneRowFixed, FST_OneRowFixed2, FST_OneRowFixed3)
_ActivateThirdFixedSlot(TwoRows, FST_TwoRowsFixed, FST_TwoRowsFixed2, FST_TwoRowsFixed3)




// initializes the metasprite engine and fills all the tile slots.
//
// Entity 0 uses a single fixed slots
// Entity 1 uses two fixed slots
// Entity 2 uses a single dynamic slot
// Entities 3-(8/9) uses two dynamic slots
inline __Init_Fill(type, nSlots) {
    a16()
    i16()
    code()
    function _Init_Fill{type} {
        jsr     _Init_MetaSpriteModule

        lda.w   #EntityPool.entity0
        tcd
        lda.w   #MSFS.FST_One{type}Fixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #EntityPool.entity1
        tcd
        lda.w   #MSFS.FST_Two{type}sFixed
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        jsr     Dma.Init

        lda.w   #EntityPool.entity2
        tcd
        lda.w   #MSFS.FST_One{type}
        jsr     _InitEntity0PaletteAndActivate
        bcc     Fail

        lda.w   #EntityPool.entity3
        Loop:
            tcd

            lda.w   #MSFS.FST_Two{type}s
            jsr     _InitEntity0PaletteAndActivate
            bcc     Fail

            jsr     Dma.Init

            tdc
            clc
            adc.w   #Entity.ENTITY_SIZE
            cmp.w   #EntityPool.entity3 + Entity.ENTITY_SIZE * ({nSlots} - 4) / 2
            bcc     Loop

        jsr     _ValidateUsedSlotLists
        bcc     Fail

        sec
        rts

    Fail:
        clc
        rts
    }
}
__Init_Fill(Tile, MetaSprite.Vram.VRAM_TILE_SLOTS)
__Init_Fill(Row, MetaSprite.Vram.VRAM_ROW_SLOTS)


// Loops through the used vram slots lists,
// verifies the prev/next pointers match.
// This routine will loop forever if there is a circular reference
au()
iu()
code()
function _ValidateUsedSlotLists {
    macro Test(list) {
        assert8a()
        assert8i()

        ldx.w   MetaSprite.Vram.slotPool.{list}
        bmi     +
            jsr     _TestList
            bcc     ReturnFalse
        +
    }

    php

    sep     #$30
a8()
i8()
    Test(oneTile)
    Test(twoTiles)
    Test(oneRow)
    Test(twoRows)

    plp
    sec
    rts

ReturnFalse:
    plp
    clc
    rts



    // IN: X = first list Index, must not be NULL
    a8()
    i8()
    code()
    function _TestList {
        ldy.b   #0x80

        Loop:
            tya
            cmp.w   MetaSprite.Vram.slots.prev,x
            bne     ReturnFailure

            txy

            lda.w   MetaSprite.Vram.slots.next,x
            tax
            bpl     Loop

        sec
        rts

    ReturnFailure:
        clc
        rts
    }
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

