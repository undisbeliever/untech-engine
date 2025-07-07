// room/maps.inc
// =============
//
// Room map routines
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


namespace Room {
namespace Maps {


// Initialize the Room Subsystem
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
macro _Init() {
    assert16a()
    assert16i()
}



// Load and verify the Map and Submap data
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
//
// OUTPUT: X = address after map data
macro _Load() {
    assert16a()
    assert16i()

    MetaTiles._LoadMap()

    lda.w   MetaTiles.map.dataSize
    clc
    adc.w   #roomData + Format.Header.size
    tax


    // ::TODO submaps::
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

