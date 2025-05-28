// SPDX-FileCopyrightText: © 2018 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2018 Marcus Rowe <undisbeliever@gmail.com>
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



// Wait until the start of the next frame, printing the number
// of loops waited on the top-right of the screen.
//
// REQUIRES: DB access lowram
//
// RETURN: Y = number of loops waited
code()
function WaitFrame_PrintFreeCycles {
    Text.Console.SetCursor(Text.Console.TEXT_WIDTH - 5, 0)

    // Using hex as it is constant time

    jsl     WaitFrame__far

    ldy.w   freeCycles
    jmp     Text.Console.PrintHex16Y
}

// vim: ft=bass-65816 ts=4 sw=4 et:

