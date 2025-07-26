// SPDX-FileCopyrightText: © 2021 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2021 Marcus Rowe <undisbeliever@gmail.com>
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

// Tests:
//   * Room Script Triggers


namespace InteractiveTests {
namespace ScriptTriggerTest {


// NOTE: The room used by this test contains `PlayerScriptTrigger` tiles
//       that do not have a script trigger attached to them.


constant TEST_TIME       = 4 * 60

constant EXPECTED_FIRST  = 2
constant EXPECTED_SECOND = 4
constant EXPECTED_THIRD  = 1
constant EXPECTED_FOURTH = 4


a16()
i16()
code()
Test.add("Room Script Trigger Test")
function Test {

constant _timer  = Test.zpTmp + 0


    lda.w   #Project.RoomList.ScriptTriggerTest
    ldx.w   #Project.PlayerIds.InteractiveTilesTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    jsr     Test.PrintCurrentTestName


    jsr     Resources.TransferToPpu

    jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far

    sep     #$20
a8()

    Text.Console.SetCursor(0, 3)
    Text.Console.PrintConstString("First:\n")
    Text.Console.PrintConstString("Second:\n")
    Text.Console.PrintConstString("Third:\n")
    Text.Console.PrintConstString("Fourth:\n")

    jsr     EnableDisplay_Full


    // Ensure test room uses all available script triggers
    lda.w   Room.ScriptTriggers.lastTriggerIndex
    cmp.b   #Room.Format.Header.ScriptTriggers.LAST_INDEX
    beq     +
        jmp     Fail
    +

    rep     #$30
a16()

    lda.w   #TEST_TIME
    sta.b   _timer

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        Text.Console.SetCursor(8, 3)
        ldy.w   Project.GameState.Words.first
        jsr     Text.Console.PrintU16Y

        Text.Console.SetCursor(8, 4)
        ldy.w   Project.GameState.Words.second
        jsr     Text.Console.PrintU16Y

        Text.Console.SetCursor(8, 5)
        ldy.w   Project.GameState.Words.third
        jsr     Text.Console.PrintU16Y

        Text.Console.SetCursor(8, 6)
        ldy.w   Project.GameState.Words.fourth
        jsr     Text.Console.PrintU16Y


        jsr     GameLoop.ProcessFrame

        dec.b   _timer
        bne     Loop


    // Confirm counters are correct at the end of the test
    lda.w   Project.GameState.Words.first
    cmp.w   #EXPECTED_FIRST
    bne     Fail

    lda.w   Project.GameState.Words.second
    cmp.w   #EXPECTED_SECOND
    bne     Fail

    lda.w   Project.GameState.Words.third
    cmp.w   #EXPECTED_THIRD
    bne     Fail

    lda.w   Project.GameState.Words.fourth
    cmp.w   #EXPECTED_FOURTH
    bne     Fail


    jsl     Entity.ValidateEntityLists__far

    jmp     Test.ResetPpuState


Fail:
    clc
    rts
}


}
}

