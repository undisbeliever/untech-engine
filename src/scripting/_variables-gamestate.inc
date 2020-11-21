// scripting/_variables-gamestate.inc
// ==================================
//
// Variables used by the GameState subsystem.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

