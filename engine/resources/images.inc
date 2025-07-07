// resources/background-image.inc
// ==============================
//
// Image resource management.
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


RegisterBreakId(INVALID_IMAGE_ID)


namespace Resources {

// Load a background image resource
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// MODIFIES: decompressionBuffer
//
// INPUT: A = background Image ID
//        Y = bgIndex for the background to load
a16()
i16()
code()
function LoadBackgroundImage {
constant _mapVramWaddr    = utZpTmp0
constant _tilesVramWaddr  = utZpTmp1
constant _tilemapDataSize = utZpTmp2

constant _ptr = LZ4.ptr
assertZeroPage(_ptr)

assert(Format.BackgroundImageHeader.size == 1)

    // Get data address
    cmp.w   #BackgroundImageList.count
    bcc     +
        break(INVALID_IMAGE_ID)
    +
    sta.b   _ptr
    asl
    clc
    adc.b   _ptr
    tax

    lda.l   Resources.BackgroundImageList,x
    sta.b   _ptr
    lda.l   Resources.BackgroundImageList + 1,x
    sta.b   _ptr + 1


    // Calculate VRAM locations
    // Y = BG index
    tya
    jsr     Resources.SceneLayout.CalcTileAndMapVramWaddr
    sta.b   _tilesVramWaddr
    sty.b   _mapVramWaddr


    assert(Format.BackgroundImageHeader.tilemapCount == 0)
    lda     [_ptr]
    inc.b   _ptr

    and.w   #Format.BackgroundImageHeader.tilemapCount.mask
    inc
    assert(Format.BackgroundImageHeader.TILEMAP_SIZE == 1 << (8 + 3))
    xba
    asl
    asl
    asl
    sta.b   _tilemapDataSize

    jsr     DecompressData_FromPtr

    // Add two entries to VramTransferBuffer
    //
    //  entry 1: tile data
    //           size        = decompression size - _tilemapDataSize
    //           source      = decompression start + _tilemapDataSize
    //           destination = _vramTileWaddr
    //
    //  entry 2: map data
    //           size        = _tilemapDataSize
    //           source      = decompression start
    //           destination = _vramMapWaddr

    // X = start of decompression buffer
    // A = size of decompression buffer

    ldy.w   VramTransferBuffer.currentIndex
    cpy.w   #VramTransferBuffer.LAST_INDEX - 1      // -1 as we are adding two entries
    bcc     +
        break(TOO_MANY_RESOURCES)
    +

    sec
    sbc.b   _tilemapDataSize
    sta.w   VramTransferBuffer.size + 0,y

    lda.b   _tilemapDataSize
    sta.w   VramTransferBuffer.size + 2,y

    txa
    sta.w   VramTransferBuffer.sourceWram7fAddr + 2,y
    clc
    adc.b   _tilemapDataSize
    sta.w   VramTransferBuffer.sourceWram7fAddr + 0,y

    lda.b   _tilesVramWaddr
    sta.w   VramTransferBuffer.destinationVramWaddr + 0,y

    lda.b   _mapVramWaddr
    sta.w   VramTransferBuffer.destinationVramWaddr + 2,y

    iny
    iny
    iny
    iny
    sty.w   VramTransferBuffer.currentIndex

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

