// scripting/_dataformat.inc
// =========================
//
// Data format of the scripting subsystem.
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Scripting {

namespace Format {
    namespace GameState {
        // The initial game state.
        //
        // An LZ4 compressed data block containing:
        //   * `GameState.wordData`
        //   * `GameState.roomId`
        //   * `GameState.entranceId`
        //   * `GameState.playerId`
        //
        // NOTE: All other bytes/data in GameState will be set to 0 on initialisation
        constant InitialGameState = Project.InitialGameState
    }
}



// Bytecode instruction function table.
//
// This routine MUST NOT invoke RTS.
// Instead this routine MUST JUMP to `GotoNextOpcode`, `GotoNextOpcode_LoadPC`, `End_Script`,
// `Yield__*`, or `YieldInstruction__*` routines.
// (with each label requiring different arguments (see the RETURN tags below).
//
// NOTE: This function will be called with the high byte of A clear (0).
//       This allows for a `tax` and `tay` instruction be be safely used with
//       an 8 bit A and 16 bit Index.
//
// REGS: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0, Ah = 0
// INPUT: X - index into `BytecodeFunctionTable` and `BytecodeResumeFunctionTable` (resume instruction opcode)
// INPUT: Y - program counter after the bytecode opcode byte.
// INPUT: currentScriptIndex - index into the `scripts` SoA (MUST NOT CHANGE).
//
// RETURN: `Scripting.Bytecode.GotoNextOpcode`
//              (OUT: Y = program counter after this instruction)
// RETURN: `Scripting.Bytecode.GotoNextOpcode_LoadPC`
//              (OUT: programCounter = program counter after this instruction)
// RETURN: `Scripting.Bytecode.End_Script`
//              (no arguments)
// RETURN: `Scripting.YieldInstruction__*`
//              (OUT: X = bytecode instruction opcode, Y = program counter at instruction arguments)
//              (See `Sleep_AnimationTicks___ImmU16` for an example of using `YieldInstruction__`* return)
// RETURN: `Scripting.Yield__*`
//              (See the routine documentation for outputs)
constant BytecodeFunctionTable = Project.BytecodeFunctionTable



// Bytecode resume instruction function table.
//
// The resume function will be called by `ProcessScripts`.  It is expected to
// either resume script execution (by jumping to `ResumeInstruction_GotoNextOpcode*`,
// or continue sleeping (by jumping to `ResumeInstruction_Sleep`).
//
// Two words of storage are provided to hold state, `scripts.resumeArg1` and
// `scripts.resumeArg2`. These variables are initialized in the bytecode yielding
// routines (`Scripting.YieldInstruction__*` and `Yield__*`).
//
// The function order within this table will match `BytecodeFunctionTable`.
// (This greatly simplifies the Yield function helpers in scripts.inc)
//
// This routine MUST NOT invoke RTS.
// Instead this routine MUST JUMP to `ResumeInstruction_Sleep`, `ResumeInstruction_Sleep_Y`,
// `ResumeInstruction_GotoNextOpcode`, or `ResumeInstruction_GotoNextOpcode_Y`.
//
// REGS: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: currentScriptIndex - index into the `scripts` SoA (MUST NOT CHANGE).
// INPUT: Y - currentScriptIndex
//
// RETURN: `ResumeInstruction_Sleep`
//              continue yielding
// RETURN: `ResumeInstruction_Sleep_Y`
//              continue yielding
//              (OUT: Y - currentScriptIndex (MUST BE UNCHANGED))
//
// RETURN: `ResumeInstruction_GotoNextOpcode`
//              yielding is over - resume script
// RETURN: `ResumeInstruction_GotoNextOpcode_Y`
//              yielding is over - resume script
//              (OUT: Y - currentScriptIndex (MUST BE UNCHANGED))
constant BytecodeResumeFunctionTable = Project.BytecodeResumeFunctionTable



// Background Task's setup routine.
//
// This routine is called when a Background Task scripting instruction is
// executed and the system is ready to start a new Background Task.
//
// The setup routine should initialize any `Scripting.BackgroundTasks.state`
// variables used by the BG task's `TaskFunctionTable` function.
//
// A background task MUST not modify the `scripts.*` variables.
//
// This function table is created by the `CreateBgTasksFunctionTables` macro.
//
// REGS: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0, Ah = 0
//
// INPUT: Y - program counter after the bytecode opcode byte.
// OUTPUT: Y - script program counter after bytecode instruction arguments
constant BackgroundTasks.SetupFunctionTable = Bytecode._BgTasksSetupFunctionTable



// Background Task's process routine.
//
// The task function will be called once per frame before any scripts are
// processed.
//
// Any state used by the Background Task should be stored in the
// `Scripting.BackgroundTasks.state` variable.
//
// A background task MUST not modify the `scripts.*` variables.
//
// This function table is created by the `CreateBgTasksFunctionTables` macro.
//
// REGS: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// OUTPUT: carry set - The BG Task has ended.
//         carry clear - The BG Task is incomplete.
constant BackgroundTasks.TaskFunctionTable = Bytecode._BgTasksFunctionTable


}

// vim: ft=bass-65816 ts=4 sw=4 et:

