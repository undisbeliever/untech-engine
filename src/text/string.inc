// text/string.inc
// ===============
//
// String manipulation routines.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Text {
namespace String {

allocateTmpWord(tmp_paddingLength)

// Converts an unsigned 16 bit integer to a string.
//
// Will use `tmpString` as string storage space.
//
// REQUIRES: 16 bit Index, 8 bit A, DB = 0x7e
//
// NOTE: Uses division registers
//
// INPUT: Y = uint16 number
// OUTPUT: X = address of string (in bank 0x7e)
a8()
i16()
code()
function U16Y_ToString {
    assert(tmpString.size >= 5)

    ldx.w   #tmpString + tmpString.size - 1

AfterLoadX:
    phd

    lda.b   #WRDIV >> 8
    xba
    lda.b   #0
    tcd

    Loop:
        sty.b   WRDIV
        lda.b   #10
        sta.b   WRDIVB

        // wait 16 cycles
        nop                                     // 2
        nop                                     // 2
        stz.w   tmpString + tmpString.size - 1  // 4
        dex                                     // 2
        clc                                     // 2
        lda.b   #Font.ZERO                      // 2
        adc.b   RDMPY                           // 2 instruction fetch

        sta.w   0,x

        ldy.b   RDDIV
        bne     Loop

    pld
    rts
}


// Converts an unsigned 32 bit integer to a string.
//
// Will use `tmpString` as string storage space.
//
// REQUIRES: 16 bit Index, 8 bit A, DB = 0x7eA
//
// NOTE: Uses division registers
//
// INPUT: XY = uint32 number
// OUTPUT: X = address of string (in bank 0x7e)
a8()
i16()
code()
function U32XY_ToString {
    assert(tmpString.size >= 10)
    assert(Font.ZERO == 1)

    cpx.w   #0
    beq     U16Y_ToString

    sty.w   Math.dividend32
    stx.w   Math.dividend32 + 2

    ldx.w   #tmpString + tmpString.size - 1

    Loop:
        phx
            lda.b   #10
            jsr     Math.Divide.u32_u8a

            // result and dividend share the same memory location
            // no need to transfer across
        plx

        dex
        inc
        sta.w   0,x

        ldy.w   Math.dividend32 + 2
        bne     Loop

    // save cycles by switching to uint16 mode when possible
    // Don't store terminating zero, this is done by U16Y_ToString

    ldy.w   Math.dividend32

    // Y is always non-zero

    bra     U16Y_ToString.AfterLoadX
}


// Converts an unsigned 16 bit integer to a padded string.
//
// Will use `tmpString` as string storage space.
//
// REQUIRES: 16 bit Index, 8 bit A, DB = 0x7e
//
// NOTE: Uses division registers
//
// INPUT: Y = uint16 number
//        A = padding length
//        `paddingChar` = padding character
// OUTPUT: X = address of string (in bank 0x7e)
a8()
i16()
code()
function U16Y_ToPaddedString {
    cmp.b   #tmpString.size - 2
    bcc     +
        lda.b   #tmpString.size - 2
    +
    sta.w   tmp_paddingLength

    jsr     U16Y_ToString

ProcessPadding:
    // set Ah to 0 as tay will transfer 16 bits
    lda.b   #0
    xba

    txa
    sec
    sbc.b   #tmpString + tmpString.size - 1
    clc
    adc.w   tmp_paddingLength
    bmi     Return
    beq     Return

    tay

    lda.w   paddingChar
    Loop:
        dex
        sta.w   0,x
        dey
        bne     Loop

Return:
    rts
}


// Converts an unsigned 32 bit integer to a padded string.
//
// Will use `tmpString` as string storage space.
//
// REQUIRES: 16 bit Index, 8 bit A, DB = 0x7e
//
// NOTE: Uses division registers
//
// INPUT: XY = uint32 number
//        A = padding length
//        `paddingChar` = padding character
// OUTPUT: X = address of string (in bank 0x7e)
a8()
i16()
code()
function U32XY_ToPaddedString {
    cmp.b   #tmpString.size - 2
    bcc     +
        lda.b   #tmpString.size - 2
    +
    sta.w   tmp_paddingLength

    pea     U16Y_ToPaddedString.ProcessPadding - 1
    bra     U32XY_ToString
}


// Tests two strings for equality
//
// REQUIRES: 16 bit Index, 8 bit A, DP = 0
//
// INPUT: str1, str2 - far address of strings
// OUTPUT: c set if both strings are equal
//         String pointers are unchanged
a8()
i16()
code()
function CompareEqual {
    ldy.w   #0

    lda     [str1],y
    beq     EndLoop

    Loop:
        cmp     [str2],y
        bne     ReturnFalse

        iny

        lda     [str1],y
        bne     Loop

EndLoop:
    // A = current str1 char
    cmp     [str2],y
    bne     ReturnFalse

    // c set
    rts

ReturnFalse:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

