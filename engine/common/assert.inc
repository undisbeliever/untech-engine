// common/assert.inc
// =================
//
// Macros that break assembly if a test fails.
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


// Asserts than an expression is true
// Breaks assembly if expression if false
macro assert(test) {
    if !({test}) {
        error "{test}"
    }
}

// Asserts than an expression is a power of two
// Breaks assembly if expression if false
macro assertPowerOfTwo(evaluate test) {
    if ({test} & ({test} - 1)) != 0 {
        error "{test} is not a power of 2"
    }
}

// Asserts that a variable is inside zero page
// Breaks assembly if variable is not zero page
macro assertZeroPage(var) {
    evaluate addr = {var}
    if {addr} >= 0x100 && ({addr} < 0x7e0000 || {addr} >= 0x7e0100) {
        error "{var} is not inside zero page"
    }
}

// Asserts that a variable is a Low-RAM address
// Breaks assembly if variable is not inside Low-RAM
macro assertLowRam(var) {
    evaluate addr = {var}
    if {addr} >= 0x2000 && ({addr} < 0x7e0000 || {addr} >= 0x7e2000) {
        error "{var} is not inside Low-RAM"
    }
}

// Asserts the variable is contained within one page
// Breaks assembly if the variable crosses a page boundary
macro assertOnePage(var, evaluate size) {
    evaluate addr = {var}
    if {addr} >> 8 != ({addr} + {size} - 1) >> 8 {
        error "{var} crossed a page boundary"
    }
}
inline assertOnePage(var) {
    assertOnePage({var}, {var}.size)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

