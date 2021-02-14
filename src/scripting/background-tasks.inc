// scripting/background-tasks.inc
// ==============================
//
// Scripting background tasks are scripting instructions that require multiple
// display frames to execute.  The script will spawn a task, which will be
// called once a frame by the main loop until the task is complete.
//
// To prevent resource contention, only one task can be executed at a time.  If
// two scripts wish to start a background task, one must wait for the other to
// finish.
//
// Examples of possible background tasks include:
//   * Spawning entities
//   * Changing room tiles (ie, opening pits, closing doors)
//   * Text boxes
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Scripting {


// Background Task Registration
// ============================


// index 0 is a null (do nothing) task
variable __BG_TASKS_COUNTER__ = 1

variable __BG_TASKS_COMPLETE__ = 0


// Create a scripting bytecode instruction for a Background Task.
//
// This macro MUST BE called in the `Scripting.Bytecodes` namespace.
//
// PARAM: name - name of the Background Task instruction
inline CreateBgTaskInstruction(name) {
    if __BG_TASKS_COMPLETE__ == 1 {
        error "Cannot add a background task after `BackgroundTaskFunctions()` call"
    }


    a8()
    i16()
    function {name} {
        ldx.w   #__BG_TASKS_COUNTER__ * 2
        jmp     BackgroundTasks.__BgTaskInstruction
    }


    evaluate _n = __BG_TASKS_COUNTER__
    __BG_TASKS_COUNTER__ = __BG_TASKS_COUNTER__ + 1

    constant _BgTasksSetupFunctionTable.__{_n} = {name}.SetupBgTask
    constant _BgTasksFunctionTable.TaskFunctionTable.__{_n} = {name}.ProcessBgTask
}


// Create a script bytecode instruction for a Background Task.
//
// This macro MUST BE called in the `Scripting.Bytecodes` namespace.
//
// PARAM: name - name of the Background Task instruction
// PARAM: arguments - a "_" separated list of argument types used by the instruction
inline CreateBgTaskInstruction(name, arguments) {
    if __BG_TASKS_COMPLETE__ == 1 {
        error "Cannot add a background task after `BackgroundTaskFunctions()` call"
    }

    a8()
    i16()
    function {name}___{arguments} {
        ldx.w   #__BG_TASKS_COUNTER__ * 2
        jmp     BackgroundTasks._BgTaskInstruction
    }


    evaluate _n = __BG_TASKS_COUNTER__
    __BG_TASKS_COUNTER__ = __BG_TASKS_COUNTER__ + 1

    constant _BgTasksSetupFunctionTable.__{_n} = {name}.SetupBgTask___{arguments}
    constant _BgTasksFunctionTable.__{_n} = {name}.ProcessBgTask
}



// Create the `SetupFunctionTable` and `TaskFunctionTable` function tables.
//
// This macro MUST BE called in the `Scripting.Bytecode` namespace.
inline CreateBgTasksFunctionTables() {
    if __BG_TASKS_COMPLETE__ == 1 {
        error "Cannot add a background task after `BackgroundTaskFunctions()` call"
    }

    constant N_BACKGROUND_TASK_FUNCTIONS = __BG_TASKS_COUNTER__
    assert(N_BACKGROUND_TASK_FUNCTIONS * 2 < 0xff)

    code()
    _BgTasksSetupFunctionTable:
        dw  Scripting.BackgroundTasks.NullBgTask

        evaluate _n = 1
        while {_n} < N_BACKGROUND_TASK_FUNCTIONS {
            dw  _BgTasksSetupFunctionTable.__{_n}

            evaluate _n = {_n} + 1
        }


    code()
    _BgTasksFunctionTable:
        dw  Scripting.BackgroundTasks.NullBgTask

        evaluate _n = 1
        while {_n} < N_BACKGROUND_TASK_FUNCTIONS {
            dw  _BgTasksFunctionTable.__{_n}

            evaluate _n = {_n} + 1
        }

    assert(pc() - _BgTasksSetupFunctionTable == N_BACKGROUND_TASK_FUNCTIONS * 2 * 2)

    __BG_TASKS_COMPLETE__ = 1
}



namespace BackgroundTasks {


// Process the new background task.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _ProcessBackgroundTask() {
    assert8a()
    assert16i()

    ldx.w   BackgroundTasks.backgroundTask
    jsr     (BackgroundTasks.TaskFunctionTable,x)

    bcc     TaskStillRunning

        // Stop executing the background task
        stz.w   BackgroundTasks.backgroundTask

        // Mark task complete
        lda.b   #BackgroundTasks.taskStatus.TASK_COMPLETED_FLAG
        sta.w   BackgroundTasks.taskStatus

TaskStillRunning:
}



// Null Background Task.
//
// This is the first routine in `TaskFunctionTable` and it will be executed
// whenever no BG Task is running.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0, DB = 0x7e
//
// OUTPUT: carry clear
a8()
i16()
code()
function NullBgTask {
    // Never mark this task as complete
    clc
    rts
}



// Starting / Waiting for a background task
// ========================================


// Start a new background task.
//
// ASSUMES background task is not active.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// REQUIRES: taskStatus == 0
// INPUT: X = index into
// INPUT: Y = script program counter
// OUTPUT: Y = script program counter after arguments
macro _StartNewBgTask() {
    assert8a()
    assert16i()

    stx.w   backgroundTask


    // Set Ah to 0
    tdc

    // IN: Y = script program counter
    jsr     (SetupFunctionTable,x)
    // OUT: Y = script program counter after arguments

    // Mark task as running
    lda.b   #taskStatus.TASK_RUNNING_FLAG
    sta.w   taskStatus
}



// Processes a Background Task scripting instruction
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0, Ah = 0
//
// INPUT: X = BG Task function table index
// INPUT: Y = program counter
//
// RETURN: Jump to `Yield__Ypc_Aopcode` or `Yield__Xarg_Ypc_Aopcode`
a8()
i16()
code()
function _BgTaskInstruction {

    assert(taskStatus.READY_TO_START_A_NEW_TASK == 0)
    lda.w   taskStatus
    bne     TaskActive
        // Ready to start a new task

        _StartNewBgTask()
        // Y = programCounter after arguments

        lda.b   #SpecialResumeOpcodes.WaitUntilBgTaskEnd
        jmp     Yield__Ypc_Aopcode


    TaskActive:
        // BG Task active - Yield until there is no BG Task active
        lda.b   #SpecialResumeOpcodes.StartBgTask
        // X = function Table Index
        jmp     Yield__Xarg_Ypc_Aopcode
}



// Scripting Resume Functions
// ==========================


// Scripting resume function that waits until a Background Task can be started.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: Y - currentScriptIndex
// INPUT: resumeArg1 - BG Task function table index
// RETURN: Jump to `ResumeInstruction_Sleep_Y`
a8()
i16()
code()
function StartBgTask___Resume {

    assert(taskStatus.READY_TO_START_A_NEW_TASK == 0)
    lda.w   taskStatus
    bne     ResumeInstruction_Sleep_Y
        // Ready to start a new task

        // set Ah to 0 (needed for tax below)
        tdc

        // index into function table
        lda.w   scripts.resumeArg1,y


        // programCounter is at the byte after the opcode byte.
        // It may be in the middle of an instruction
        ldx.w   scripts.programCounter,y
        txy
        tax

        // X = function table Index
        // Y = programCounter

        _StartNewBgTask()

        // Y = programCounter after arguments

        ldx.w   currentScriptIndex

        // Change resume function
        lda.b   #SpecialResumeOpcodes.WaitUntilBgTaskEnd
        sta.w   scripts.resumeOpcode,x

        // Update programCounter
        rep     #$30
    a16()
        tya
        sta.w   scripts.programCounter,x

        // ResumeInstruction_Sleep_Y can be called with any register size
        txy
        bra     ResumeInstruction_Sleep_Y
}



// Scripting resume function that waits until a background task has completed.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_Sleep_Y` or `ResumeInstruction_GotoNextOpcode_Y`
a8()
i16()
code()
function WaitUntilBgTaskEnd___Resume {

    // Sleep if the background task is not complete
    // (This is safe as there is only one script with a `WaitForBgTaskEnd` resume function at any given time)

    assert(taskStatus.TASK_COMPLETED_FLAG == 0x80)
    lda.w   taskStatus
    bpl     ResumeInstruction_Sleep_Y
        // Task completed

        // allow the next BG task to start
        assert(taskStatus.READY_TO_START_A_NEW_TASK == 0)
        stz.w   taskStatus

        jmp     ResumeInstruction_GotoNextOpcode_Y
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

