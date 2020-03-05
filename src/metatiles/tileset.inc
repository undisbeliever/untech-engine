// metatiles/tileset.inc
// =====================
//
// MetaTile Tileset routines.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_METATILE_TILEMAP)

namespace MetaTiles {

// Load a MetaSprite Tileset into WRAM/VRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, force blank
// DMA: Uses DMA channel 0
// MODIFIES: Disable HDMA, Disable Interrupts, Force Blank
//
// INPUT: A = tileset Id
//        Resources.AnimatedTileset.bgIndex = bgIndex for the animated tileset
a16()
i16()
code()
function LoadTileset {
constant _tilesetPtr = LZ4.ptr

    cmp.w   #MetaTiles.TilesetList.count
    bcs     Fail
    sta.b   _tilesetPtr
    asl
    clc
    adc.b   _tilesetPtr
    tax

    lda.l   MetaTiles.TilesetList,x
    sta.b   _tilesetPtr
    lda.l   MetaTiles.TilesetList + 1,x
    sta.b   _tilesetPtr + 1


    // Transfer tilemap block to WRAM

    ldx.w   #Format.Tileset.wramLocation
    lda.w   #Format.Tileset.wramLocation >> 16
    ldy.w   #Format.Tileset.size

    jsr     LZ4.Decompress


    // ensure tileMap is full
    cpy.w   #Format.Tileset.wramLocation + Format.Tileset.size
    bne     Fail


    // Load tiles

    jmp     Resources.AnimatedTileset.LoadAnimatedTileset_FromPtr


Fail:
    break(INVALID_METATILE_TILEMAP)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

