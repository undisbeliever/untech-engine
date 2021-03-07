// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Scripting {
namespace Bytecode {



// Jumps to the unit test fail screen
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// RETURN: Test.Fail
a8()
i16()
code()
constant Fail = Test.Fail



// Jump to unit test fail screen if the flag is not set
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode or Test.Fail
macro Assert_Flag_Set() {
    TestGameStateFlag_BranchIfSet(FlagSet)
        jmp     Test.Fail

FlagSet:
    jmp     GotoNextOpcode
}
Flag_Instruction(Assert_Flag_Set)



// Jump to unit test fail screen if the flag is not set
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode or Test.Fail
macro Assert_Flag_Clear() {
    TestGameStateFlag_BranchIfClear(FlagClear)
        jmp     Test.Fail

FlagClear:
    jmp     GotoNextOpcode
}
Flag_Instruction(Assert_Flag_Clear)



// Asserts a GameState word is equal to a immediate value
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode or Test.Fail
a8()
i16()
code()
function Assert_Word_Equals___Word_ImmU16 {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    rep     #$30
a16()

    // second argument: immediate u16 word
    lda.w   scriptData,y
    iny
    iny

    cmp.w   GameState.wordData,x
    beq     +
        jmp     Test.Fail
    +

    sep     #$20
a8()

    jmp     GotoNextOpcode
}



// Yielding instruction helper tests
// ---------------------------------

// Set the next available bit in `ScriptingTests.testYieldInstructionsBitfield`.
// Also increments `_SetTestYieldInstructionsBitfield__counter`
//
// Used to confirm the resume function has called and the test passed.
macro _SetTestYieldInstructionsBitfield() {
    assert8a()

    lda.b   #1 << _SetTestYieldInstructionsBitfield__counter
    tsb.w   ScriptingTests.testYieldInstructionsBitfield

    _SetTestYieldInstructionsBitfield__counter = _SetTestYieldInstructionsBitfield__counter + 1
}
variable _SetTestYieldInstructionsBitfield__counter = 0



// Tests `YieldInstruction__NoArg`
constant TestYield_NoArg = YieldInstruction__NoArg

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_GotoNextOpcode_Y` or `Test.Fail`
a8()
i16()
code()
function TestYield_NoArg___Resume {
    ldx.w   scripts.resumeArg1,y
    bne     Wrong

    ldx.w   scripts.resumeArg2,y
    bne     Wrong

    // Corrupt Y, test `ResumeInstruction_GotoNextOpcode`
    dey
    dey

    _SetTestYieldInstructionsBitfield()

    jmp     ResumeInstruction_GotoNextOpcode

Wrong:
    jmp     Test.Fail
}



// Tests `YieldInstruction__2ByteArg`
// The test passes if the argument is `42`
constant TestYield_2ByteArg_42___ImmU16 = YieldInstruction__2ByteArg

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_GotoNextOpcode_Y` or `Test.Fail`
a8()
i16()
code()
function TestYield_2ByteArg_42___ImmU16___Resume {
    ldx.w   scripts.resumeArg1,y
    cpx.w   #42
    bne     Wrong

    ldx.w   scripts.resumeArg2,y
    bne     Wrong

    _SetTestYieldInstructionsBitfield()

    jmp     ResumeInstruction_GotoNextOpcode_Y

Wrong:
    jmp     Test.Fail
}



// Tests `YieldInstruction__4ByteArg`
// The test passes if the arguments are `1024` and `8192`.
constant TestYield_4ByteArg_1024_8192___ImmU16_ImmU16 = YieldInstruction__4ByteArg

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_GotoNextOpcode_Y` or `Test.Fail`
a8()
i16()
code()
function TestYield_4ByteArg_1024_8192___ImmU16_ImmU16___Resume {
    ldx.w   scripts.resumeArg1,y
    cpx.w   #1024
    bne     Wrong

    ldx.w   scripts.resumeArg2,y
    cpx.w   #8192
    bne     Wrong

    _SetTestYieldInstructionsBitfield()

    jmp     ResumeInstruction_GotoNextOpcode_Y

Wrong:
    jmp     Test.Fail
}




// Tests `Yield__Aopcode`
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// OUTPUT: Y - program counter
// RETURN: Jump to `Yield__Aopcode`
a8()
i16()
code()
function TestYield_Aopcode {
    sty.b   programCounter

    // corrupt Y PC
    dey

    lda.b   #Project.BytecodeOpcodes.Yielding.TestYield_Aopcode
    jmp     Yield__Aopcode

    // Also tests `Yield__Ypc_Aopcode`
    assert(Yield__Ypc_Aopcode == Yield__Aopcode + 2)
}

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_GotoNextOpcode_Y` or `Test.Fail`
a8()
i16()
code()
function TestYield_Aopcode___Resume {
    ldx.w   scripts.resumeArg1,y
    bne     Wrong

    ldx.w   scripts.resumeArg2,y
    bne     Wrong

    // Corrupt Y, test `ResumeInstruction_GotoNextOpcode`
    dey
    dey

    _SetTestYieldInstructionsBitfield()

    jmp     ResumeInstruction_GotoNextOpcode

Wrong:
    jmp     Test.Fail
}



// Tests `Yield__Xarg_Ypc_Aopcode`.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to `Yield__Xarg_Ypc_Aopcode`
a8()
i16()
code()
function TestYield_Xarg_Ypc_Aopcode {
    ldx.w   #0x4242
    lda.b   #Project.BytecodeOpcodes.Yielding.TestYield_Xarg_Ypc_Aopcode

    jmp     Yield__Xarg_Ypc_Aopcode

    // Also tests Yield__Xarg_Aopcode
    assert(Yield__Xarg_Aopcode == Yield__Xarg_Ypc_Aopcode + 2)
}

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_GotoNextOpcode_Y` or `Test.Fail`
a8()
i16()
code()
function TestYield_Xarg_Ypc_Aopcode___Resume {
    ldx.w   scripts.resumeArg1,y
    cpx.w   #0x4242
    bne     Wrong

    ldx.w   scripts.resumeArg2,y
    bne     Wrong

    _SetTestYieldInstructionsBitfield()

    jmp     ResumeInstruction_GotoNextOpcode_Y

Wrong:
    jmp     Test.Fail
}



// Tests `TestYield_XYarg_Aopcode`
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// OUTPUT: Y - program counter
// RETURN: Jump to `Yield__XYargs_Aopcode`
a8()
i16()
code()
function TestYield_XYarg_Aopcode {
    sty.b   programCounter

    ldx.w   #0x1234
    ldy.w   #0x5678
    lda.b   #Project.BytecodeOpcodes.Yielding.TestYield_XYarg_Aopcode

    jmp     Yield__XYarg_Aopcode
}

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_GotoNextOpcode_Y` or `Test.Fail`
a8()
i16()
code()
function TestYield_XYarg_Aopcode___Resume {
    ldx.w   scripts.resumeArg1,y
    cpx.w   #0x1234
    bne     Wrong

    ldx.w   scripts.resumeArg2,y
    cpx.w   #0x5678
    bne     Wrong

    _SetTestYieldInstructionsBitfield()

    jmp     ResumeInstruction_GotoNextOpcode_Y

Wrong:
    jmp     Test.Fail
}



// Background Task Instructions
// ============================


CreateBgTaskInstruction(TestBgTask, TestBgTask, ImmU16)
namespace TestBgTask {

assert(BackgroundTasks.state.size >= 4)
constant _var   = BackgroundTasks.state + 0
constant _timer = BackgroundTasks.state + 2


// Incremented every time `SetupBgTask___ImmU16` is called
allocateTmpWord(setupCounter)

// Incremented every time `ProcessBgTask` is called
allocateTmpWord(taskCounter)

// Number of frames to wait for each task
// (ensures the BG tasks are constantly waiting in ScriptingTests)
constant TASK_DELAY = 7


// BG Task setup routine
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// OUTPUT: Y - program counter
a8()
i16()
code()
function SetupBgTask___ImmU16 {
    // Argument 1 = immediate u16

    rep     #$30
a16()

    lda.w   scriptData,y
    iny
    iny

    sta.w   _var
    stz.w   _timer

    inc.w   setupCounter

    sep     #$20
a8()
    rts
}



// BG Task process routine
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e
// OUTPUT: carry set if the task has completed
a8()
i16()
code()
function ProcessBgTask {

    // Confirm argument to `TestBgTask` is correct
    ldx.w   _var
    cpx.w   #42
    bne     Wrong


    // Increment taskCounter
    ldx.w   taskCounter
    inx
    stx.w   taskCounter


    // Increment timer, test if the task is complete
    lda.w   _timer
    inc
    sta.w   _timer

    cmp.b   #TASK_DELAY
    // carry set (end task) if _timer >= TASK_DELAY

    rts


Wrong:
    jmp     Test.Fail
}

}



CreateBgTasksFunctionTables()

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

