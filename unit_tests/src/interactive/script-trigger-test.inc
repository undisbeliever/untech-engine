// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//   * Room Script Triggers


namespace InteractiveTests {
namespace ScriptTriggerTest {

constant GRAVITY = 10000

constant TEST_TIME = 4 * 60


a16()
i16()
code()
Test.add("Room Script Trigger Test")
function Test {

constant _timer  = Test.dpTmp + 0


    lda.w   #Project.RoomList.ScriptTriggerTest
    ldx.w   #Project.PlayerIds.InteractiveTilesTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx


    jsr     Test.PrintCurrentTestName


    jsr     Resources.TransferToPpu

    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    sep     #$20
a8()

    // Ensure test room uses all available script triggers
    lda.w   Room.ScriptTriggers.lastTriggerIndex
    cmp.b   #Room.Format.Header.ScriptTriggers.LAST_INDEX
    bne     Fail


    jsr     EnableDisplay_Full

    Text.Console.SetCursor(0, 3)
    Text.Console.PrintConstString("First:\n")
    Text.Console.PrintConstString("Second:\n")
    Text.Console.PrintConstString("Third:\n")

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


        jsr     GameLoop.ProcessFrame

        dec.b   _timer
        bne     Loop


    // ::TODO add count tests::

    jsr     Entity.ValidateEntityLists

    jmp     Test.ResetPpuState


Fail:
    clc
    rts
}


}
}

