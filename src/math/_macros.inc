// math/_macros.inc
// ================
//
// Macros for simple mathematical operations.
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


namespace Math {

// Negate A (8 bit A)
macro neg.b() {
    assert8a()

    eor.b   #0xff
    inc
}


// Negate A (16 bit A)
macro neg.w() {
    assert16a()

    eor.w   #0xffff
    inc
}


// Negates the 32 bit value in XY
//
// REQUIRES: 16 bit Index, 16 bit A
// MODIFIES: A, X, Y
macro Negate32xy() {
    assert16a()
    assert16i()

    tya
    eor.w   #0xffff
    clc
    adc.w   #1
    tay
    txa
    eor.w   #0xffff
    adc.w   #0
    tax
}


// Negates the 32 bit variable in DP
//
// REQUIRES: 16 bit A
// MODIFIES: A
macro Negate32.b(var) {
    assert16a()

    lda.b   {var}
    eor.w   #0xffff
    clc
    adc.w   #1
    sta.b   {var}
    lda.b   {var} + 2
    eor.w   #0xffff
    adc.w   #0
    sta.b   {var} + 2
}


// Negates the 32 bit variable
//
// REQUIRES: 16 bit A
// MODIFIES: A
macro Negate32.w(var) {
    assert16a()

    lda.w   {var}
    eor.w   #0xffff
    clc
    adc.w   #1
    sta.w   {var}
    lda.w   2 + {var}
    eor.w   #0xffff
    adc.w   #0
    sta.w   2 + {var}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

