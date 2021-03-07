// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace TextTest {
namespace String {

a16()
i16()
code()
Test.add("Text.String.U16Y_ToString")
function U16Y_ToString {
    sep     #$20
a8()
    ldy.w   #0
    jsr     Text.String.U16Y_ToString

    lda.w   0,x
    cmp.b   #Text.Font.ZERO
    bne     Fail

    lda.w   1,x
    bne     Fail


    ldy.w   #0xffff
    jsr     Text.String.U16Y_ToString

    ldy.w   #Str_FFFF
    jsr     _TestString
    bcc     Fail


    ldy.w   #812
    jsr     Text.String.U16Y_ToString
    ldy.w   #Str_812
    jmp     _TestString

Fail:
    clc
    rts

rodata(rom0)
Str_FFFF:
    db  "65535", 0

Str_812:
    db  "812", 0
}


a16()
i16()
code()
Test.add("Text.String.U32XY_ToString")
function U32XY_ToString {
    sep     #$20
a8()
    ldy.w   #0
    ldx.w   #0
    jsr     Text.String.U32XY_ToString

    lda.w   0,x
    cmp.b   #Text.Font.ZERO
    bne     Fail

    lda.w   1,x
    bne     Fail


    ldy.w   #0xffff
    ldx.w   #0xffff
    jsr     Text.String.U32XY_ToString

    ldy.w   #Str_FFFFFFFF
    jsr     _TestString
    bcc     Fail


    ldy.w   #1000000 & 0xffff
    ldx.w   #1000000 >> 16
    jsr     Text.String.U32XY_ToString
    ldy.w   #Str_Million
    jmp     _TestString

Fail:
    clc
    rts

rodata(rom0)
Str_FFFFFFFF:
    db  "4294967295", 0

Str_Million:
    db  "1000000", 0
}


a16()
i16()
code()
Test.add("Text.String.U16Y_ToPaddedString")
function U16Y_ToPaddedString {
    sep     #$20
a8()
    lda.b   #Text.Font.CAPITAL_A
    sta.w   Text.paddingChar

    ldy.w   #0
    lda.b   #4
    jsr     Text.String.U16Y_ToPaddedString

    ldy.w   #Str_0
    jsr     _TestString
    bcc     Fail

    sec
    rts

    // test not enough padding
    ldy.w   #0xffff
    lda.b   #2
    jsr     Text.String.U16Y_ToPaddedString

    ldy.w   #Str_FFFF
    jsr     _TestString
    bcc     Fail


    // test same amount of padding
    ldy.w   #111
    lda.b   #3
    jsr     Text.String.U16Y_ToPaddedString
    ldy.w   #Str_111
    jmp     _TestString

Fail:
    clc
    rts

rodata(rom0)
Str_0:
    db  "AAA0", 0

Str_FFFF:
    db  "65535", 0

Str_111:
    db  "111", 0
}


a16()
i16()
code()
Test.add("Text.String.U32XY_ToPaddedString")
function U32XY_ToPaddedString {
    sep     #$20
a8()
    lda.b   #Text.Font.CAPITAL_A + 1
    sta.w   Text.paddingChar

    ldy.w   #0
    ldx.w   #0
    lda.b   #0
    jsr     Text.String.U32XY_ToPaddedString

    lda.w   0,x
    cmp.b   #Text.Font.ZERO
    bne     Fail

    lda.w   1,x
    bne     Fail

    // not enough padding
    ldy.w   #1000 & 0xffff
    ldx.w   #1000 >> 16
    lda.b   #8
    jsr     Text.String.U32XY_ToPaddedString

    ldy.w   #Str_1000
    jsr     _TestString
    bcc     Fail

    // test too little padding
    ldy.w   #1000000 & 0xffff
    ldx.w   #1000000 >> 16
    lda.b   #4
    jsr     Text.String.U32XY_ToPaddedString
    ldy.w   #Str_Million
    jmp     _TestString

Fail:
    clc
    rts

rodata(rom0)
Str_1000:
    db  "BBBB1000", 0

Str_Million:
    db  "1000000", 0
}


a16()
i16()
code()
Test.add("Text.String.CompareEqual")
function CompareEqual {
    sep     #$20
a8()
    // Test two null strings
    ldx.w   #String_Null
    lda.b   #String_Null >> 16
    stx.b   Text.str1
    sta.b   Text.str1 + 2
    stx.b   Text.str2
    sta.b   Text.str2 + 2

    jsr     Text.String.CompareEqual
    bcc     Fail


    // Test null against message - comparison fails
    ldx.w   #String_Text
    lda.b   #String_Text >> 16
    stx.b   Text.str2
    sta.b   Text.str2 + 2

    jsr     Text.String.CompareEqual
    bcs     Fail


    // Test two strings same length - comparison fails
    ldx.w   #String_Short1
    lda.b   #String_Short1 >> 16
    stx.b   Text.str1
    sta.b   Text.str1 + 2

    ldx.w   #String_Short2
    lda.b   #String_Short2 >> 16
    stx.b   Text.str2
    sta.b   Text.str2 + 2

    jsr     Text.String.CompareEqual
    bcs     Fail


    // Test equal
    ldx.w   #String_Text
    lda.b   #String_Text >> 16
    stx.b   Text.str1
    sta.b   Text.str1 + 2
    stx.b   Text.str2
    sta.b   Text.str2 + 2

    jsr     Text.String.CompareEqual
    bcc     Fail

    // c set
    rts

Fail:
    clc
    rts

rodata(rom0)
String_Null:
    db  0

String_Text:
    db  "The quick brown fox jumped over the lazy dog.", 0

String_Short1:
    db  "Hello World!", 0
String_Short1.end:

String_Short2:
    db  "Hello Tester", 0
String_Short2.end:

assert(String_Short1.end - String_Short1 == String_Short2.end - String_Short2)
}


// REQUIRES: 8 bit A, 16 bit Index, DP = 0
// IN: X = string from string function (in bank 0x7e)
// IN: Y = string to compare against (rom0 bank)
// OUT: C set if equal
a8()
i16()
code()
function _TestString {
    stx.b   Text.str1
    lda.b   #0x7e
    sta.b   Text.str1 + 2

    sty.b   Text.str2
    lda.b   #RomBank
    sta.b   Text.str2 + 2

    jmp     Text.String.CompareEqual

rodata(rom0)
    constant RomBank = pc() >> 16
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

