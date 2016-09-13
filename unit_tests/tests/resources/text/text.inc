
scope Text {
scope Font {

    map '0', 0x01, 10
    map 'A', 0x0b, 26
    map 'a', 0x25, 26
    map '!', 0x3f, 15
    map ':', 0x4e, 7
    map '[', 0x55, 6
    map '{', 0x5b, 4
    map ' ', 0x5f

    constant ZERO(0x01)
    constant CAPITAL_A(0x0b)
    constant LOWER_A(0x25)
    constant SPACE(0x5f)
    constant MINUS(0x4b)


rodata(rom0)
    insert FixedTiles, "font-fixed.2bpp"

    // ::TODO change 1bpp tiles::
    constant FixedTiles.bitDepth(2)
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

