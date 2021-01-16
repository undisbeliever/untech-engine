// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace ScriptingTests {


// This file requires the scripts in the `ScriptTests` room exist in the following order:
constant ScriptId__Increment_Counter      = 0
constant ScriptId__Test_Set_Clear_Flags   = 1
constant ScriptId__Test_GameState_Words   = 2
constant ScriptId__Test_End_Script        = 3


a16()
i16()
code()
function _LoadScriptTestsRoom {
    lda.w   #Project.RoomList.ScriptTests
    ldx.w   #Project.PlayerIds.Blank
    ldy.w   #0
    jmp     InteractiveTests.GameLoop.Init_LoadRoom
}



// PARAM: n - flag id
// BRANCH: to `label` if flag set
macro BranchIfFlagSet(evaluate n, define label) {
    assert16a()

    lda.w   #1 << ({n} % 8)
    bit.w   GameState.flagData + ({n} / 8)
    bne     {label}
}


// PARAM: n - flag id
// BRANCH: to `label` if flag set
macro BranchIfFlagClear(evaluate n, define label) {
    assert16a()

    lda.w   #1 << ({n} % 8)
    bit.w   GameState.flagData + ({n} / 8)
    beq     {label}
}



a16()
i16()
code()
Test.add("ActivateScript")
function ActivateScript {
    jsr     _LoadScriptTestsRoom

    // Confirm counter reset
    lda.w   Project.GameState.Words.counter
    bne     Fail


    // Activate test script
    lda.w   #ScriptId__Increment_Counter
    jsr     Scripting.ActivateScript
    jsr     Scripting.ProcessScripts


    // Confirm counter incremented
    lda.w   Project.GameState.Words.counter
    cmp.w   #1
    bne     Fail


    // Confirm counter does not increment if script is not active
    jsr     Scripting.ProcessScripts

    lda.w   Project.GameState.Words.counter
    cmp.w   #1
    bne     Fail


    // Confirm counter increments once if script activated twice
    lda.w   #ScriptId__Increment_Counter
    jsr     Scripting.ActivateScript

    lda.w   #ScriptId__Increment_Counter
    jsr     Scripting.ActivateScript

    jsr     Scripting.ProcessScripts

    lda.w   Project.GameState.Words.counter
    cmp.w   #2
    bne     Fail

    sec
    rts


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("GameState Flag Bytecodes")
function GameStateFlagBytecodes {
    jsr     _LoadScriptTestsRoom


    // Confirm the flags are cleared
    BranchIfFlagSet(1, Fail)
    BranchIfFlagSet(300, Fail)


    jsr     Scripting.ProcessScripts


    // Check the flags were unchanged
    BranchIfFlagSet(1, Fail)
    BranchIfFlagSet(300, Fail)


    // Execute the script
    lda.w   #ScriptId__Test_Set_Clear_Flags
    jsr     Scripting.ActivateScript
    jsr     Scripting.ProcessScripts


CheckGameStateChanged:
    // Check the flags were set by the script
    BranchIfFlagClear(1, Fail)
    BranchIfFlagClear(300, Fail)

    sec
    rts


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("GameState Word Bytecodes")
function GameStateWordBytecodes {
    jsr     _LoadScriptTestsRoom


    // Confirm GameState is reset to initial values

    // seventh is a local variable and not accessible by the global engine
    lda.w   GameState.wordData + 7 * 2
    cmp.w   #7
    bne     Fail

    lda.w   Project.GameState.Words.eighth
    cmp.w   #8
    bne     Fail


    lda.w   #ScriptId__Test_GameState_Words
    jsr     Scripting.ActivateScript
    jsr     Scripting.ProcessScripts


CheckGameStateChanged:
    // Confirm GameState has changed
    lda.w   GameState.wordData + 7 * 2
    cmp.w   #7777
    bne     Fail

    lda.w   Project.GameState.Words.eighth
    cmp.w   #8888
    bne     Fail


    sec
    rts


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Test End_Script Bytecode")
function TestEndScriptBytecode {
    jsr     _LoadScriptTestsRoom


    // This script is is two instructions: `End_Script` followed by `Fail`

    lda.w   #ScriptId__Test_End_Script
    jsr     Scripting.ActivateScript
    jsr     Scripting.ProcessScripts

    sec
    rts
}



a16()
i16()
code()
Test.add("Test Multiple Scripts")
function TestMultipleScripts {
    jsr     _LoadScriptTestsRoom


    // Confirm GameState Reset
    BranchIfFlagSet(300, Fail)

    lda.w   Project.GameState.Words.eighth
    cmp.w   #8
    bne     Fail


    // Activate multiple scripts at once
    lda.w   #ScriptId__Test_Set_Clear_Flags
    jsr     Scripting.ActivateScript

    lda.w   #ScriptId__Test_GameState_Words
    jsr     Scripting.ActivateScript


    jsr     Scripting.ProcessScripts


    // Confirm the script changed the GameState

    jsr     GameStateFlagBytecodes.CheckGameStateChanged
    bcc     Fail

    jsr     GameStateWordBytecodes.CheckGameStateChanged
    bcc     Fail


    sec
    rts


Fail:
    clc
    rts
}


}

// vim: ft=bass-65816 ts=4 sw=4 et:

