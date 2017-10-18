// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace MetaTileRenderTest {

// Tests:
//  * MetaTile.Render
//  * MetaTile.LoadTileset
//  * Resources.AnimatedTileset
//  * Resource palette animations


constant MAP_WIDTH = 84
constant MAP_HEIGHT = 75

constant MAP_GRID_WIDTH = 3
constant MAP_GRID_HEIGHT = 3
constant MAP_REPEAT_HORIZONTAL = 24
constant MAP_REPEAT_VERTICAL = 21

assert(MAP_GRID_WIDTH * MAP_REPEAT_HORIZONTAL <= MAP_WIDTH)
assert(MAP_GRID_HEIGHT * MAP_REPEAT_VERTICAL <= MAP_HEIGHT)
assert(MAP_WIDTH * MAP_HEIGHT < MetaTiles.MAP_SIZE)

constant MIN_CAMERA_X = 0x1000
constant MIN_CAMERA_Y = 0x8000

constant MAX_CAMERA_X = MIN_CAMERA_X + MAP_WIDTH * 16 - Camera.DISPLAY_WIDTH
constant MAX_CAMERA_Y = MIN_CAMERA_Y + MAP_HEIGHT * 16 - Camera.DISPLAY_HEIGHT


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

    lda.b   #TM.bg1 | TM.bg3
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

    lda.w   #MIN_CAMERA_Y
    sta.w   MetaTiles.map.top

    lda.w   #MIN_CAMERA_X
    sta.w   MetaTiles.map.left

    lda.w   #MAP_WIDTH * 2
    sta.w   MetaTiles.map.bytesPerRow

    lda.w   #MAP_WIDTH * MetaTiles.METATILE_DISPLAY_HEIGHT * 2
    sta.w   MetaTiles.map.bytesPerDisplayHeight


    // clear tilemap

    ldx.w   #MetaTiles.MAP_SIZE * 2 - 2
    ClearLoop:
        stz.w   MetaTiles.map.data,x
        dex
        dex
        bmi     ClearLoop


    // Repeat grid pattern

    constant MARGIN_X = (MAP_WIDTH - MAP_REPEAT_HORIZONTAL * MAP_GRID_WIDTH) / 2
    constant MARGIN_Y = (MAP_HEIGHT - MAP_REPEAT_VERTICAL * MAP_GRID_HEIGHT) / 2

    ldx.w   #(MARGIN_X + MARGIN_Y * MAP_WIDTH) * 2

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
                    lda.w   #(xPos + yPos * MAP_GRID_WIDTH + 1) * 2
                    sta.w   MetaTiles.map.data + (xPos + MAP_WIDTH * yPos) * 2,x

                    xPos = xPos + 1
                }
                yPos = yPos + 1
            }

            txa
            clc
            adc.w   #MAP_GRID_WIDTH * 2
            tax

            dey
            bne     LineLoop

        lda.b   _columnIndex
        clc
        adc.w   #MAP_WIDTH * MAP_GRID_HEIGHT * 2
        tax

        dec     _columnsLeft
        bne     Loop
}


// Moves the camera depending on the state of the joypad
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, AutoJoy on
macro MoveCamera() {
    assert16a()
    assert16i()

constant _velocity = Test.dpTmp + 3 * 2

    lda.l   JOY1
    tax

    ldy.w   #1

    bit.w   #JOY.r
    beq     +
        ldy.w   #256
    +
    sty.b   _velocity


    bit.w   #JOY.up
    beq     ++
        lda.w   Camera.yPos
        sec
        sbc.b   _velocity
        cmp.w   #MIN_CAMERA_Y
        bcs     +
            lda.w   #MIN_CAMERA_Y
        +
        sta.w   Camera.yPos
        txa
    +
    bit.w   #JOY.down
    beq     ++
        lda.w   Camera.yPos
        clc
        adc.b   _velocity
        cmp.w   #MAX_CAMERA_Y + 1
        bcc     +
            lda.w   #MAX_CAMERA_Y
        +
        sta.w   Camera.yPos
        txa
    +
    bit.w   #JOY.left
    beq     ++
        lda.w   Camera.xPos
        sec
        sbc.b   _velocity
        cmp.w   #MIN_CAMERA_X
        bcs     +
            lda.w   #MIN_CAMERA_X
        +
        sta.w   Camera.xPos
        txa
    +
    bit.w   #JOY.right
    beq     ++
        lda.w   Camera.xPos
        clc
        adc.b   _velocity
        cmp.w   #MAX_CAMERA_X + 1
        bcc     +
            lda.w   #MAX_CAMERA_X
        +
        sta.w   Camera.xPos
    +
}


// Prints the camera position on the screen
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
macro PrintCameraPos() {
    assert8a()
    assert16i()

    constant textXPos = (Text.Console.TEXT_WIDTH - 10) / 2
    constant textYPos = (Text.Console.TEXT_HEIGHT - 3) / 2


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
    Text.Console.SetCursor(0, 20)
    Text.Console.PrintString(UnitTestMessage)

    sep     #$20
a8()

    lda.b   #0
    sta.l   NMITIMEN

    jsr     ForceBlank

    SetupPpu()

    rep     #$30
a16()

    jsr     Resources.Init

    lda.w   #MetaTiles.TilesetList.MetaTileRenderTest
    jsr     MetaTiles.LoadTileset

    lda.w   #Resources.PaletteList.MetaTileRenderTest
    jsr     Resources.Palettes.LoadPalette

    GenerateMap()

    lda.w   #MIN_CAMERA_X + (MAX_CAMERA_X - MIN_CAMERA_X) / 2 - 7
    sta.w   Camera.xPos

    lda.w   #MIN_CAMERA_Y + (MAX_CAMERA_Y - MIN_CAMERA_Y) / 2 + 13
    sta.w   Camera.yPos

RedrawMap:
    jsr     ForceBlank

    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    sep     #$20
a8()

    jsr     EnableVBlank
    jsr     WaitFrame

    jsr     EnableDisplay_Full

    rep     #$30
a16()

    Loop:
        lda.l   JOY1
        bit.w   #JOY.select
        bne     RedrawMap

        MoveCamera()

        jsr     Resources.ProcessAnimations

        sep     #$20
    a8()
        PrintCameraPos()

        rep     #$30
    a16()

        jsr     MetaTiles.Render.Update

        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame


        lda.l   JOY1
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Resources.AnimatedTileset.DisableAnimation()
    Resources.Palettes.DisableAnimation()

    jmp     Test.ResetPpuState
}


// ROM Data
// ========

rodata(rom0)
UnitTestMessage:
    db  "Use D-PAD to move the map\n"
    db  "Hold R to zip\n"
    db  "Press SELECT to redraw map", 0

}
}

// Temporary - will be generated by a program
rodata(rom0)
MetaTiles.TilesetData.MetaTileRenderTest:
    insert "../../gen/metatile-tilesets/metatile-render-test.tilemap.lz4b"

    db  2
    db  1
    dw  35
    insert "../../gen/metatile-tilesets/metatile-render-test.tiles.lz4b"

// vim: ft=bass-65816 ts=4 sw=4 et:

