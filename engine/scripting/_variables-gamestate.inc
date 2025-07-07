// scripting/_variables-gamestate.inc
// ==================================
//
// Variables used by the GameState subsystem.
//
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


constant GameStateData.size = N_GAME_STATE_PAGES * 256

allocate(GameStateData, wram7e, GameStateData.size)
assert(GameStateData & 0xff == 0)


// Game state variables.
//
// Projects are allowed to add extra fields to the GameState.
namespace GameState {
    struct(GameStateData)
    struct_maxsize(GameStateData.size)

    if Scripting.GAME_STATE_FORMAT_VERSION != 1 {
        error "Invalid untech-compiler Game State Format"
    }

    constant wordData.size = Project.GameState.MAX_WORDS * 2
    constant flagData.size = Project.GameState.MAX_FLAGS / 8


    // Data storage for the GameState word variables
    // (Project.GameState.MAX_WORDS * uint16)
    field(wordData, wordData.size)

    assertOnePage(wordData, wordData.size)
    assert(wordData.size / 2 > Project.GameState.N_WORDS)


    // Current room id.
    // (uint8)
    field(roomId, 1)

    // Entrance id.
    // The entrance to start the room in
    // (byte)
    field(entranceId, 1)

    // The player's data id.
    // Used to determine which player to spawn when loading the game.
    // (byte)
    field(playerDataId, 1)

constant _EndOfInitialState = __STRUCT__.pos


    // Data storage for the GameState flag variables
    // (Project.GameState.MAX_FLAGS bits)
    field(flagData, flagData.size)

    assert(flagData.size * 8 > Project.GameState.N_FLAGS)
    assert(Project.GameState.MAX_FLAGS % 8 == 0)
    assertOnePage(flagData, flagData.size)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

