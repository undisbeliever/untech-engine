// hdma/_variables.inc
// ===================
//
// Variables used by the HDMA module.
//
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


namespace Hdma {

// The current buffer index
// (byte)
allocate(currentBuffer, wram7e, 1)

// size of a single HDMA table
constant BUFFER_SIZE = 256 * 6 + 3 + 1

// HDMA Table Buffers
// (double buffered block of data)
allocate(buffer1, wram7e, BUFFER_SIZE)
allocate(buffer2, wram7e, BUFFER_SIZE)

namespace ChannelSettings {
    // Enable HDMA Channel
    //
    // Settings will be copied to HDMA Channel 7 on non-zero during VBlank.
    //
    // (byte)
    allocate(enabled, lowram, 1)

    // HDMA Setup Register Buffer
    // (data matches DMAPx)
    allocate(dmap, lowram, 1)

    // HDMA PPU Register Selection
    // (byte - data matches DDADx)
    allocate(register, lowram, 1)

    // Word address of HDMA table (bank is always 0x7E).
    // (word addr)
    allocate(tableAddr, lowram, 2)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

