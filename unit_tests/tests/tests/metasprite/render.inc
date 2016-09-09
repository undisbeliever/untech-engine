
// These tests don't check the charattr values.
// That will be preformed by the interactive tests.

scope MetaSpriteTest {
scope Render {

constant oneSmallObjEntity(EntityPool.entity0)
constant oneLargeObjEntity(EntityPool.entity1)
constant manyObjEntity(EntityPool.entity2)

scope manyObjEntity {
    constant frameSet(MSFS.FST_TwoRows)
    constant frame(MSEO.TwoFrames.Frames.frame2)
    constant nLargeObjects(11)
    constant nSmallObjects(4)
    constant nObjects(nLargeObjects + nSmallObjects)
}

constant SCREEN_WIDTH(256)
constant SCREEN_HEIGHT(224)


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (Null)")
scope RenderEntity_Null: {
    jsr     _Init
    bcc     Fail

    // Try render a destroyed frameset
    lda.w   #EntityPool.entity0
    tcd
    MetaSprite.EntityDestructor()
    jsr     _RenderEntityCenter

    // Then a metasprite with no frame
    lda.w   #EntityPool.entity1
    tcd
    stz.b   BaseEntity.MetaSprite.currentFrame
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    // Check that bufferPos is 0
    lda.w   MetaSprite.Render.bufferPos
    bne     Fail

    // Check all sprites offscreen
    lda.w   #0
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (One Small)")
scope RenderEntity_OneSmall: {
    jsr     _Init
    bcc     Fail

    lda.w   #oneSmallObjEntity
    tcd
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    // Check that bufferPos is 4
    lda.w   MetaSprite.Render.bufferPos
    cmp.w   #1 * 4
    bne     Fail


    sep     #$20
a8()
    lda.w   MetaSprite.Render.oamBuffer.xPos
    cmp.b   #(SCREEN_WIDTH - 8) / 2
    bne     Fail

    lda.w   MetaSprite.Render.oamBuffer.yPos
    cmp.b   #(SCREEN_HEIGHT - 8) / 2
    bne     Fail

    lda.w   MetaSprite.Render.oamHiBuffer
    and.b   #3
    bne     Fail

    rep     #$20
a16()
    // Check all other sprites are offscreen
    lda.w   #1
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (One Large)")
scope RenderEntity_OneLarge: {
    jsr     _Init
    bcc     Fail

    lda.w   #oneLargeObjEntity
    tcd
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    // Check that bufferPos is 4
    lda.w   MetaSprite.Render.bufferPos
    cmp.w   #1 * 4
    bne     Fail


    sep     #$20
a8()
    lda.w   MetaSprite.Render.oamBuffer.xPos
    cmp.b   #(SCREEN_WIDTH - 16) / 2
    bne     Fail

    lda.w   MetaSprite.Render.oamBuffer.yPos
    cmp.b   #(SCREEN_HEIGHT - 16) / 2
    bne     Fail

    lda.w   MetaSprite.Render.oamHiBuffer
    and.b   #3
    cmp.b   #2
    bne     Fail

    rep     #$20
a16()
    // Check all other sprites are offscreen
    lda.w   #1
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (Top Left)")
scope RenderEntity_TopLeft: {
    jsr     _Init
    bcc     Fail

    lda.w   #oneLargeObjEntity
    tcd
    lda.w   #-10 + 16 / 2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #-5 + 16 / 2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos

    jsr     MetaSprite.Render.RenderEntity

    jsr     MetaSprite.Render.EndLoop


    // Check that bufferPos is 4
    lda.w   MetaSprite.Render.bufferPos
    cmp.w   #1 * 4
    bne     Fail

    // Check position
    sep     #$20
a8()
    lda.w   MetaSprite.Render.oamBuffer.xPos
    cmp.b   #-10
    bne     Fail

    lda.w   MetaSprite.Render.oamBuffer.yPos
    cmp.b   #-5
    bne     Fail

    // check x8 set
    lda.w   MetaSprite.Render.oamHiBuffer
    and.b   #3
    cmp.b   #3
    bne     Fail

    rep     #$20
a16()
    // Check all other sprites are offscreen
    lda.w   #1
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (Offscreen)")
scope RenderEntity_Offscreen: {
    jsr     _Init
    bcc     Fail

    lda.w   #oneSmallObjEntity
    tcd
    lda.w   #SCREEN_WIDTH + 8/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #128 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos
    jsr     MetaSprite.Render.RenderEntity

    lda.w   #128 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #-16 + 8/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos
    jsr     MetaSprite.Render.RenderEntity

    lda.w   #-16 + 8/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #128 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos
    jsr     MetaSprite.Render.RenderEntity

    lda.w   #128 + INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #SCREEN_HEIGHT + 8/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos
    jsr     MetaSprite.Render.RenderEntity

    jsr     MetaSprite.Render.EndLoop


    // Check that bufferPos is 0
    lda.w   MetaSprite.Render.bufferPos
    bne     Fail

    // Check all the sprites are offscreen
    lda.w   #0
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (Overflow)")
scope RenderEntity_Overflow: {
    // Test that we don't crash

    jsr     _Init
    bcc     Fail

    lda.w   #manyObjEntity
    tcd

    lda.w   #128 / 10
    Loop:
        pha
            jsr     _RenderEntityCenter
        pla
        dec
        bne     Loop

    jsr     MetaSprite.Render.EndLoop


    // Check that bufferPos is 128 sprites
    lda.w   MetaSprite.Render.bufferPos
    cmp.w   #(128 - (128 % manyObjEntity.nObjects)) * 4
    bne     Fail

    lda.w   #(128 - (128 % manyObjEntity.nObjects))
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (HiTable)")
scope RenderEntity_HiTable: {

    jsr     _Init
    bcc     Fail

    lda.w   #manyObjEntity
    tcd
    jsr     _RenderEntityCenter

    lda.w   #oneLargeObjEntity
    tcd
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    assert(manyObjEntity.nLargeObjects == 11)
    assert(manyObjEntity.nSmallObjects == 4)

    lda.w   MetaSprite.Render.oamHiBuffer
    cmp.w   #%10'10'10'10'10'10'10'10
    bne     Fail

    lda.w   MetaSprite.Render.oamHiBuffer + 2
    cmp.w   #%10'00'00'00'00'10'10'10
    bne     Fail

    lda.w   #manyObjEntity.nObjects + 1
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.RenderEntity (HiTable Size)")
scope RenderEntity_HiTableSize: {

// There was a bug where the size of an object with an
// xPos < 0 would always be large. This tests for correct
// behaviour

    jsr     _Init
    bcc     Fail

    lda.w   #oneSmallObjEntity
    tcd

    lda.w   #-1 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #SCREEN_HEIGHT/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos

    jsr     MetaSprite.Render.RenderEntity



    lda.w   #oneLargeObjEntity
    tcd

    lda.w   #-1 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #SCREEN_HEIGHT/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos

    jsr     MetaSprite.Render.RenderEntity



    jsr     MetaSprite.Render.EndLoop

    lda.w   MetaSprite.Render.oamHiBuffer
    and.w   #%11'11
    cmp.w   #%11'01
    bne     Fail

    lda.w   #2
    jmp     _CheckSpritesOffscreen

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.EndLoop")
scope EndLoop: {

    jsr     _Init
    bcc     Fail

    jsr     MetaSprite.Render.InitLoop

    lda.w   #oneSmallObjEntity
    tcd
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    lda.w   #1
    jsr     _CheckSpritesOffscreen
    bcc     Fail



    jsr     MetaSprite.Render.InitLoop

    lda.w   #manyObjEntity
    tcd
    jsr     _RenderEntityCenter

    lda.w   #oneLargeObjEntity
    tcd
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    lda.w   #manyObjEntity.nObjects + 1
    jsr     _CheckSpritesOffscreen
    bcc     Fail



    jsr     MetaSprite.Render.InitLoop

    lda.w   #oneLargeObjEntity
    tcd
    jsr     _RenderEntityCenter
    jsr     _RenderEntityCenter
    jsr     _RenderEntityCenter

    jsr     MetaSprite.Render.EndLoop

    lda.w   #3
    jsr     _CheckSpritesOffscreen
    bcc     Fail



    jsr     MetaSprite.Render.InitLoop
    jsr     MetaSprite.Render.EndLoop

    lda.w   #0
    jmp     _CheckSpritesOffscreen


Fail:
    clc
    rts
}


// Preps the test
//
//  * Initializes the Dma/MetaSprite Engines
//  * Creates sample entities
//     * oneSmallObjEntity (entity0) has one small object
//     * oneLargeObjEntity (entity1) has one large object
//     * manyObjectsEntity (entity2) contains many large and small object
//  * Initializes the render loop
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// OUTPUT: C set on success
code()
a16()
i16()
scope _Init: {
    jsr     _Init_MetaSpriteModule

    lda.w   #oneSmallObjEntity
    tcd
    lda.w   #MSFS.FST_TwoTilesFixed
    ldx.w   #MSEO.OneSmallAndLarge.Frames.OneSmallObj
    ldy.w   #0
    jsr     _InitEntityFrameActivate
    bcc     Fail

    lda.w   #oneLargeObjEntity
    tcd
    lda.w   #MSFS.FST_TwoTilesFixed
    ldx.w   #MSEO.OneSmallAndLarge.Frames.OneLargeObj
    ldy.w   #0
    jsr     _InitEntityFrameActivate
    bcc     Fail

    lda.w   #manyObjEntity
    tcd
    lda.w   #manyObjEntity.frameSet
    ldx.w   #manyObjEntity.frame
    ldy.w   #0
    jsr     _InitEntityFrameActivate
    bcc     Fail

    jsr     MetaSprite.Render.InitLoop

    sec
    rts

Fail:
    clc
    rts
}

// Renders the entity at the center of the screen
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
code()
a16()
i16()
scope _RenderEntityCenter: {
    lda.w   #SCREEN_WIDTH/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.xPos
    lda.w   #SCREEN_HEIGHT/2 - INT_MS8_OFFSET
    sta.w   MetaSprite.Render.yPos

    jmp     MetaSprite.Render.RenderEntity
}


// Checks that sprites A -> 127 are offscreen.
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// OUTPUT: C set if all sprites offscreen
code()
a16()
i16()
scope _CheckSpritesOffscreen: {
    asl
    asl
    tax

    sep     #$20
a8()
    Loop:
        lda.w   MetaSprite.Render.oamBuffer.yPos,x
        cmp.b   #SCREEN_HEIGHT
        bcc     Fail

        inx
        inx
        inx
        inx
        cpx.w   #128 * 4
        bcc     Loop

    rep     #$20
a16()
    // carry set
    rts

Fail:
    clc
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

