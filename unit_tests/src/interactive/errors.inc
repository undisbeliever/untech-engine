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

RegisterBreakId(UNIT_TEST_BRK)
Warnings.Register(UNIT_TEST_WARNING, 8, 20, 31)

namespace InteractiveTests {
namespace Errors {

// Tests:
//  * Break Handler
//  * break Macro
//  * Warnings Module

a16()
i16()
code()
Test.add("Errors Test")
function Test {
    Text.Console.SetCursor(0, 4)
    Text.Console.PrintConstString("Press SELECT to trigger BRK\n")
    Text.Console.PrintConstString("Press A/B/X/Y to skip BRK")

    Text.Console.SetCursor(0, 21)
    Text.Console.PrintConstString("A UNIT_TEST_WARNING warning\n")
    Text.Console.PrintConstString("has been issued and this\n")
    Text.Console.PrintConstString("text should be light orange.")

    Warnings.Warn(UNIT_TEST_WARNING)

    Loop:
        jsl     WaitFrame__far

        lda.w   Controller.Joy1.pressed
        bit.w   #JOY.select
        beq     +
            break(UNIT_TEST_BRK)
        +

        // A = JOY1
        bit.w   #JOY.buttons
        beq     Loop

    jmp     Test.ResetPpuState
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

