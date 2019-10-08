// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//  * MetaTiles.Render
//  * MetaTiles.SetMapCell (including the drawMapCellBuffer does not overflow)
//  * MetaTiles.LoadTileset
//  * MetaTiles.EntityPositionToMapIndex
//  * Resources.AnimatedTileset
//  * Resource palette animations

namespace Entities {

namespace MapCursor {
    buildFunctionTable(BaseEntityFunctionTable, MapCursor)

    namespace EntityStruct {
        childstruct(BaseEntity)
        field(tmp, 2)
        field(tmp2, 2)
        field(tmp3, 2)
        endstruct()
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        SetFrame(frame)

        // always activate
        sec
        rts
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
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

        // Set Velocity depending on joypad settings

        lda.w   Controller.Joy1.current

        ldx.w   #1
        bit.w   #JOY.r
        beq     +
            ldx.w   #32
        +

        // X = speed
        // A = Controller.Joy1.current
        ldy.w   #0
        bit.w   #JOY.up | JOY.down
        beq     SkipUpDown
            bit.w   #JOY.down
            beq     +
                txy
                bra     ++
            +
            txa
            eor.w   #0xffff
            inc
            tay
            lda.w   Controller.Joy1.current
        +
    SkipUpDown:
        sty.b   EntityStruct.yVecl + 2


        // X = speed
        // A = Controller.Joy1.current
        ldy.w   #0
        bit.w   #JOY.left | JOY.right
        beq     SkipLeftRight
            bit.w   #JOY.right
            beq     +
                txy
                bra     ++
            +
            txa
            eor.w   #0xffff
            inc
            tay
        +
    SkipLeftRight:
        sty.b   EntityStruct.xVecl + 2


        macro _processAxis(c, minMap, maxMap, displaySize) {
            //  pos += vecl
            //
            //  if pos < minMap:
            //      pos = minMap
            //  else if pos >= maxMap:
            //      pos = mapMap - 1
            //
            //  if pos < Camera.pos:
            //      Camera.pos = Pos
            //  else if pos - displaySize > Camera.pos:
            //      Camera.pos = pos - displaySize

            clc
            lda.b   EntityStruct.{c}Pos.sx
            adc.b   EntityStruct.{c}Vecl
            sta.b   EntityStruct.{c}Pos.sx
            lda.b   EntityStruct.{c}Pos.px
            adc.b   EntityStruct.{c}Vecl + 2

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

            cmp.w   Camera.{c}Pos
            bcc     +
            sec
            sbc.w   #Camera.{displaySize}
            cmp.w   Camera.{c}Pos
            bcc     ++
            +
                sta.w   Camera.{c}Pos
            +
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
            stx.b   EntityStruct._mapIndex
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
                    // X = _mapIndex - _displacement / METATILE_SIZE_PX * 2
                    lda.b   EntityStruct._displacement
                    assert(MetaTiles.METATILE_SIZE_PX == 16)
                    lsr
                    lsr
                    lsr
                    lsr
                    eor.w   #0xffff
                    sec
                    adc.b   EntityStruct._mapIndex
                    tax

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
                    // X = _mapIndex + _displacement / METATILE_SIZE_PX * 2
                    lda.b   EntityStruct._displacement
                    assert(MetaTiles.METATILE_SIZE_PX == 16)
                    lsr
                    lsr
                    lsr
                    lsr
                    clc
                    adc.b   EntityStruct._mapIndex
                    tax

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
        jmp     Entity.DrawAndGotoNextEntity
    }
}
}


namespace InteractiveTests {
namespace MetaTileRenderTest {

constant MAP_WIDTH = 84
constant MAP_HEIGHT = 75

constant MAP_GRID_WIDTH = 3
constant MAP_GRID_HEIGHT = 3
constant MAP_GRID_TILE_OFFSET = 16
constant MAP_GRID_LINE_WIDTH = 16;
constant MAP_REPEAT_HORIZONTAL = 24
constant MAP_REPEAT_VERTICAL = 21

assert(MAP_GRID_WIDTH * MAP_REPEAT_HORIZONTAL <= MAP_WIDTH)
assert(MAP_GRID_HEIGHT * MAP_REPEAT_VERTICAL <= MAP_HEIGHT)
assert(MAP_WIDTH * MAP_HEIGHT < MetaTiles.MAP_SIZE)

constant MIN_CAMERA_X = MetaTiles.map.LEFT
constant MIN_CAMERA_Y = MetaTiles.map.TOP

constant MAX_CAMERA_X = MIN_CAMERA_X + MAP_WIDTH * 16 - Camera.DISPLAY_WIDTH
constant MAX_CAMERA_Y = MIN_CAMERA_Y + MAP_HEIGHT * 16 - Camera.DISPLAY_HEIGHT

constant CAMERA_START_X = MIN_CAMERA_X + (MAX_CAMERA_X - MIN_CAMERA_X) / 2 - 7
constant CAMERA_START_Y = MIN_CAMERA_Y + (MAX_CAMERA_Y - MIN_CAMERA_Y) / 2 + 13


// Initializes the screen and loads tiles/palette to PPU
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, ForceBlank
macro SetupPpu() {
    assert8a()
    assert16i()

    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB = REGISTER_DB


    // Setup PPU
    // MODE 1 - bg3 priority
    //   BG1 - MetaTiles map
    //   BG3 - text console buffer

    lda.b   #BGMODE.mode1Bg3
    sta.w   BGMODE

    lda.b   #((METATILES_MAP_WADDR / BGXSC.base.walign) << BGXSC.base.shift) | BGXSC.map.s64x32
    sta.w   BG1SC

    lda.b   #((VRAM_CONSOLE_MAP_WADDR / BGXSC.base.walign) << BGXSC.base.shift) | BGXSC.map.s32x32
    sta.w   BG3SC

    lda.b   #(METATILES_TILES_WADDR / BG12NBA.walign) << BG12NBA.bg1.shift
    sta.w   BG12NBA

    lda.b   #(VRAM_CONSOLE_TILES_WADDR / BG34NBA.walign) << BG34NBA.bg3.shift
    sta.w   BG34NBA

    lda.b   #(VRAM_OBJ_WADDR / OBSEL.base.walign) | OBSEL.size.s8_16
    sta.w   OBSEL

    lda.b   #TM.bg1 | TM.bg3 | TM.obj
    sta.w   TM

    plb
    // DB = 0x7e
}


// Generates the map.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro GenerateMap() {
    assert16a()
    assert16i()

constant _columnsLeft = Test.dpTmp
constant _columnIndex = Test.dpTmp + 2

    assert(MetaTiles.map.width + 1 == MetaTiles.map.height)
    lda.w   #MAP_WIDTH | (MAP_HEIGHT << 8)
    sta.w   MetaTiles.map.width

    lda.w   #MIN_CAMERA_Y + MAP_HEIGHT * 16
    sta.w   MetaTiles.map.bottom

    lda.w   #MIN_CAMERA_X + MAP_WIDTH * 16
    sta.w   MetaTiles.map.right

    lda.w   #MAP_WIDTH * MAP_HEIGHT
    sta.w   MetaTiles.map.dataSize

    lda.w   #MAP_WIDTH
    sta.w   MetaTiles.map.bytesPerRow

    lda.w   #MAP_WIDTH * MetaTiles.METATILE_DISPLAY_HEIGHT
    sta.w   MetaTiles.map.bytesPerDisplayHeight


    // clear tilemap

    assert(MetaTiles.MAP_SIZE % 2 == 0)
    ldx.w   #MetaTiles.MAP_SIZE - 2
    ClearLoop:
        stz.w   MetaTiles.map.data,x
        dex
        dex
        bmi     ClearLoop


    // Repeat grid pattern

    constant MARGIN_X = (MAP_WIDTH - MAP_REPEAT_HORIZONTAL * MAP_GRID_WIDTH) / 2
    constant MARGIN_Y = (MAP_HEIGHT - MAP_REPEAT_VERTICAL * MAP_GRID_HEIGHT) / 2

    ldx.w   #(MARGIN_X + MARGIN_Y * MAP_WIDTH)

    lda.w   #MAP_REPEAT_VERTICAL
    sta.b   _columnsLeft

    Loop:
        stx.b   _columnIndex

        ldy.w   #MAP_REPEAT_HORIZONTAL

        LineLoop:
            variable yPos = 0
            while yPos < MAP_GRID_HEIGHT {
                variable xPos = 0
                while xPos < MAP_GRID_WIDTH {
                    lda.w   #(xPos + yPos * MAP_GRID_LINE_WIDTH + MAP_GRID_TILE_OFFSET)
                    sta.w   MetaTiles.map.data + (xPos + MAP_WIDTH * yPos),x

                    xPos = xPos + 1
                }
                yPos = yPos + 1
            }

            txa
            clc
            adc.w   #MAP_GRID_WIDTH
            tax

            dey
            bne     LineLoop

        lda.b   _columnIndex
        clc
        adc.w   #MAP_WIDTH * MAP_GRID_HEIGHT
        tax

        dec.b   _columnsLeft
        bne     Loop
}


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
a16()
i16()
code()
Test.add("MetaTile Render Test")
function Test {
    Text.Console.SetCursor(0, 19)
    Text.Console.PrintString(UnitTestMessage)

    sep     #$20
a8()

    jsr     ForceBlank

    SetupPpu()

    rep     #$30
a16()

    jsr     Entity.Init
    jsr     Resources.Init


    lda.w   #Project.MetaTileTilesetList.MetaTileRenderTest
    jsr     MetaTiles.LoadTileset

    lda.w   #Project.PaletteList.MetaTileRenderTest
    jsr     Resources.Palette.LoadPalette

    GenerateMap()

    lda.w   #CAMERA_START_X
    sta.w   Camera.xPos

    lda.w   #CAMERA_START_Y
    sta.w   Camera.yPos


    lda.w   #CAMERA_START_X + Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   #CAMERA_START_Y + Camera.DISPLAY_HEIGHT / 2
    sta.w   Entity.Spawn.yPos

    lda.w   #Project.EntityIds.MapCursor
    jsr     Entity.Spawn
    bcs     +
        rts
    +


RedrawMap:
    jsr     ForceBlank

    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    sep     #$20
a8()

    jsr     EnableDisplay_Full

    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_PrintFreeCycles
        jsr     MetaSprite.Render.InitLoop

        // Flash of black when the map is redrawn is too fast
        // Constantly redraw the map whenever select is held
        lda.w   Controller.Joy1.current
        bit.w   #JOY.select
        bne     RedrawMap

        jsr     EntityLoop.ProcessFrame

        jsr     Resources.ProcessAnimations

        jsr     MetaTiles.Render.Update

        sep     #$20
    a8()
        PrintCameraPos()

        rep     #$30
    a16()
        jsr     MetaSprite.Render.EndLoop


        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Resources.AnimatedTileset.DisableAnimation()
    Resources.Palette.DisableAnimation()

    jmp     Test.ResetPpuState
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

