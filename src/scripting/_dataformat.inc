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
// Instead this routine MUST JUMP to `GotoNextOpcode`, `GotoNextOpcode_LoadPC` or `End_Script`
// (with each label requiring different arguments (see the RETURN tags below).
//
// NOTE: This function will be called with the high byte of A clear (0).
//       This allows for a `tax` and `tay` instruction be be safely used with
//       an 8 bit A and 16 bit Index.
//
// REGS: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0, Ah = 0
// INPUT: Y - program counter after the bytecode opcode byte.
// INPUT: currentScriptIndex - the index (2 * scriptId) of the current script.
// RETURN: `Scripting.Bytecode.GotoNextOpcode`
//             (OUT: Y = program counter after this instruction)
// RETURN: `Scripting.Bytecode.GotoNextOpcode_LoadPC`
//             (OUT: programCounter = program counter after this instruction)
// RETURN: `Scripting.Bytecode.End_Script`
//          (no arguments)
constant BytecodeFunctionTable = Project.BytecodeFunctionTable

}

// vim: ft=bass-65816 ts=4 sw=4 et:

