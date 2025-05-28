// errors/break.inc
// ================
//
// Macro to generate a brk interrupt with an identifying string.
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


constant __BREAK__.first = 0x10

variable __BREAK__.pos = __BREAK__.first

// Register a break identifier
//
// If the `breakIdStrings` ROM Block exists, then a string with the
// value of `id` will be appended to the `breakIdStrings` ROM Block.
//
// This macro MUST BE called in the root scope.
//
// PARAM: id - warning identifier
inline RegisterBreakId(id) {
    if __BREAK__.pos > 255 {
        error "Too many Break IDs"
    }

    constant __BREAK__.ids.{id} = __BREAK__.pos
    __BREAK__.pos = __BREAK__.pos + 1

    if {defined __MEMORY__.romBlocks.breakIdStrings} {
        pushBlock()
        rodata(breakIdStrings)
            db  "{id}", 0
        popBlock()
    }
}

// Raise a BRK Interrupt.
//
// The break identifier must be registered with `RegisterBreakId`
//
// PARAM: id - break identifier
macro break(id) {
    brk     #__BREAK__.ids.{id}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

