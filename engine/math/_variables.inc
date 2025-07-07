// math/_variables.inc
// ===================
//
// Variables used by the Math module.
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

if {defined ENABLE_32_BIT_MULTIPLICATION} {
    allocate(product32, lowram, 4)
    allocate(factor32, lowram, 4)
}

if {defined ENABLE_32_BIT_DIVISION} {
    allocate(divisor32, zeropage, 4)
}

// Used in `Divide.u32_u8a` for printing decimal Text
allocate(dividend32, zeropage, 4)
allocate(remainder32, zeropage, 4)
constant result32 = dividend32


allocate(tmp1, lowram, 2)
allocate(tmp2, lowram, 2)
allocate(tmp3, lowram, 2)

}

// vim: ft=bass-65816 ts=4 sw=4 et:

