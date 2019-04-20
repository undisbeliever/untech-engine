// common/tmp_words.inc
// ====================
//
// Marks unused words as a temporary variable to be accessed later.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

