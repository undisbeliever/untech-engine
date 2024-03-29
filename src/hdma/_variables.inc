// hdma/_variables.inc
// ===================
//
// Variables used by the HDMA module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

