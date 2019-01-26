// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace MetaTileRenderTest {

// Tests:
//  * MetaTile.Render
//  * MetaTile.LoadTileset
//  * MetaTile.EntityPositionToMapIndex
//  * Resources.AnimatedTileset
//  * Resource palette animations


constant MAP_WIDTH = 84
constant MAP_HEIGHT = 75

constant MAP_GRID_WIDTH = 3
constant MAP_GRID_HEIGHT = 3
constant MAP_GRID_TILE_OFFSET = 3
constant MAP_REPEAT_HORIZONTAL = 24
constant MAP_REPEAT_VERTICAL = 21

assert(MAP_GRID_WIDTH * MAP_REPEAT_HORIZONTAL <= MAP_WIDTH)
assert(MAP_GRID_HEIGHT * MAP_REPEAT_VERTICAL <= MAP_HEIGHT)
assert(MAP_WIDTH * MAP_HEIGHT < MetaTiles.MAP_SIZE)

constant MIN_CAMERA_X = 0x1000
constant MIN_CAMERA_Y = 0x8000

constant MAX_CAMERA_X = MIN_CAMERA_X + MAP_WIDTH * 16 - Camera.DISPLAY_WIDTH
constant MAX_CAMERA_Y = MIN_CAMERA_Y + MAP_HEIGHT * 16 - Camera.DISPLAY_HEIGHT

constant CAMERA_START_X = MIN_CAMERA_X + (MAX_CAMERA_X - MIN_CAMERA_X) / 2 - 7
constant CAMERA_START_Y = MIN_CAMERA_Y + (MAX_CAMERA_Y - MIN_CAMERA_Y) / 2 + 13


namespace MapCursorEntity {
    buildFunctionTable(BaseEntityFunctionTable, MapCursorEntity)

    namespace EntityStruct {
        childstruct(BaseEntity)
        field(tmp, 2)
        endstruct()
    }


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        lda.w   #0
        jsr     MetaSprite.SetFrame

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

            cmp.w   MetaTiles.map.{minMap}
            bcs     +
                lda.w   MetaTiles.map.{minMap}
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
        _processAxis(x, left, right, DISPLAY_WIDTH)
        _processAxis(y, top, bottom, DISPLAY_HEIGHT)


        lda.w   Controller.Joy1.current
        and.w   #JOY.l
        beq     SkipSetCell
            // Set tile under cursor if L is down

            constant _mapIndex = EntityStruct.tmp

            jsr     MetaTiles.EntityPositionToMapIndex
            stx.b   _mapIndex
            lda.w   #1
            jsr     MetaTiles.SetMapCell

            // If R is down then draw map cells to the left and right of
            // current cell (if possible)

            lda.w   Controller.Joy1.current
            and.w   #JOY.r
            beq     SkipSetCell
                lda.b   EntityStruct.xPos.px
                sec
                sbc.w   #MetaTiles.METATILE_SIZE_PX
                cmp.w   MetaTiles.map.left
                bcc     +
                    ldx.b   _mapIndex
                    dex
                    dex
                    lda.w   #1
                    jsr     MetaTiles.SetMapCell
                +

                lda.b   EntityStruct.xPos.px
                clc
                adc.w   #MetaTiles.METATILE_SIZE_PX
                cmp.w   MetaTiles.map.right
                bcs     +
                    ldx.b   _mapIndex
                    inx
                    inx
                    lda.w   #1
                    jsr     MetaTiles.SetMapCell
                +

    SkipSetCell:
        jmp     Entity.DrawAndGotoNextEntity
    }


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  EntityLists.PLAYERS     // initialListId
        db  0                       // defaultPalette
        dw  MSFS.Cursor             // frameSetId

    rodata(EN_EntityDataList)
    constant ENTITY_DATA_ID = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData
}


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

    lda.w   #MIN_CAMERA_Y
    sta.w   MetaTiles.map.top

    lda.w   #MIN_CAMERA_X
    sta.w   MetaTiles.map.left

    lda.w   #MIN_CAMERA_Y + MAP_HEIGHT * 16
    sta.w   MetaTiles.map.bottom

    lda.w   #MIN_CAMERA_X + MAP_WIDTH * 16
    sta.w   MetaTiles.map.right

    lda.w   #MAP_WIDTH * MAP_HEIGHT * 2
    sta.w   MetaTiles.map.dataSize

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
                    lda.w   #(xPos + yPos * MAP_GRID_WIDTH + MAP_GRID_TILE_OFFSET) * 2
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

    lda.w   #MapCursorEntity.ENTITY_DATA_ID
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
        jsr     WaitFrame
        jsr     MetaSprite.Render.InitLoop

        lda.l   JOY1
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


        lda.l   JOY1
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

rodata(rom0)
UnitTestMessage:
    db  "Use D-PAD to move cursor\n"
    db  "Hold R to zip\n"
    db  "Press SELECT to redraw map\n"
    db  "Press L to set map cell\n"
    db  "Press L&R to set 3 map cells",0

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

