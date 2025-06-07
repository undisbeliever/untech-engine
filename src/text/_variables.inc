// text/_variables.inc
// ===================
//
// Variables used by the Text module.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Text {
    // Word address of the tiles of the BG used by the text
    //
    // If negative then the text console is not loaded
    //
    // (vram word address, lowram)
    allocate(tilesVramWaddr, lowram, 2)

    // Word address of the tilemap of the BG used by the text
    //
    // If negative then the text console is not loaded
    //
    // (vram word address, lowram)
    allocate(tilemapVramWaddr, lowram, 2)


    // The buffer in which to hold the data to transfer to VRAM
    //
    // This buffer is only loaded to the low byte of VRAM in order
    // to save space and DMA time
    //
    // (whatever the format)
    constant buffer.size = 32 * 32
    allocate(buffer, wram7e, buffer.size)

    // If non-zero then the buffer is transferred to VRAM on vblank
    // (dp)
    allocate(bufferDirty, zeropage, 1)

    // Variables used by the different text modes
    // (dp)
    constant vars.size = 4
    allocate(vars, zeropage, vars.size)

    // string pointer 1
    // (dp far addr)
    allocate(str1, zeropage, 3)

    // string pointer 2
    // (dp far addr)
    allocate(str2, zeropage, 3)

    // A temporary buffer used for string generation
    // (wram7e)
    constant tmpString.size = 12
    allocate(tmpString, wram7e, tmpString.size)

    // Padding character
    // (wram7e)
    allocate(paddingChar, wram7e, 1)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

