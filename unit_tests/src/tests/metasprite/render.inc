// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// These tests don't check the charattr values.
// That will be preformed by the interactive tests.

namespace MetaSpriteTest {
namespace Render {

constant oneSmallObjEntity = EntityPool.entity0
constant oneLargeObjEntity = EntityPool.entity1
constant manyObjEntity = EntityPool.entity2

namespace manyObjEntity {
    constant frameSet = MSFS.FST_TwoRows
    constant frame = MSEO.TwoFrames.Frames.frame2
    constant nLargeObjects = 11
    constant nSmallObjects = 4
    constant nObjects = nLargeObjects + nSmallObjects
}

constant SCREEN_WIDTH = 256
constant SCREEN_HEIGHT = 224

constant MS_SCREEN_XPOS = SCREEN_WIDTH / 3;
constant MS_SCREEN_YPOS = SCREEN_HEIGHT * 2 / 3;

constant CAMERA_XPOS = 0xdead;
constant CAMERA_YPOS = 0xbeef;


// Moves the current entity to a given screen position and renders it.
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: dp = entity
// PARAM: xPos/yPos - screen position to render the entity
macro _MoveAndRenderEntity(evaluate xPos, evaluate yPos) {
    assert16a()
    assert16i()

    lda.w   #CAMERA_XPOS + {xPos}
    sta.b   BaseEntity.xPos.px
    lda.w   #CAMERA_YPOS + {yPos}
    sta.b   BaseEntity.yPos.px

    jsr     MetaSprite.Render.RenderEntity
}


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (One Small)")
function RenderEntity_OneSmall {
    jsr     _Init
    bcc     Fail

    lda.w   #oneSmallObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

    jsr     MetaSprite.Render.EndLoop

    // Check that bufferPos is 4
    lda.w   MetaSprite.Render.bufferPos
    cmp.w   #1 * 4
    bne     Fail


    sep     #$20
a8()
    lda.w   MetaSprite.Render.oamBuffer.xPos
    cmp.b   #MS_SCREEN_XPOS - 8 / 2
    bne     Fail

    lda.w   MetaSprite.Render.oamBuffer.yPos
    cmp.b   #MS_SCREEN_YPOS - 8 / 2
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


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (One Large)")
function RenderEntity_OneLarge {
    jsr     _Init
    bcc     Fail

    lda.w   #oneLargeObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

    jsr     MetaSprite.Render.EndLoop

    // Check that bufferPos is 4
    lda.w   MetaSprite.Render.bufferPos
    cmp.w   #1 * 4
    bne     Fail


    sep     #$20
a8()
    lda.w   MetaSprite.Render.oamBuffer.xPos
    cmp.b   #MS_SCREEN_XPOS - 16 / 2
    bne     Fail

    lda.w   MetaSprite.Render.oamBuffer.yPos
    cmp.b   #MS_SCREEN_YPOS - 16 / 2
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


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (Top Left)")
function RenderEntity_TopLeft {
    jsr     _Init
    bcc     Fail

    lda.w   #oneLargeObjEntity
    tcd

    _MoveAndRenderEntity(-10 + 16 / 2, -5 + 16 / 2)

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


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (Offscreen)")
function RenderEntity_Offscreen {
    jsr     _Init
    bcc     Fail

    lda.w   #oneSmallObjEntity
    tcd

    _MoveAndRenderEntity(SCREEN_WIDTH + 8 / 2, 128)

    _MoveAndRenderEntity(128, -16 + 8 / 2)

    _MoveAndRenderEntity(-16 + 8 / 2, 128)

    _MoveAndRenderEntity(128, SCREEN_HEIGHT + 8 / 2)

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


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (Overflow)")
function RenderEntity_Overflow {
    // Test that we don't crash

    jsr     _Init
    bcc     Fail

    lda.w   #manyObjEntity
    tcd

    lda.w   #128 / 10
    Loop:
        pha
            jsr     MetaSprite.Render.RenderEntity
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


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (HiTable)")
function RenderEntity_HiTable {

    jsr     _Init
    bcc     Fail

    lda.w   #manyObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

    lda.w   #oneLargeObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

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


a16()
i16()
code()
Test.add("MetaSprite.RenderEntity (HiTable Size)")
function RenderEntity_HiTableSize {

// There was a bug where the size of an object with an
// xPos < 0 would always be large. This tests for correct
// behaviour

    jsr     _Init
    bcc     Fail

    lda.w   #oneSmallObjEntity
    tcd
    _MoveAndRenderEntity(-1, SCREEN_HEIGHT / 2)

    lda.w   #oneLargeObjEntity
    tcd
    _MoveAndRenderEntity(-1, SCREEN_HEIGHT / 2)

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


a16()
i16()
code()
Test.add("MetaSprite.EndLoop")
function EndLoop {

    jsr     _Init
    bcc     Fail

    jsr     MetaSprite.Render.InitLoop

    lda.w   #oneSmallObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

    jsr     MetaSprite.Render.EndLoop

    lda.w   #1
    jsr     _CheckSpritesOffscreen
    bcc     Fail



    jsr     MetaSprite.Render.InitLoop

    lda.w   #manyObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

    lda.w   #oneLargeObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity

    jsr     MetaSprite.Render.EndLoop

    lda.w   #manyObjEntity.nObjects + 1
    jsr     _CheckSpritesOffscreen
    bcc     Fail



    jsr     MetaSprite.Render.InitLoop

    lda.w   #oneLargeObjEntity
    tcd
    jsr     MetaSprite.Render.RenderEntity
    jsr     MetaSprite.Render.RenderEntity
    jsr     MetaSprite.Render.RenderEntity

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
//  * Moves the camera to CAMERA_XPOS/CAMERA_YPOS
//  * Creates sample three entities, all at the screen position MS_SCREEN_XPOS/MS_SCREEN_YPOS
//     * oneSmallObjEntity (entity0) has one small object
//     * oneLargeObjEntity (entity1) has one large object
//     * manyObjectsEntity (entity2) contains many large and small object
//  * Initializes the render loop
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// OUTPUT: C set on success
a16()
i16()
code()
function _Init {
    macro InitEntity(addr, frameSet, frame) {
        lda.w   #{addr}
        tcd

        lda.w   #CAMERA_XPOS + MS_SCREEN_XPOS
        sta.b   BaseEntity.xPos.px
        lda.w   #CAMERA_YPOS + MS_SCREEN_YPOS
        sta.b   BaseEntity.yPos.px

        lda.w   #{frameSet}
        ldx.w   #{frame}
        ldy.w   #0
        jsr     _InitEntityFrameActivate
        bcc     Fail
    }

    jsr     _Init_MetaSpriteModule

    lda.w   #CAMERA_XPOS
    sta.w   Camera.xPos
    lda.w   #CAMERA_YPOS
    sta.w   Camera.yPos

    InitEntity(oneSmallObjEntity, MSFS.FST_TwoTilesFixed, MSEO.OneSmallAndLarge.Frames.OneSmallObj)
    InitEntity(oneLargeObjEntity, MSFS.FST_TwoTilesFixed, MSEO.OneSmallAndLarge.Frames.OneLargeObj)
    InitEntity(manyObjEntity, manyObjEntity.frameSet, manyObjEntity.frame)

    jsr     MetaSprite.Render.InitLoop

    sec
    rts

Fail:
    clc
    rts
}


// Checks that sprites A -> 127 are offscreen.
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// OUTPUT: C set if all sprites offscreen
a16()
i16()
code()
function _CheckSpritesOffscreen {
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

// vim: ft=bass-65816 ts=4 sw=4 et:

