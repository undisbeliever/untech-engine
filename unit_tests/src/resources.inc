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

include "../gen/project.inc"
include "../resources/text/text.inc"

namespace Text {
    namespace Font {
rodata(rom0)
        insert FixedTiles, "../gen/font-fixed.1bpp"
        constant FixedTiles.bitDepth = 1
    }
}

namespace LZ4Test {
rodata(rom0)
    insert FontFixed_LZ4, "../gen/lz4-test/font-fixed.1bpp.lz4b"
    insert OneKZeros_LZ4, "../gen/lz4-test/1kzeros.lz4b"
    insert Incompressable_LZ4, "../gen/lz4-test/incompressable.lz4b"
}

// vim: ft=bass-65816 ts=4 sw=4 et:

