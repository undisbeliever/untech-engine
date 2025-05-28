// common/print.inc
// ================
//
// Macros for extended printing to the screen.
//
//
// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
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


// Prints a decimal integer number with padding
//
// PARAMS:
//  * width - the printing width
//  * value - the value to print
macro print_padded(evaluate width, evaluate value) {
    variable w = {width} - 1
    variable v = {value}

    if v < 0 {
        v = 0 - v
        w = w - 1
    }

    while w > 0 {
        if v < 10 {
            print " "
        }
        w = w - 1
        v = v / 10
    }

    print {value}
}

// Prints a hexadecimal number with padding
//
// NOTE: cannot print negative numbers
//
// PARAMS:
//  * width - the printing width
//  * value - the value to print
macro print_hex(evaluate width, evaluate value) {
    if {width} > 0 || {value} > 0 {
        print_hex({width} - 1, {value} / 16)

        evaluate d = {value} % 16
        if {d} < 10 {
            print {d}
        } else if {d} == 10 {
            print "a"
        } else if {d} == 11 {
            print "b"
        } else if {d} == 12 {
            print "c"
        } else if {d} == 13 {
            print "d"
        } else if {d} == 14 {
            print "e"
        } else if {d} == 15 {
            print "f"
        }
    }
}

macro print_hex(evaluate value) {
    print_hex(1, {value})
}

// vim: ft=bass-65816 ts=4 sw=4 et:

