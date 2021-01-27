// scripting/_dataformat.inc
// =========================
//
// Data format of the scripting subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

}

// vim: ft=bass-65816 ts=4 sw=4 et:

