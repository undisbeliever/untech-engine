
include "../gen/metasprites.inc"
include "../resources/text/text.inc"

scope Text {
    scope Font {
rodata(rom0)
        insert FixedTiles, "../gen/font-fixed.1bpp"
        constant FixedTiles.bitDepth(1)
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

