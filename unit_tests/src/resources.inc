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

namespace LZ4Test {
rodata(rom0)
    insert FontFixed_LZ4, "../gen/lz4-test/font-fixed.1bpp.lz4b"
    insert OneKZeros_LZ4, "../gen/lz4-test/1kzeros.lz4b"
    insert Incompressable_LZ4, "../gen/lz4-test/incompressable.lz4b"
}

namespace Resources {

namespace PaletteData {
rodata(rom0)
    insert MetaTileRenderTest, "../gen/palettes/metatile-render-test.lz4b"
}

rodata(rom0)
PaletteList:
namespace PaletteList {
    variable i = 0
    inline _add(l) {
        dl PaletteData.{l}
        constant {l} = i
        i = i + 1
    }
    _add(MetaTileRenderTest)


    constant count = i
}
}

namespace MetaTiles {

namespace TilesetData {
}

rodata(rom0)
TilesetList:
namespace TilesetList {
    variable i = 0
    inline _add(l) {
        dl TilesetData.{l}
        constant {l} = i
        i = i + 1
    }
    _add(MetaTileRenderTest)


    constant count = i
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

