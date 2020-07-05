// resources/scene-layout.inc
// ==========================
//
// Resource Scene Layout routines/macros
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_SCENE_LAYOUT)

namespace Resources {
namespace SceneLayout {


// Reset the SceneLayout subsystem
//
// REQUIRES: 16 bit A, 16 bit index, DB = 0x7e, DP = 0
macro _Init() {
    stz.w   SceneLayout.dataIndex
}


// Set the current scene layout
//
// REQUIRES: 16 bit A, 16 bit index, DB = 0x7e, DP = 0
//
// INPUT: A = scene layout id
macro _SetSceneLayout() {
    assert16a()
    assert16i()

    assert(SceneLayoutData.count < 256)

    and.w   #0xff
    cmp.w   #SceneLayoutData.count
    bcc     +
    InvalidSceneLayout:
        break(INVALID_SCENE_LAYOUT)
    +

    assert(Format.SceneLayout.size == 1 << 3)
    asl
    asl
    asl

    sta.w   SceneLayout.dataIndex
}


// Calculate VRAM word address of the tile data and map data for a given BG
//
// REQUIRES: 16 bit A, 16 bit index, DB = 0x7e, DP = 0
// REQUIRES: SceneLayout set
//
// INPUT: A = bgIndex
// OUTPUT: A = VRAM word address of the tile data
// OUTPUT: Y = VRAM word address of the tile map
a16()
i16()
code()
function CalcTileAndMapVramWaddr {
    and.w   #Format.SceneLayout.BG_INDEX_MASK
    clc
    adc.w   dataIndex
    tax

    lda.l   Format.SceneLayout.bg1Sc - 1,x
    and.w   #BGXSC.base.mask << 8
    assert(BGXSC.base.walign == 1 << (BGXSC.base.shift + 8))
    tay

    lda.l   Format.SceneLayout.bg1Nba - 1,x
    and.w   #BG12NBA.bg1.mask << 8
    assert(BG12NBA.walign == 1 << (BG12NBA.bg1.shift + 8 + 4))
    asl
    asl
    asl
    asl

    rts
}



// Setup the PPU registers according to the SceneLayout data
//
// REQUIRES: 8 bit A, 16 bit Index, DB = REGISTER_DB, DP = 0, Force-blank
macro _SetupPpuRegisters_RegisterDB() {
    assert8a()
    assert16i()
    // DB = REGISTER_DB

    assertZeroPage(_tmp)


    // Set OBSEL
    assert(VRAM_OBJ_WADDR % OBSEL.base.walign == 0)
    lda.b   #(VRAM_OBJ_WADDR / OBSEL.base.walign) | OBSEL.size.s8_16
    sta.w   OBSEL


    assertShadowRam(SceneLayout.dataIndex)
    ldx.w   SceneLayout.dataIndex


    // Set BG tile map address and size
    lda.l   Format.SceneLayout.bg1Sc,x
    sta.w   BG1SC
    lda.l   Format.SceneLayout.bg2Sc,x
    sta.w   BG2SC
    lda.l   Format.SceneLayout.bg3Sc,x
    sta.w   BG3SC
    lda.l   Format.SceneLayout.bg4Sc,x
    sta.w   BG4SC


    // Set BG tile addresses
    // BG12NBA = (bg1Nba & 0xf) | (bg3Nba << 4)
    // BG34NBA = (bg3Nba & 0xf) | (bg4Nba << 4)
    lda.l   Format.SceneLayout.bg1Nba,x
    and.b   #0x0f
    sta.b   _tmp
    lda.l   Format.SceneLayout.bg2Nba,x
    asl
    asl
    asl
    asl
    ora.b   _tmp
    sta.w   BG12NBA

    lda.l   Format.SceneLayout.bg3Nba,x
    and.b   #0x0f
    sta.b   _tmp
    lda.l   Format.SceneLayout.bg4Nba,x
    asl
    asl
    asl
    asl
    ora.b   _tmp
    sta.w   BG34NBA
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

