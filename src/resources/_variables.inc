// resources/_variables.inc
// ========================
//
// Variables used by the resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

constant decompressionBuffer.size = 32 * 1024
allocate(decompressionBuffer, wram7f, decompressionBuffer.size)


namespace Palettes {
    // memory buffer to store the decompressed palettes in
    // (wram7f)
    constant paletteBuffer.size = 256 * 2
    allocate(paletteBuffer, wram7f, paletteBuffer.size)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

