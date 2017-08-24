// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

include "../gen/metasprites.inc"
include "../resources/text/text.inc"

namespace Text {
    namespace Font {
rodata(rom0)
        insert FixedTiles, "../gen/font-fixed.1bpp"
        constant FixedTiles.bitDepth = 1
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

