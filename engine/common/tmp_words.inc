// common/tmp_words.inc
// ====================
//
// Marks unused words as a temporary variable to be accessed later.
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


variable __TMP_WORDS__.nMarks = 0
variable __TMP_WORDS__.nAllocations = 0

// Mark an unused word as a temporary variable that can be accessed
// later.
//
// This macro MUST BE called in the root scope.
//
// var must be in DB 0x7e
inline markTmpWord(evaluate var) {
    assert(({var} >> 16) == 0x7e)

    evaluate __TMP_WORDS__.n = __TMP_WORDS__.nMarks
    constant __TMP_WORDS__.{__TMP_WORDS__.n} = {var}

    __TMP_WORDS__.nMarks = __TMP_WORDS__.nMarks + 1
}

// Allocates a tmp Word to a given variable name.
//
// The variable will be located somewhere in DB 0x7e
//
// If you get a "unrecognized variable: __TMP_WORDS__.*" error
// then either:
//   * a `markTmpWord` macro was not called in the root scope, or
//   * you have run out of tmp words.
inline allocateTmpWord(name) {
    evaluate __TMP_WORDS__.n = __TMP_WORDS__.nAllocations
    constant {name} = __TMP_WORDS__.{__TMP_WORDS__.n}

    __TMP_WORDS__.nAllocations = __TMP_WORDS__.nAllocations + 1
}

// vim: ft=bass-65816 ts=4 sw=4 et:

