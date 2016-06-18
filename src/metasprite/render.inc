
// The Renderer assumes:
//	* The sprites are 8x8 and 16x16
//	* The screen size is 256x224
//
//
// The handling of the OAM hi-Table was inspired by psycopathicteen[1].
//
// The MetaSprite block*CharAttrOffset variales are designed to allow the
// MetaSprite VRAM allocator to allocate and deallocate 2 VRAM Rows/Tiles
// without worrying about fragmentation.
//
// [1] http://forums.nesdev.com/viewtopic.php?p=92234#p92234

scope MetaSprite {
scope Render {

constant SCREEN_WIDTH(256)
constant SCREEN_HEIGHT(224)

// Resets the oamXposBuffer, prevent possible bugs
//
// REQUITES: 16 bit A, 16 bit Index, DB = $7e
macro _Init() {
	assert16a()
	assert16i()

    scope Render: {
        ldx.w   #127 * 4

        Loop:
            stz.w   MetaSprite.Render.oamXposBuffer,x
            dex
            dex
            dex
            dex
            bpl     Loop

        ldy.w   #128 * 4
        sty.w   MetaSprite.Render.previousBufferPos
    }
}


// Start the render loop
//
// REQUITES: 16 bit A, DB = $7e
code()
a16()
scope InitLoop: {
    stz.w   MetaSprite.Render.bufferPos
    Dma.MetaSprite._InitLoop()

Return:
    rts
}



// Render a Entity.
//
// REQUITES: 16 bit A, 16 bit Index, DB = $7e
//
// ASSUMES: Entity has a frameSet
//          Entity has a tileset and palette loaded into the PPU
//          Screen is `SCREEN_HEIGHT` pixels tall
//
// INPUT: DP: Entity
//        xPos: entity xPos - INT_MS8_OFFSET
//        yPos: entity yPos - INT_MS8_OFFSET
code()
a16()
i16()
scope RenderEntity: {
constant Return(InitLoop.Return)

    ldx.b   BaseEntity.MetaSprite.currentFrame
    beq     Return

End:
}


// Render a specific frame in a fixed tileset Entity.
//
// ASSUMES: Entity has a frameSet
//          Entity has the frame's tileset loaded in VRAM
//          Entity had a palette loaded into the PPU
//          Screen is `SCREEN_HEIGHT` pixels tall
//
// REQUITES: 16 bit A, 16 bit Index, DB = $7e
//
// INPUT: DP: Entity
//        X: frame address
//        xPos: entity xPos - INT_MS8_OFFSET
//        yPos: entity yPos - INT_MS8_OFFSET
code()
a16()
i16()
scope RenderEntityFrame: {
    assert(pc() == RenderEntity.End)

allocateTmpWord(nObjectsLeft)
constant Return(InitLoop.Return)

    lda.l   MetaSprite.Format.Frame.frameObjects,x
    beq     Return
    tax

    lda.l   MetaSprite.Format.FrameObjects.count,x
    and.w   #0x001f
    beq     Return
    sta.w   nObjectsLeft

    // Do a single buffer overflow check
    asl
    asl
    // carry clear
    adc.w   bufferPos
    cmp.w   #128 * 4 + 1
    bcs     Return

    ldy.w   bufferPos

    Loop:
        clc
        lda.l   MetaSprite.Format.FrameObjects.Object.charAttr,x
        bit.w   #MetaSprite.Format.FrameObjects.Object.charAttr.blockBit
        beq     +
            and.w   #MetaSprite.Format.FrameObjects.Object.charAttr.mask
            // carry clear
            adc.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset
            bra     ++
        +
            and.w   #MetaSprite.Format.FrameObjects.Object.charAttr.mask
            // carry clear
            adc.b   BaseEntity.MetaSprite.blockOneCharAttrOffset
        +
        sta.w   oamBuffer.charAttr,y

        lda.l   MetaSprite.Format.FrameObjects.Object.xOffset,x
        and.w   #0x00ff
        clc
        adc.w   xPos

        // don't render if xPos <= -16 || xpos >= SCREEN_WIDTH
        cmp.w   #-16 + 1
        bcs     +
            cmp.w   #SCREEN_WIDTH
            bcs     SkipSprite
        +

        sta.w   oamBuffer.xPos,y

        // store x8/size in oamXposBuffer
        // this works because FO.Object.charAttr size bit matches
        // oamXposBuffer size bit
        ora.l   MetaSprite.Format.FrameObjects.Object.charAttr,x
        and.w   #0x0300
        sta.w   oamXposBuffer,y


        lda.l   MetaSprite.Format.FrameObjects.Object.yOffset,x
        and.w   #0x00ff
        clc
        adc.w   yPos

        // don't render if yPos <= -16 || ypos >= SCREEN_HEIGHT
        cmp.w   #-16 + 1
        bcs     +
            cmp.w   #SCREEN_HEIGHT
            bcs     SkipSprite
        +

        sep     #$20
    a8()
        sta.w   oamBuffer.yPos,y

        iny
        iny
        iny
        iny

        inx
        inx
        inx
        inx

    SkipSprite:
        dec.w   nObjectsLeft
        rep     #$30
    a16()
        bne     Loop

    sty.w   bufferPos
    rts
}



// Finalize render loop.
//
// Convert oamXposBuffer to oamHiBuffer,
// ensure unused sprites are offscreen
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
code()
a16()
i16()
scope EndLoop: {
    lda.w   bufferPos
    beq     SkipHiTable

    dec
    and.w   #0xfff0
    tax
    lsr
    lsr
    lsr
    lsr
    tay

    Loop:
        sep     #$20
    a8()
        lda.w   oamXposBuffer + 3 * OamFormat.size + 1,x
        asl
        asl
        ora.w   oamXposBuffer + 2 * OamFormat.size + 1,x
        asl
        asl
        ora.w   oamXposBuffer + 1 * OamFormat.size + 1,x
        asl
        asl
        ora.w   oamXposBuffer + 0 * OamFormat.size + 1,x

        sta.w   oamHiBuffer,y

        rep     #$31
    a16()
        txa
        // carry clear
        sbc.w   #4 * OamFormat.size - 1
        tax

        dey
        bpl     Loop


SkipHiTable:
    // Move all sprites that were onscreen in the previous frame offscreen
    ldx.w   bufferPos
    cpx.w   previousBufferPos
    bcs     Skip

        lda.w   previousBufferPos
        stx.w   previousBufferPos

        sec
        sbc.w   bufferPos
        lsr
        lsr
        tay

        sep     #$20
    a8()
        lda.b   #SCREEN_HEIGHT
        -
            sta.w   oamBuffer.yPos,x
            inx
            inx
            inx
            inx
            dey
            bne     -

        bra     EndIf

    Skip:
        sep     #$20
    a8()
        stx.w   previousBufferPos

        cpx.w   #128 * 4
        bcs     +
            // BUGFIX: yPos is invalid if the last sprite
            // processed by `Render` has a valid xPos, but an
            // invalid yPos.

            lda.b   #SCREEN_HEIGHT
            sta.w   oamBuffer.yPos,x
        +
EndIf:

    Dma.MetaSprite._EndLoop()

    rep     #$30
a16()
i16()
    rts
}
}

}

// vim: ft=asm ts=4 sw=4 et:

