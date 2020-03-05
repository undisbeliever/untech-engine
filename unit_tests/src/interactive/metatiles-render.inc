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
            //      pos = mapMap - 1
            //
            //  if pos < Camera.pos:
            //      Camera.pos = Pos
            //  else if pos - displaySize > Camera.pos:
            //      Camera.pos = pos - displaySize

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


// Generates the map.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// REQUIRES: MAP_WIDTH, MAP_HEIGHT, MAP_REPEAT_HORIZONTAL & MAP_REPEAT_VERTICAL set
macro GenerateMap() {
    assert16a()
    assert16i()

constant _rowsLeft = Test.dpTmp
constant _rowIndex = Test.dpTmp + 2

    constant MAP_GRID_WIDTH = 3
    constant MAP_GRID_HEIGHT = 3
    constant MAP_GRID_TILE_OFFSET = 16
    constant MAP_GRID_LINE_WIDTH = 16

    assert(MAP_WIDTH < 256)
    assert(MAP_HEIGHT == MetaTiles.MAP_HEIGHT_SMALL || MAP_HEIGHT == MetaTiles.MAP_HEIGHT_LARGE)
    assert(MAP_GRID_WIDTH * MAP_REPEAT_HORIZONTAL <= MAP_WIDTH)
    assert(MAP_GRID_HEIGHT * MAP_REPEAT_VERTICAL <= MAP_HEIGHT)
    assert(MAP_WIDTH * MAP_HEIGHT < MetaTiles.MAP_SIZE)


    lda.w   #MAP_WIDTH
    sta.w   MetaTiles.map.width

    lda.w   #MAP_HEIGHT
    sta.w   MetaTiles.map.bytesPerColumn

    lda.w   #MetaTiles.map.TOP + MAP_HEIGHT * 16
    sta.w   MetaTiles.map.bottom

    lda.w   #MetaTiles.map.LEFT + MAP_WIDTH * 16
    sta.w   MetaTiles.map.right

    lda.w   #MAP_WIDTH * MAP_HEIGHT
    sta.w   MetaTiles.map.dataSize


    // clear tilemap

    assert(MetaTiles.MAP_SIZE % 2 == 0)
    ldx.w   #MetaTiles.MAP_SIZE - 2
    ClearLoop:
        stz.w   MetaTiles.map.data,x
        dex
        dex
        bpl     ClearLoop


    // Repeat grid pattern

    constant MARGIN_X = (MAP_WIDTH - MAP_REPEAT_HORIZONTAL * MAP_GRID_WIDTH) / 2
    constant MARGIN_Y = (MAP_HEIGHT - MAP_REPEAT_VERTICAL * MAP_GRID_HEIGHT) / 2

    assert(MARGIN_Y > 1)
    assert(MARGIN_X > 1)

    ldx.w   #(MARGIN_Y + MARGIN_X * MAP_HEIGHT)

    lda.w   #MAP_REPEAT_HORIZONTAL
    sta.b   _rowsLeft

    Loop:
        stx.b   _rowIndex

        ldy.w   #MAP_REPEAT_VERTICAL

        LineLoop:
            variable yPos = 0
            while yPos < MAP_GRID_HEIGHT {
                variable xPos = 0
                while xPos < MAP_GRID_WIDTH {
                    lda.w   #(xPos + yPos * MAP_GRID_LINE_WIDTH + MAP_GRID_TILE_OFFSET)
                    sta.w   MetaTiles.map.data + (yPos + xPos * MAP_HEIGHT),x

                    xPos = xPos + 1
                }
                yPos = yPos + 1
            }

            txa
            clc
            adc.w   #MAP_GRID_HEIGHT
            tax

            dey
            bne     LineLoop

        lda.b   _rowIndex
        clc
        adc.w   #MAP_HEIGHT * MAP_GRID_WIDTH
        tax

        dec.b   _rowsLeft
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
//
// REQUIRES: MetaTile map loaded
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function DoTest {
    Text.Console.SetCursor(0, 19)
    Text.Console.PrintString(UnitTestMessage)

    jsr     Entity.Init
    jsr     Resources.Init


    // ::TODO move to resources scene data::
    sep     #$20
a8()
    // Use BG2 for metatiles
    lda.b   #1 * 2
    sta.w   Resources.AnimatedTileset.bgIndex

    rep     #$30
a16()


    lda.w   #Project.MetaTileTilesetList.MetaTileRenderTest
    jsr     MetaTiles.LoadTileset

    lda.w   #Project.PaletteList.MetaTileRenderTest
    jsr     Resources.Palette.LoadPalette


    lda.w   MetaTiles.map.right
    lsr
    adc.w   #MetaTiles.map.LEFT / 2
    sta.w   Camera.xPos
    clc
    adc.w   #Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   MetaTiles.map.bottom
    lsr
    adc.w   #MetaTiles.map.TOP / 2
    sta.w   Camera.yPos
    clc
    adc.w   #Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.yPos

    lda.w   #Project.EntityIds.MapCursor
    jsr     Entity.Spawn
    bcs     +
        rts
    +


    jsr     Resources.TransferToPpu

    sep     #$20
a8()
    // ::TODO move elsewhere::
    lda.b   #TM.bg2 | TM.bg3 | TM.obj
    sta.l   TM

    rep     #$30
a16()


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


a16()
i16()
code()
Test.add("MetaTile Render (Small map)")
function SmallMapHeightTest {
    constant MAP_WIDTH = 84
    constant MAP_HEIGHT = MetaTiles.MAP_HEIGHT_SMALL

    constant MAP_REPEAT_HORIZONTAL = 24
    constant MAP_REPEAT_VERTICAL = 19

    GenerateMap()

    jmp     DoTest
}


a16()
i16()
code()
Test.add("MetaTile Render (Large map)")
function LargeMapHeightTest {
    constant MAP_WIDTH = 84
    constant MAP_HEIGHT = MetaTiles.MAP_HEIGHT_LARGE

    constant MAP_REPEAT_HORIZONTAL = 24
    constant MAP_REPEAT_VERTICAL = 40

    GenerateMap()

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

