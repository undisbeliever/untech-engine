// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

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
        jsr     WaitFrame

        lda.l   JOY1
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

