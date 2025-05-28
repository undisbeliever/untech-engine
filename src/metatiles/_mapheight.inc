// metatiles/_mapheight.inc
// ==========================
//
// Macros for branching/jumping depending on map height.
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


namespace MetaTiles {

// Confirm bit 7 of `map.bytesPerColumn` can be used to determine map height
assert(MAP_HEIGHT_SMALL & 0x80 == 0)
assert(MAP_HEIGHT_LARGE & 0x80 == 0x80)


// Branch to {label} if map height is small
//
// This macro is branch if small (not branch if large) as the large map height
// code uses more CPU time then the small height code.  Branching if small
// helps minimise the difference in CPU time for each code path.
//
// REQUIRES: DB = 0x7e
// PARAM: label - label to branch to if map height is small
// KEEP: A, X, Y
// BRANCH: {label} if map height is small
inline BranchIfMapIsSmall(label) {
    if __CPU__.aSize == 8 {
        bit.w   map.bytesPerColumn
        bpl     {label}

    } else if __CPU__.aSize == 16 {
        bit.w   map.bytesPerColumn - 1
        bpl     {label}

    } else {
        error "Unknown A size"
    }
}

// Branch to {label} if map height is small
//
// This macro is used if invoking `BranchIfMapIsSmall` causes a
// "branch out of bounds" assembly error.
//
// REQUIRES: DB = 0x7e
// PARAM: label - label to branch to if map height is small
// KEEP: A, X, Y
// BRANCH: {label} if map height is small
inline BranchIfMapIsSmall_Long(label) {
    if __CPU__.aSize == 8 {
        bit.w   map.bytesPerColumn
        bmi     _Large_{#}
            jmp     {label}
    _Large_{#}:

    } else if __CPU__.aSize == 16 {
        bit.w   map.bytesPerColumn - 1
        bmi     _Large_{#}
            jmp     {label}
    _Large_{#}:

    } else {
        error "Unknown A size"
    }
}

}

