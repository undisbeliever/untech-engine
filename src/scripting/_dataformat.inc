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

}

// vim: ft=bass-65816 ts=4 sw=4 et:

