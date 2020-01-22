// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//  * MetaTiles.Render
//  * MetaTiles.LoadTileset
//  * MetaTiles.MoveEntityAndProcessTileCollisions
//  * MetaSprite Tile Collision Hitbox

namespace Entities {

namespace TileCollisionTestPlayer {
    buildFunctionTable(BaseEntityFunctionTable, TileCollisionTestPlayer)

    namespace EntityStruct {
    childstruct(BaseEntity)
        field(currentFrame, 2)
        field(gravityIndex, 1)
    endstruct()
    }

    constant X_MOMENTUM  = 0x028000
    constant Y_MOMENTUM  = 0x010000


    // REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
    // INPUT: DP = entity
    // OUTPUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        stz.b   EntityStruct.gravityIndex

        lda.w   #0
        sta.b   EntityStruct.currentFrame
        SetFrame_A()

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
        // Confirm entity momentum is a 24 bit value
        assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)

        // Update movement
        lda.w   Controller.Joy1.current
        bit.w   #JOY.left | JOY.right
        beq     NoLeftRight
            bit.w   #JOY.left
            beq     +
                lda.w   #Entity.MovementState.MOVING_RIGHT_FLAG
                trb.b   EntityStruct.movementState
                bra     ++
            +
            bit.w   #JOY.right
            beq     +
                lda.w   #Entity.MovementState.MOVING_RIGHT_FLAG
                tsb.b   EntityStruct.movementState
            +

            lda.w   #X_MOMENTUM
            sta.b   BaseEntity.xMomentum.sx
            lda.w   #X_MOMENTUM >> 8
            sta.b   BaseEntity.xMomentum.sx + 1

            bra     EndLeftRight

        NoLeftRight:
            stz.b   BaseEntity.xMomentum.sx
            stz.b   BaseEntity.xMomentum.sx + 1
    EndLeftRight:


        lda.w   Controller.Joy1.current
        bit.w   #JOY.up | JOY.down
        beq     NoUpDown
            bit.w   #JOY.up
            beq     +
                lda.w   #Entity.MovementState.MOVING_DOWN_FLAG
                bit.b   EntityStruct.movementState
                beq     +
                    trb.b   EntityStruct.movementState
                    // Clear standing flag
                    lda.w   #Entity.MovementState.STANDING_FLAG
                    trb.b   EntityStruct.movementState
            +
            lda.w   Controller.Joy1.current
            bit.w   #JOY.down
            beq     +
                lda.w   #Entity.MovementState.MOVING_DOWN_FLAG
                bit.b   EntityStruct.movementState
                bne     +
                    tsb.b   EntityStruct.movementState
                    // Clear standing flag
                    lda.w   #Entity.MovementState.STANDING_FLAG
                    trb.b   EntityStruct.movementState
            +

            lda.w   #Y_MOMENTUM
            sta.b   BaseEntity.yMomentum.sx
            lda.w   #Y_MOMENTUM >> 8
            sta.b   BaseEntity.yMomentum.sx + 1

            bra     EndUpDown

        NoUpDown:
            lda.b   BaseEntity.movementState
            bit.w   #Entity.MovementState.NO_GRAVITY_FLAG
            beq     +
                // No Gravity, clear momentum when not pressing up or down
                stz.b   BaseEntity.yMomentum.sx
                stz.b   BaseEntity.yMomentum.sx + 1
            +
    EndUpDown:


        // Change frame on L press
        lda.w   Controller.Joy1.pressed
        bit.w   #JOY.l
        beq     +
            assert({ExportOrder}.Frames.frame8 == 7)
            lda.b   EntityStruct.currentFrame
            inc
            and.w   #7
            sta.b   EntityStruct.currentFrame
            SetFrame_A()
        +

        // Change gravity settings on R press
        sep     #$20
    a8()
        lda.w   Controller.Joy1.pressed
        bit.b   #JOYL.r
        beq     EndChangeGravity
            // Alternate between down gravity, up gravity and no gravity
            lda.b   EntityStruct.movementState
            bit.b   #Entity.MovementState.NO_GRAVITY_FLAG
            beq     +
                and.b   #~(Entity.MovementState.NO_GRAVITY_FLAG | Entity.MovementState.UP_GRAVITY_FLAG)
                bra     ++
            +
            lda.b   EntityStruct.movementState
            eor.b   #Entity.MovementState.UP_GRAVITY_FLAG
            bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
            bne     +
                ora.b   #Entity.MovementState.NO_GRAVITY_FLAG
                ora.b   #Entity.MovementState.UP_GRAVITY_FLAG
            +
            sta.b   EntityStruct.movementState
    EndChangeGravity:

        rep     #$20
    a16()

        MoveEntityAndProcessTileCollisions()

        // Move camera to center of object
        lda.b   EntityStruct.xPos.px
        clc
        adc.w   #Camera.DISPLAY_WIDTH / 2
        cmp.w   MetaTiles.map.right
        bcc     +
            lda.w   MetaTiles.map.right
        +
        sec
        sbc.w   #Camera.DISPLAY_WIDTH
        cmp.w   #MetaTiles.map.LEFT
        bcs     +
            lda.w   #MetaTiles.map.LEFT
        +
        sta.w   Camera.xPos

        jmp     Entity.DrawAndGotoNextEntity
    }
}
}


namespace InteractiveTests {
namespace TileCollisionTest {

constant ENTITY_X_START = 32
constant ENTITY_Y_START = 48

constant GRAVITY = 1555

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


// Loads the map into memory.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro LoadMap() {
    assert16a()
    assert16i()

constant _columnsLeft = Test.dpTmp
constant _columnIndex = Test.dpTmp + 2

    assert(MetaTiles.map.width + 1 == MetaTiles.map.height)
    lda.w   #Map.WIDTH | (Map.HEIGHT << 8)
    sta.w   MetaTiles.map.width

    lda.w   #MetaTiles.map.LEFT + Map.WIDTH * 16
    sta.w   MetaTiles.map.right

    lda.w   #MetaTiles.map.TOP + Map.HEIGHT * 16
    sta.w   MetaTiles.map.bottom

    lda.w   #Map.Data.size
    sta.w   MetaTiles.map.dataSize

    lda.w   #Map.WIDTH
    sta.w   MetaTiles.map.bytesPerRow

    lda.w   #Map.WIDTH * MetaTiles.METATILE_DISPLAY_HEIGHT
    sta.w   MetaTiles.map.bytesPerDisplayHeight


    // copy tile map

    assert(MetaTiles.MAP_SIZE % 2 == 0)
    ldx.w   #Map.Data.size
    -
        lda.l   Map.Data,x
        sta.w   MetaTiles.map.data,x
        dex
        dex
        bpl     -
}


// Tile Collision Test
a16()
i16()
code()
Test.add("Tile Collision Test")
function Test {
    Text.Console.SetCursor(0, 21)
    Text.Console.PrintString(UnitTestMessage)

constant _entity = Test.tmp + 0

    sep     #$20
a8()

    jsr     ForceBlank

    SetupPpu()

    rep     #$30
a16()

    jsr     Entity.Init
    jsr     Resources.Init


    lda.w   #Project.MetaTileTilesetList.TileCollisionTest
    jsr     MetaTiles.LoadTileset

    lda.w   #Project.PaletteList.TileCollisionTest
    jsr     Resources.Palette.LoadPalette

    LoadMap()

    lda.w   #GRAVITY
    sta.w   Room.gravity_sx

    lda.w   #MetaTiles.map.LEFT
    sta.w   Camera.xPos

    lda.w   #MetaTiles.map.TOP
    sta.w   Camera.yPos


    lda.w   #MetaTiles.map.LEFT + ENTITY_X_START
    sta.w   Entity.Spawn.xPos

    lda.w   #MetaTiles.map.TOP + ENTITY_Y_START
    sta.w   Entity.Spawn.yPos

    lda.w   #Project.EntityIds.TileCollisionTestPlayer
    jsr     Entity.Spawn
    bcs     +
        rts
    +
    sty.w   _entity


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

        jsr     EntityLoop.ProcessFrame

        jsr     Resources.ProcessAnimations

        jsr     MetaTiles.Render.Update

        jsr     MetaSprite.Render.EndLoop


        sep     #$20
    a8()
        Text.Console.SetCursor(24, 2)

        ldx.w   _entity
        lda.b   BaseEntity.movementState,x
        jsr     Text.Console.PrintHex8A

        rep     #$20
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


// ROM Data
// ========

rodata(rom0)
UnitTestMessage:
    db  "Use D-PAD to move entity\n"
    db  "Press L to change size\n"
    db  "Press R to change gravity\n"
    db  0


// ::TODO replace with room subsystem::
rodata(rom0)
namespace Map {
    constant WIDTH = 32
    constant HEIGHT = 14

Data:
    db  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
    db  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
    db  0x11, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    db  0x13, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x10, 0x10, 0x10, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x10, 0x10, 0x10, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x10, 0x14, 0x20, 0x15, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x16, 0x17, 0x20, 0x18, 0x19, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x14, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x14, 0x20, 0x11, 0x20, 0x15, 0x10, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19
    db  0x10, 0x16, 0x17, 0x20, 0x11, 0x11, 0x11, 0x20, 0x18, 0x19, 0x10, 0x10, 0x10, 0x14, 0x20, 0x11
    db  0x11, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
    db  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x1a, 0x20, 0x11, 0x20, 0x1b, 0x10, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    db  0x10, 0x1c, 0x1d, 0x20, 0x11, 0x11, 0x11, 0x20, 0x1e, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x10, 0x1a, 0x20, 0x1b, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x1c, 0x1d, 0x20, 0x1e, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
    db  0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x12, 0x12, 0x12
    db  0x10, 0x13, 0x13, 0x13, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1c, 0x1d, 0x20, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x12
    db  0x12, 0x12, 0x10, 0x10, 0x10, 0x10, 0x10, 0x14, 0x20, 0x11, 0x10, 0x10, 0x10, 0x10, 0x1a, 0x11
    db  0x11, 0x10, 0x10, 0x10, 0x11, 0x20, 0x15, 0x10, 0x11, 0x10, 0x14, 0x20, 0x11, 0x10, 0x10, 0x12
    db  0x12, 0x12, 0x10, 0x10, 0x10, 0x16, 0x17, 0x20, 0x20, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11
    db  0x11, 0x18, 0x19, 0x10, 0x11, 0x11, 0x20, 0x15, 0x11, 0x14, 0x20, 0x11, 0x11, 0x10, 0x10, 0x12
    db  0x12, 0x12, 0x10, 0x16, 0x17, 0x20, 0x20, 0x20, 0x20, 0x11, 0x10, 0x10, 0x10, 0x10, 0x16, 0x11
    db  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
    db  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11
constant Data.size = pc() - Data
assert(Data.size == WIDTH * HEIGHT)
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

