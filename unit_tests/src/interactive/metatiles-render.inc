// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
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

// Tests:
//  * MetaTiles.Render
//  * MetaTiles.SetMapCell (including the drawMapCellBuffer does not overflow)
//  * MetaTiles.LoadTileset
//  * MetaTiles.EntityPositionToMapIndex
//  * Room.LoadRoom
//  * Resources.AnimatedTileset
//  * Resource palette animations

namespace Entities {
namespace Players {

namespace MapCursor {
    buildFunctionTable(BasePlayerFunctionTable, MapCursor)

    namespace EntityStruct {
        childstruct(BaseEntity)
        field(tmp, 2)
        field(tmp2, 2)
        field(tmp3, 2)
        endstruct()
    }


    constant DrawPlayer = MetaSprite.Render.RenderEntity


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Init {
        SetFrame(frame)

        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    RoomAboutToChange:
    HitboxCollision:
    HurtboxCollision:
    ShieldCollision:
    function Null {
        rts
    }


    // REQUIRES: Map Loaded
    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    function Process {
        // Update entity position depending on joypad settings

        lda.w   Controller.Joy1.current

        ldx.w   #1
        bit.w   #JOY.r
        beq     +
            ldx.w   #32
        +

        // X = speed
        // A = Controller.Joy1.current
        bit.w   #JOY.left
        beq     +
            txa
            eor.w   #0xffff
            sec
            adc.b   BaseEntity.xPos.px
            sta.b   BaseEntity.xPos.px

            lda.w   Controller.Joy1.current
        +

        bit.w   #JOY.right
        beq     +
            txa
            clc
            adc.b   BaseEntity.xPos.px
            sta.b   BaseEntity.xPos.px

            lda.w   Controller.Joy1.current
        +

        bit.w   #JOY.up
        beq     +
            txa
            eor.w   #0xffff
            sec
            adc.b   BaseEntity.yPos.px
            sta.b   BaseEntity.yPos.px

            lda.w   Controller.Joy1.current
        +

        bit.w   #JOY.down
        beq     +
            txa
            clc
            adc.b   BaseEntity.yPos.px
            sta.b   BaseEntity.yPos.px
        +

        macro _processAxis(c, minMap, maxMap, displaySize) {
            //  if pos < minMap:
            //      pos = minMap
            //  else if pos >= maxMap:
            //      pos = maxMap - 1

            lda.b   EntityStruct.{c}Pos.px
            cmp.w   #MetaTiles.map.{minMap}
            bcs     +
                lda.w   #MetaTiles.map.{minMap}
                bra     ++
            +
            cmp.w   MetaTiles.map.{maxMap}
            bcc     +
                lda.w   MetaTiles.map.{maxMap}
                dec
            +
            sta.b   EntityStruct.{c}Pos.px
        }
        _processAxis(x, LEFT, right, DISPLAY_WIDTH)
        _processAxis(y, TOP, bottom, DISPLAY_HEIGHT)


        lda.w   Controller.Joy1.current
        and.w   #JOY.l
        beq     SkipSetCell
            // Set tile under cursor if L is down

            constant EntityStruct._mapIndex     = EntityStruct.tmp
            constant EntityStruct._displacement = EntityStruct.tmp2
            constant EntityStruct._cellsLeft    = EntityStruct.tmp3

            jsr     MetaTiles.EntityPositionToMapIndex
            sty.b   EntityStruct._mapIndex
            lda.w   #1
            jsr     MetaTiles.SetMapCell

            // If R is down then draw map cells to the left and right of
            // current cell (if possible), repeated as may times as
            // needed to overflow the drawMapCellBuffer.

            lda.w   Controller.Joy1.current
            and.w   #JOY.r
            beq     SkipSetCell

                lda.w   #MetaTiles.drawMapCellBuffer.BUFFER_SIZE
                sta.b   EntityStruct._cellsLeft

                lda.w   #MetaTiles.METATILE_SIZE_PX

            Loop:
                // A = displacement
                sta.b   EntityStruct._displacement

                // Set tile to the left (if in bounds)
                lda.b   EntityStruct.xPos.px
                sec
                sbc.b   EntityStruct._displacement
                cmp.w   #MetaTiles.map.LEFT
                bcc     +
                    // Y = _mapIndex - _displacement / METATILE_SIZE_PX * 2
                    lda.b   EntityStruct._displacement
                    assert(MetaTiles.METATILE_SIZE_PX == 16)
                    lsr
                    lsr
                    lsr
                    lsr
                    eor.w   #0xffff
                    sec
                    adc.b   EntityStruct._mapIndex
                    tay

                    lda.w   #1
                    jsr     MetaTiles.SetMapCell

                    dec.b   EntityStruct._cellsLeft
                    beq     EndLoop
                +

                // Set tile to the right (if in bounds)
                // A = displacement
                lda.b   EntityStruct.xPos.px
                clc
                adc.b   EntityStruct._displacement
                cmp.w   MetaTiles.map.right
                bcs     +
                    // Y = _mapIndex + _displacement / METATILE_SIZE_PX * 2
                    lda.b   EntityStruct._displacement
                    assert(MetaTiles.METATILE_SIZE_PX == 16)
                    lsr
                    lsr
                    lsr
                    lsr
                    clc
                    adc.b   EntityStruct._mapIndex
                    tay

                    lda.w   #1
                    jsr     MetaTiles.SetMapCell

                    dec.b   EntityStruct._cellsLeft
                    beq     EndLoop
                +

                lda.b   EntityStruct._displacement
                clc
                adc.w   #MetaTiles.METATILE_SIZE_PX
                bra     Loop

            EndLoop:

            // Test that drawMapCellBufferIndex did not overflow
            ldx.w   MetaTiles.drawMapCellBufferIndex
            cpx.w   #MetaTiles.drawMapCellBuffer.BUFFER_SIZE * 2
            beq     +
            bcc     +
                break(ASSERT_FAILURE)
            +

    SkipSetCell:
        rts
    }
}
}
}


namespace InteractiveTests {
namespace MetaTileRenderTest {


// Prints the camera position on the screen
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
macro PrintCameraPos() {
    assert8a()
    assert16i()

    constant textXPos = 0
    constant textYPos = 4

    Text.Console.SetCursor(textXPos, textYPos)
    Text.Console.PrintConstString("xPos: ")

    ldy.w   Camera.xPos
    jsr     Text.Console.PrintHex16Y

    Text.Console.SetCursor(textXPos, textYPos + 2)
    Text.Console.PrintConstString("yPos: ")

    ldy.w   Camera.yPos
    jsr     Text.Console.PrintHex16Y
}


// MetaTile Render Test
//
// REQUIRES: MetaTile map loaded
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = roomId
a16()
i16()
code()
function DoTest {
    // A = room Id
    ldx.w   #Project.PlayerIds.MapCursor
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    jsr     Test.PrintCurrentTestName

    Text.Console.SetCursor(0, 19)
    Text.Console.PrintString(UnitTestMessage)


    jsr     Resources.TransferToPpu


RedrawMap:
    jsl     ForceBlank__far

    jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far

    sep     #$20
a8()

    jsr     EnableDisplay_Full

    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        // Flash of black when the map is redrawn is too fast
        // Constantly redraw the map whenever select is held
        lda.w   Controller.Joy1.current
        bit.w   #JOY.select
        bne     RedrawMap


        jsr     GameLoop.ProcessFrame


        sep     #$20
    a8()
        PrintCameraPos()

        rep     #$30
    a16()
        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Resources.AnimatedTileset.DisableAnimation()
    Resources.Palette.DisableAnimation()

    jmp     Test.ResetPpuState
}


a16()
i16()
code()
Test.add("MetaTile Render (Small map)")
function SmallMapHeightTest {
    lda.w   #Project.RoomList.MetaTileRenderTestSmall
    jmp     DoTest
}


a16()
i16()
code()
Test.add("MetaTile Render (Large map)")
function LargeMapHeightTest {
    lda.w   #Project.RoomList.MetaTileRenderTestLarge
    jmp     DoTest
}


// ROM Data
// ========

evaluate LR_CELLS = MetaTiles.drawMapCellBuffer.BUFFER_SIZE + 1

rodata(rom0)
UnitTestMessage:
    db  "Use D-PAD to move cursor\n"
    db  "Hold R to zip\n"
    db  "Press SELECT to redraw map\n"
    db  "Press L to set map cell\n"
    db  "Press L&R to set {LR_CELLS} map cells",0
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

