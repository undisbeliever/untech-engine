
// ::TODO redraw font::

namespace Text {
namespace Font {
    map '0', 0x01, 10
    map 'A', 0x0b, 26
    map 'a', 0x0b, 26       // no lower case letters
    map '!', 0x25, 15
    map ':', 0x34, 7
    map '[', 0x5b, 6
    map '|', 0x41
    map '~', 0x41
    map ' ', 0x45

    map '\n', 0xff

    constant ZERO = 0x01
    constant CAPITAL_A = 0x0b
    constant SPACE = 0x45
    constant MINUS = 0x32

    constant NEW_LINE = 0xff

    constant SHIELD_LABEL = 0x46
    constant SHIELD_LABEL_LEN = 6

    constant SHIELD_1 = 0x4c
    constant SHIELD_2 = 0x4d
    constant NO_SHIELD_1 = 0x4e
    constant NO_SHIELD_2 = 0xef
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

