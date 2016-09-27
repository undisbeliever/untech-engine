
// Don't test palettes were uploaded into buffer
// That is easily tested by using the engine
// but hard to code for.

scope MetaSpriteTest {
scope Palette {

allocateTmpWord(tmp0)

code()
a16()
i16()
Test.add("MetaSprite.Palette.Activate")
scope Activate: {
constant tmp_paletteBits(tmp0)
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
    lda.w   Dma.Palette.dirty
    cmp.b   #Dma.Palette.dirty.objects
    bne     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    stz.w   Dma.Palette.dirty



    // Test that reactivating an entity again doesn't reupload Palette
    rep     #$30
a16()
    jsr     MetaSprite.Activate

    sep     #$20
a8()
    lda.w   Dma.Palette.dirty
    bne     Fail

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
    lda.w   Dma.Palette.dirty
    bne     Fail

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
    lda.w   Dma.Palette.dirty
    beq     Fail

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


code()
a16()
i16()
Test.add("MetaSprite.Palette.SetPalette")
scope SetPalette: {
constant tmp_paletteBits(tmp0)
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

    stz.w   Dma.Palette.dirty

    lda.b   #9
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.Palette.dirty
    beq     Fail

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

    stz.w   Dma.Palette.dirty

    lda.b   #3
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.Palette.dirty
    bne     Fail

    lda.b   BaseEntity.MetaSprite.status
    bpl     Fail

    lda.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1
    and.b   #OamFormat.attr.palette.mask
    cmp.w   tmp_paletteBits
    bne     Fail


    // Test that setting the palette to the same palette id does nothing

    stz.w   Dma.Palette.dirty

    lda.b   #3
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.Palette.dirty
    bne     Fail

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


code()
a16()
i16()
Test.add("MetaSprite.Palette.Deactivate")
scope Deactivate: {
    jsr     _Init_8Palettes
    bcc     Fail

    lda.w   #EntityPool.entity4
    tcd

    jsr     MetaSprite.Deactivate

    // Test that setting the palette of a deactivated palette
    // does not upload a palette
    sep     #$20
a8()
    stz.w   Dma.Palette.dirty

    lda.b   #9
    jsr     MetaSprite.Palette.SetPalette
    bcc     Fail

    lda.w   Dma.Palette.dirty
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
Test.add("MetaSprite.Palette.DeactivateAndTestList (First)")
scope Deactivate_List_First: {
    lda.w   #EntityPool.entity0
    bra     _DeactivateAndTestList_A
}


a16()
i16()
code()
Test.add("MetaSprite.Palette.DeactivateAndTestList (Middle)")
scope Deactivate_List_Middle: {
    lda.w   #EntityPool.entity4
    bra     _DeactivateAndTestList_A
}


a16()
i16()
code()
Test.add("MetaSprite.Palette.DeactivateAndTestList (Last)")
scope Deactivate_List_Last: {
    lda.w   #EntityPool.entity7
    bra     _DeactivateAndTestList_A
}

// IN: A = entity to remove
a16()
i16()
code()
scope _DeactivateAndTestList_A: {
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


code()
a16()
i16()
Test.add("MetaSprite.Palette.Overflow")
scope Overflow: {
    jsr     _Init_8Palettes
    bcc     Fail

    // Test that overflowing the palettes slots fails

    lda.w   #EntityPool.entity8
    tcd

    ldy.w   #8
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntityAndActivate
    bcs     Fail


    lda.w   #EntityPool.entity9
    tcd

    ldy.w   #9
    lda.w   #MSFS.FST_10Palettes
    jsr     _InitEntityAndActivate
    bcs     Fail

    // The overflow should still fail here
    sep     #$20
a8()
    stz.w   Dma.Palette.dirty

    lda.b   #8
    jsr     MetaSprite.Palette.SetPalette

    rep     #$20
a16()
    jsr     MetaSprite.Activate
    bcs     Fail


    // Deactivate an already inactive entity should do nothing
    lda.w   #EntityPool.entity9
    tcd
    jsr     MetaSprite.Deactivate

    lda.w   #EntityPool.entity8
    tcd
    jsr     MetaSprite.Activate
    bcs     Fail


    // Check that activating a duplicate palette works
    sep     #$20
a8()
    lda.b   #4
    jsr     MetaSprite.Palette.SetPalette

    rep     #$20
a16()
    jsr     MetaSprite.Activate
    bcc     Fail


    // Check that activate works after deactivating an existing one
    lda.w   #EntityPool.entity0
    tcd
    jsr     MetaSprite.Deactivate

    lda.w   #EntityPool.entity9
    tcd
    jsr     MetaSprite.Activate
    bcc     Fail

    jsr     _ValidateUsedSlotList
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}


// Initializes the MetaSprite Engine and
// creates entities 0 - 7, using palettes 0 - 7
code()
a16()
i16()
scope _Init_8Palettes: {
    constant tmp_palleteId(tmp0)

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
code()
scope _ValidateUsedSlotList: {
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

