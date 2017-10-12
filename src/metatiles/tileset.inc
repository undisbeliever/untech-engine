// metatiles/tileset.inc
// =====================
//
// MetaTile Tileset routines.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_METATILE_TILEMAP)

namespace MetaTiles {

// Load a MetaSprite Tileset into WRAM/VRAM/CGRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, force blank
// DMA: Uses DMA channel 0
// MODIFIES: Disable HDMA, Disable Interrupts, Force Blank
//
// INPUT: A = tileset Id
a16()
i16()
code()
function LoadTileset {
constant _tilesetPtr = dpTmp0
constant _tilesetPtrBank = dpTmp1
constant _tilesSize = dpTmp2

assert(_tilesetPtr + 2 == _tilesetPtrBank)

    cmp.w   #MetaTiles.TilesetList.count
    bcc     +
        break(INVALID_METATILE_TILEMAP)
    +
    sta.b   _tilesetPtr
    asl
    clc
    adc.b   _tilesetPtr
    tax

    lda.l   MetaTiles.TilesetList,x
    sta.b   _tilesetPtr
    lda.l   MetaTiles.TilesetList + 1,x
    sta.b   _tilesetPtr + 1


    // Format.Tileset.tilesSize
    lda     [_tilesetPtr]
    sta.b   _tilesSize


    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB == REGISTER_DB

    // Transfer tilemap data to WRAM

    lda.b   _tilesetPtr
    clc
    adc.w   #Format.Tileset.dataBlocks
    tay

    sep     #$20
a8()
i16()
    // Disable NMI/HDMA to prevent VRAM/CGRAM corruption
    stz.w   NMITIMEN
    stz.w   HDMAEN

    lda.b   #INIDISP.force
    sta.w   INIDISP


    // Store data block address
    // Y = data block address
    sty.w   A1T0
    lda.b   _tilesetPtrBank
    sta.w   A1B0


    // Transfer tileMap data to WRAM

    ldx.w   #MetaTiles.tileMap.topLeft
    stx.w   WMADD

    lda.b   #MetaTiles.tileMap.topLeft >> 16
    sta.w   WMADD + 2

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (WMDATA << 8)
    stx.w   DMAP0   // also sets BBAD0

    ldx.w   #N_TILES * 4 * 2
    stx.w   DAS0

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN


    // Transfer tiles to VRAM

    lda.b   #VMAIN.increment.by1 | VMAIN.incrementMode.high
    sta.w   VMAIN

    ldx.w   #METATILES_TILES_WADDR
    stx.w   VMADD

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.two | (VMDATA << 8)
    stx.w   DMAP0   // also sets BBAD0

    ldx.b   _tilesSize
    stx.w   DAS0

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN


    // Transfer palette to CGRAM

    stz.w   CGADD

    ldx.w   #DMAP.direction.toPpu | DMAP.transfer.one | (CGDATA << 8)
    stx.w   DMAP0   // also sets BBAD0

    ldx.w   #8 * 16 * 2
    stx.w   DAS0

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    plb
    // DB = 0x7e

    rep     #$30
a16()
i16()
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

