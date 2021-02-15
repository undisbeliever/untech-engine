// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace ScriptingTests {


// This file requires the scripts in the `ScriptTests` room exist in the following order:
constant ScriptId__Increment_Counter            = 0
constant ScriptId__Test_Set_Clear_Flags         = 1
constant ScriptId__Test_GameState_Words         = 2
constant ScriptId__Test_End_Script              = 3
constant ScriptId__Test_If_Statements           = 4
constant ScriptId__Test_Sleep_AnimationTicks    = 5
constant ScriptId__Test_Yield_Helpers           = 6
constant ScriptId__Test_While_Statements        = 7

constant LastScriptId = 7


constant N_TEST_YIELD_INSTRUCTIONS = Scripting.Bytecode._SetTestYieldInstructionsBitfield__counter

// Each bit in this variable represents a successful yield instruction test
// See `Scripting.Bytecode._SetTestYieldInstructionsBitfield__counter` macro.
// (byte bitfield)
allocateTmpWord(testYieldInstructionsBitfield)



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
Test.add("Test If Statements")
function TestIfStatements {
    jsr     _LoadScriptTestsRoom

    lda.w   #ScriptId__Test_If_Statements
    jsr     Scripting.ActivateScript
    jsr     Scripting.ProcessScripts

    lda.w   Project.GameState.Words.counter
    cmp.w   #15
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
Test.add("Test Sleep_AnimationTicks")
function TestSleepAnimationTicks {
constant _loopCounter = Test.dpTmp

    jsr     _LoadScriptTestsRoom

    lda.w   #ScriptId__Test_Sleep_AnimationTicks
    jsr     Scripting.ActivateScript


    lda.l   STAT77
    and.w   #STAT78.pal << 8
    beq     +
        // pal
        lda.w   #50
        bra     ++
    +
        // ntsc
        lda.w   #60
    +
    sta.b   _loopCounter


    jsr     Scripting.ProcessScripts

    // Wait 1 second
    Loop:
        lda.w   Project.GameState.Words.counter
        bne     Fail

        jsr     Scripting.ProcessScripts

        dec.b   _loopCounter
        bne     Loop


    lda.w   Project.GameState.Words.counter
    cmp.w   #1
    bne     Fail

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
Test.add("Test Yielding Helpers")
function TestYieldingHelpers {
constant _oldCounter = Test.dpTmp


    jsr     _LoadScriptTestsRoom

    lda.w   #ScriptId__Test_Yield_Helpers
    jsr     Scripting.ActivateScript

    lda.w   Project.GameState.Words.counter
    sta.b   _oldCounter


    stz.w   Project.GameState.Words.last

    stz.w   testYieldInstructionsBitfield


    // Start first yielding instruction
    jsr     Scripting.ProcessScripts

    // Each helper function will test to ensure the arguments are correct
    // and will wait for only 1 frame.

    Loop:
        jsr     Scripting.ProcessScripts

        // Check counter has incremented
        // (confirms each yield instruction waited one frame)
        lda.b   _oldCounter
        inc
        cmp.w   Project.GameState.Words.counter
        bne     Fail

        sta.b   _oldCounter

        // Loop until the last word has changed
        lda.w   Project.GameState.Words.last
        beq     Loop


    lda.w   Project.GameState.Words.counter
    cmp.w   #N_TEST_YIELD_INSTRUCTIONS
    bne     Fail

    // Test all yield instructions were tested successfully
    lda.w   testYieldInstructionsBitfield
    cmp.w   #(1 << N_TEST_YIELD_INSTRUCTIONS) - 1
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
Test.add("Test While Statements")
function TestWhileStatements {
    jsr     _LoadScriptTestsRoom

    lda.w   #ScriptId__Test_While_Statements
    jsr     Scripting.ActivateScript


    // The script compiler will generate a possible infinite loop warning
    // if the while loop does not contain a yielding statement.
    //
    // Thus we run the test until it is completed.
    Loop:
        jsr     Scripting.ProcessScripts

        lda.w   Project.GameState.Words.last
        beq     Loop


    lda.w   Project.GameState.Words.counter
    cmp.w   #24
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

    // Activate a script outside the N_ROOM_SCRIPTS range
    lda.w   #0xff
    jsr     Scripting.ActivateScript

    // Test activating an unused script does nothing
    assert(LastScriptId + 1 < Room.N_ROOM_SCRIPTS)
    lda.w   #LastScriptId + 1
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


a16()
i16()
code()
Test.add("Activate/Deactivate Scripts Test")
function ActivateDeactivateScriptsTest {

    // The `ActivateDeactivateScriptsTest` room contains `N_SCRIPTS` scripts that will
    // Sleep for a given number of frames and then increment the GameState counter.

    lda.w   #Project.RoomList.ActivateDeactivateScriptTest
    ldx.w   #Project.PlayerIds.Blank
    ldy.w   #0
    jsr     InteractiveTests.GameLoop.Init_LoadRoom


    lda.w   #5
    sta.w   Resources.animationSpeed

    lda.w   Project.GameState.Words.counter
    bne     Fail


    // Activate the first 8 scripts
    lda.w   #7
    FirstLoop:
        pha
        jsr     Scripting.ActivateScript
        pla
        dec
        bpl     FirstLoop

    // Process 3 frames of scripts
    // Four of the first eight scripts will increment the counter within 3 frames of execution.
    jsr     Scripting.ProcessScripts
    jsr     Scripting.ProcessScripts
    jsr     Scripting.ProcessScripts

    // Confirm 4 scripts completed
    lda.w   Project.GameState.Words.counter
    cmp.w   #4
    bne     Fail


    // Try to activate all scripts
    // This will only activate `N_SCRIPTS - 4` scripts, Four scripts are still active.
    // ActivateScript will not activate a script if it is already active.
    lda.w   #Room.N_ROOM_SCRIPTS
    SecondLoop:
        pha
        jsr     Scripting.ActivateScript
        pla
        dec
        bpl     SecondLoop

    // Process 6 frames of scripts
    // All scripts will be complete after 6 frames of execution.
    lda.w   #6
    ProcessLoop:
        pha
        jsr     Scripting.ProcessScripts
        pla
        dec
        bne     ProcessLoop

    // Confirm `N_SCRIPTS + 4` scripts completed
    lda.w   Project.GameState.Words.counter
    cmp.w   #Room.N_ROOM_SCRIPTS + 4
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
Test.add("Script BG Tasks")
function ScriptBgTasksTest {

constant N_SCRIPTS = 5
constant TASKS_PER_SCRIPT = 2

constant EXPECTED_SETUP_COUNTER = N_SCRIPTS * TASKS_PER_SCRIPT
constant EXPECTED_TASK_COUNTER  = Scripting.Bytecode.TestBgTask.TASK_DELAY * EXPECTED_SETUP_COUNTER


    // The `ScriptBgTasksTest` room contains `N_SCRIPTS` scripts that will
    // sleep and execute a `TestBgTask` scripting instruction `TASKS_PER_SCRIPT` times.


    lda.w   #Project.RoomList.ScriptBgTasksTest
    ldx.w   #Project.PlayerIds.Blank
    ldy.w   #0
    jsr     InteractiveTests.GameLoop.Init_LoadRoom


    stz.w   Scripting.Bytecode.TestBgTask.setupCounter
    stz.w   Scripting.Bytecode.TestBgTask.taskCounter


    // Activate the scripts
    lda.w   #N_SCRIPTS - 1
    ActivateLoop:
        pha
        jsr     Scripting.ActivateScript
        pla
        dec
        bpl     ActivateLoop


    // Run the scripts until they have all ended
    Loop:
        jsr     Scripting.ProcessScripts

        lda.w   Scripting.activeScriptsEnd
        bne     Loop


    // Confirm setupCounter is correct
    lda.w   Scripting.Bytecode.TestBgTask.setupCounter
    cmp.w   #EXPECTED_SETUP_COUNTER
    bne     Fail

    // Confirm taskCounter is correct
    lda.w   Scripting.Bytecode.TestBgTask.taskCounter
    cmp.w   #EXPECTED_TASK_COUNTER
    bne     Fail


    sec
    rts


Fail:
    clc
    rts
}


}

// vim: ft=bass-65816 ts=4 sw=4 et:

