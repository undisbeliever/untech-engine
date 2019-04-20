// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Don't test palettes were uploaded into buffer
// That is easily tested by using the engine
// but hard to code for.

namespace MetaSpriteTest {
namespace Palette {

allocateTmpWord(tmp0)

a16()
i16()
code()
Test.add("MetaSprite.Palette.Activate")
function Activate {
constant tmp_paletteBits = tmp0
    assert(MetaSprite.StatusFlags.paletteSet == 0x80)

    jsr     _Init_MetaSpriteModule

    lda.w   #EntityPool.entity0
    tcd

    ldy.w   #0
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntityAndActivate
    bcc     Fail

    sep     #$20
a8()
    lda.w   Dma.ObjPalette.transferOnZero
    bne     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero



    // Test that reactivating an entity again doesn't reupload Palette
    rep     #$30
a16()
    jsr     MetaSprite.Activate

    sep     #$20
a8()
    lda.w   Dma.ObjPalette.transferOnZero
    beq     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail


    // Test that creating a new entity with the same palette as before
    // will use the same palette bits

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    sta.w   tmp_paletteBits

    rep     #$30
a16()
    lda.w   #EntityPool.entity1
    tcd

    ldy.w   #0
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntityAndActivate
    bcc     Fail

    sep     #$20
a8()
    lda.w   Dma.ObjPalette.transferOnZero
    beq     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    cmp.w   tmp_paletteBits
    bne     Fail

    // Test that creating a new entity with the different palette as before
    // will use different palette bits
    rep     #$30
a16()
    lda.w   #EntityPool.entity2
    tcd

    ldy.w   #1
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntityAndActivate
    bcc     Fail

    sep     #$20
a8()
    lda.w   Dma.ObjPalette.transferOnZero
    bne     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    cmp.w   tmp_paletteBits
    beq     Fail

    jsr     _ValidateUsedSlotList
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
Test.add("MetaSprite.Palette.SetPalette")
function SetPalette {
constant tmp_paletteBits = tmp0
    jsr     _Init_8Palettes
    bcc     Fail

    // Test that changing the entity's palette does not change pallete ID,
    // but does upload a new palette.

    lda.w   #EntityPool.entity0
    tcd

    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.blockOneCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    sta.w   tmp_paletteBits

    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero

    lda.b   #9
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.ObjPalette.transferOnZero
    bne     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    cmp.w   tmp_paletteBits
    bne     Fail


    // Test that changing the entity's palette to something that already
    // exists will find the existing palette and not upload a new palette.
    lda.w   EntityPool.entity3 + BaseEntity.MetaSprite.blockOneCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    sta.w   tmp_paletteBits

    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero

    lda.b   #3
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.ObjPalette.transferOnZero
    beq     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    cmp.w   tmp_paletteBits
    bne     Fail


    // Test that setting the palette to the same palette id does nothing

    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero

    lda.b   #3
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.ObjPalette.transferOnZero
    beq     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    cmp.w   tmp_paletteBits
    bne     Fail

    jsr     _ValidateUsedSlotList
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
Test.add("MetaSprite.Palette.Deactivate")
function Deactivate {
    jsr     _Init_8Palettes
    bcc     Fail

    lda.w   #EntityPool.entity4
    tcd

    jsr     MetaSprite.Deactivate

    // Test that setting the palette of a deactivated palette
    // does not upload a palette
    sep     #$20
a8()
    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero

    lda.b   #9
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.ObjPalette.transferOnZero
    beq     Fail

    jsr     _ValidateUsedSlotList
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
Test.add("MetaSprite.Palette.DeactivateAndTestList (First)")
function Deactivate_List_First {
    lda.w   #EntityPool.entity0
    bra     _DeactivateAndTestList_A
}


a16()
i16()
code()
Test.add("MetaSprite.Palette.DeactivateAndTestList (Middle)")
function Deactivate_List_Middle {
    lda.w   #EntityPool.entity4
    bra     _DeactivateAndTestList_A
}


a16()
i16()
code()
Test.add("MetaSprite.Palette.DeactivateAndTestList (Last)")
function Deactivate_List_Last {
    lda.w   #EntityPool.entity7
    bra     _DeactivateAndTestList_A
}


// IN: A = entity to remove
a16()
i16()
code()
function _DeactivateAndTestList_A {
allocateTmpWord(tmp_entityToRemove)

    sta.w   tmp_entityToRemove

    jsr     _Init_8Palettes
    bcc     Fail

    lda.w   tmp_entityToRemove
    tcd

    jsr     MetaSprite.Deactivate

    jsr     _ValidateUsedSlotList
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
Test.add("MetaSprite.Palette.Overflow")
function Overflow {
    jsr     _Init_8Palettes
    bcc     Fail

    // Test that overflowing the palettes slots fails

    lda.w   #EntityPool.entity8
    tcd

    ldy.w   #8
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntity
    jsr     _ActivatePalette
    bcs     Fail

    // Test NO_MS_PALETTE_SLOTS warning was emitted
    lda.w   Warnings.current
    cmp.w   #__WARNINGS__.ids.NO_MS_PALETTE_SLOTS
    bne     Fail
    stz.w   Warnings.current


    lda.w   #EntityPool.entity9
    tcd

    ldy.w   #9
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntity
    jsr     _ActivatePalette
    bcs     Fail

    // Test NO_MS_PALETTE_SLOTS warning was emitted
    lda.w   Warnings.current
    cmp.w   #__WARNINGS__.ids.NO_MS_PALETTE_SLOTS
    bne     Fail
    stz.w   Warnings.current


    // The overflow should still fail here
    sep     #$20
a8()
    lda.b   #1
    sta.w   Dma.ObjPalette.transferOnZero

    lda.b   #8
    jsr     MetaSprite.Palette.SetPalette

    rep     #$20
a16()
    jsr     _ActivatePalette
    bcs     Fail


    // Deactivate an already inactive entity should do nothing
    lda.w   #EntityPool.entity9
    tcd
    jsr     MetaSprite.Deactivate

    lda.w   #EntityPool.entity8
    tcd
    jsr     _ActivatePalette
    bcs     Fail

    // Test NO_MS_PALETTE_SLOTS warning was emitted
    lda.w   Warnings.current
    cmp.w   #__WARNINGS__.ids.NO_MS_PALETTE_SLOTS
    bne     Fail
    stz.w   Warnings.current


    // Check that activating a duplicate palette works
    sep     #$20
a8()
    lda.b   #4
    jsr     MetaSprite.Palette.SetPalette

    rep     #$20
a16()
    jsr     _ActivatePalette
    bcs     _Pass
    Fail:
        clc
        rts
    _Pass:


    // Check that activate works after deactivating an existing one
    lda.w   #EntityPool.entity0
    tcd
    jsr     MetaSprite.Deactivate

    lda.w   #EntityPool.entity9
    tcd
    jsr     _ActivatePalette
    bcc     Fail

    jsr     _ValidateUsedSlotList
    bcc     Fail

    sec
    rts


function _ActivatePalette {
    sep     #$20
    rep     #$10
a8()
i16()

    ldx.b   BaseEntity.MetaSprite.frameSet
    jsr     MetaSprite.Palette._ProcessSlots

    rep     #$30
a16()
i16()
    rts

Fail:
    rep     #$31
a16()
i16()
}

}


// Initializes the MetaSprite Engine and
// creates entities 0 - 7, using palettes 0 - 7
a16()
i16()
code()
function _Init_8Palettes {
    constant tmp_palleteId = tmp0

    jsr     _Init_MetaSpriteModule

    stz.w   tmp_palleteId

    lda.w   #EntityPool.entity0

    Loop:
        tcd

        ldy.w   tmp_palleteId
        lda.w   #MSFS.FST_10Palettes
        jsr     _InitEntityAndActivate
        bcc     Return

        inc.w   tmp_palleteId

        tdc
        clc
        adc.w   #Entity.ENTITY_SIZE
        cmp.w   #EntityPool.entity8
        bcc     Loop

    jsr     _ValidateUsedSlotList
    bcc     Return

    sec
Return:
    rts
}


// Loops through the used palette slot list,
// verifies the prev/next pointers match.
// This routine will loop forever if there is a circular reference
au()
iu()
code()
function _ValidateUsedSlotList {
    php

    sep     #$30
a8()
i8()

    ldx.w   MetaSprite.Palette.slotPool.used
    bmi     ReturnTrue

    ldy.b   #0x80

    Loop:
        tya
        cmp.w   MetaSprite.Palette.slots.prev,x
        bne     ReturnFalse

        txy

        lda.w   MetaSprite.Palette.slots.next,x
        tax
        bpl     Loop

ReturnTrue:
    plp
    sec
    rts

ReturnFalse:
    plp
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

