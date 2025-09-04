
namespace Text {
namespace Font {
    map '0', 0x01, 10
    map 'A', 0x0b, 26
    map 'a', 0x25, 26
    map '!', 0x3f, 15
    map ':', 0x4e, 7
    map '[', 0x55, 6
    map '{', 0x5b, 4
    map ' ', 0x5f

    map '\n', 0xff

    constant ZERO = 0x01
    constant CAPITAL_A = 0x0b
    constant LOWER_A = 0x25
    constant SPACE = 0x5f
    constant MINUS = 0x4b

    constant NEW_LINE = 0xff



constant _BSOD_BG_COLOR = 13 << Palette.blue.shift
constant _BSOD_TEXT_COLOR = 0x7fff

BsodPalette:
    dw  _BSOD_BG_COLOR
    dw  _BSOD_TEXT_COLOR
    constant BsodPalette.size = pc() - BsodPalette
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

