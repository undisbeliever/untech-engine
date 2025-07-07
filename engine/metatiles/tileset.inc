// metatiles/tileset.inc
// =====================
//
// MetaTile Tileset routines.
//
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


RegisterBreakId(INVALID_METATILE_TILEMAP)

namespace MetaTiles {

// Load a MetaSprite Tileset into WRAM/VRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, force blank
// DMA: Uses DMA channel 0
// MODIFIES: Disable HDMA, Disable Interrupts, Force Blank
//
// INPUT: A = tileset Id (only 8 bits read)
//        Y = bgIndex (only 8 bits read)
a16()
i16()
code()
function LoadTileset {
constant _tilesetPtr = LZ4.ptr

    sep     #$30
a8()
i8()
    sty.w   Resources.AnimatedTileset.bgIndex

    cmp.b   #MetaTiles.TilesetList.count
    bcs     Fail

    rep     #$30
a16()
i16()
    and.w   #0xff
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
    beq     +
    Fail:
        break(INVALID_METATILE_TILEMAP)
    +

    jsr     MetaTiles._ResetQueues

    sep     #$20
a8()
    CrumblingTiles._CalculateTransitionDelays()

    rep     #$30
a16()

assert16a()
assert16i()

    // Load tiles

    jmp     Resources.AnimatedTileset.LoadAnimatedTileset_FromPtr
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

