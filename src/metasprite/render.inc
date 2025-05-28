// metasprite/render.inc
// =====================
//
// MetaSprite Renderer.
//
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
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


namespace MetaSprite {
namespace Render {

constant SCREEN_WIDTH = 256
constant SCREEN_HEIGHT = 224

// Reset `oamXposBuffer` and move all sprites in `oamBuffer` off-screen.
//
// REQUITES: 16 bit A, 16 bit Index, DB = $7e
macro _Init() {
	assert16a()
	assert16i()

    lda.w   #256 - 16
    ldx.w   #(128 - 1) * 4
    Loop:
        stz.w   MetaSprite.Render.oamXposBuffer,x
        sta.w   MetaSprite.Render.oamBuffer.yPos,x
        dex
        dex
        dex
        dex
        bpl     Loop

    ldy.w   #128 * 4
    sty.w   MetaSprite.Render.previousBufferPos


    sep     #$20
a8()
    stz.w   Dma.MetaSprite.transferOnZero

    rep     #$20
a16()
}


// Start the render loop
//
// This routine MUST BE called directly after WaitFrame in the game loop to
// prevent a VBlank overrun.
//
// REQUITES: 16 bit A, DB = $7e
a16()
iu()
code()
function InitLoop {
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
//          Camera is active
//          Screen is `SCREEN_HEIGHT` pixels tall
//
// INPUT: DP: Entity
//        Camera.xPos/Camera.yPos : Camera position
a16()
i16()
code()
function RenderEntity {
constant Return = InitLoop.Return

    ldx.b   BaseEntity.MetaSprite.currentFrame

    lda.b   BaseEntity.xPos.px
    sec
    sbc.w   #INT_MS8_OFFSET
    sec
    sbc.w   Camera.xPos
    sta.w   MetaSprite.Render.xPos

    lda.b   BaseEntity.yPos.px
    sec
    sbc.w   #INT_MS8_OFFSET
    sec
    sbc.w   Camera.yPos
    sta.w   MetaSprite.Render.yPos

End:
}


// Render an entity with a given frame at a given screen position.
//
// ASSUMES: Entity has a frameSet
//          Entity has the frame's tileset loaded in VRAM
//          Entity had a palette loaded into the PPU
//          Screen is `SCREEN_HEIGHT` pixels tall
//
// REQUITES: 16 bit A, 16 bit Index, DB = $7e
//
// INPUT: DP: Entity
//        X: frame address (NOT NULL)
//        xPos: entity xPos - INT_MS8_OFFSET
//        yPos: entity yPos - INT_MS8_OFFSET
a16()
i16()
code()
function RenderEntityFrameAndPos {
    assert(pc() == RenderEntity.End)

allocateTmpWord(nObjectsLeft)
constant Return = InitLoop.Return

    lda.l   MetaSprite.Format.Frame.frameObjects,x
    beq     Return
    tax

    // count is `number of objects - 1`
    lda.l   MetaSprite.Format.FrameObjects.count,x
    and.w   #MetaSprite.Format.FrameObjects.COUNT_MASK
    sta.w   nObjectsLeft

    // Do a single buffer overflow check
    asl
    asl
    // carry clear
    adc.w   bufferPos
    cmp.w   #(128 - 1) * 4 + 1
    bcs     Return

    ldy.w   bufferPos

    Loop:
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
        and.w   #0x0100
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

        sta.w   oamBuffer.yPos,y


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


        iny
        iny
        iny
        iny

    SkipSprite:
        inx
        inx
        inx
        inx

        dec.w   nObjectsLeft
        bpl     Loop

    sty.w   bufferPos
    rts
}



// Finalize render loop.
//
// Convert oamXposBuffer to oamHiBuffer,
// ensure unused sprites are offscreen
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function EndLoop {
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

// vim: ft=bass-65816 ts=4 sw=4 et:

