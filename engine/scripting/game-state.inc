// scripting/game-state.inc
// ========================
//
// Game State subsystem
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


namespace GameState {


// Load the initial game state into memory.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function LoadInitialGameState {

constant INITIAL_STATE_SIZE = GameState._EndOfInitialState - GameStateData

    ldx.w   #Project.InitialGameState
    lda.w   #Project.InitialGameState >> 16
    stx.b   LZ4.ptr
    sta.b   LZ4.ptr + 2

    ldx.w   #GameStateData
    lda.w   #GameStateData >> 16
    ldy.w   #GameStateData

    jsr     LZ4.Decompress

    // Ensure all data was written
    cpy.w   #GameState._EndOfInitialState
    beq     +
        break(LZ4_FAILED)
    +


    // Fill remaining data with zeros

    stz.w   GameState._EndOfInitialState
    tyx
    iny
    lda.w   #GameStateData.size - INITIAL_STATE_SIZE - 2
    mvn     0x7e=0x7e

    rts
}



// Resets all unnamed words and flags to 0
//
// Called whenever a room is loaded.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _ClearRoomState() {
    assert16a()
    assert8i()

    constant WORDS_TO_CLEAR = Project.GameState.MAX_WORDS - Project.GameState.N_WORDS

    if WORDS_TO_CLEAR > 0 {
        assert(WORDS_TO_CLEAR * 2 < 0xff)

        ldx.b   #WORDS_TO_CLEAR * 2
        WordLoop:
            stz.w   GameState.wordData + (Project.GameState.N_WORDS - 1) * 2,x
            dex
            dex
            bne     WordLoop
    }

    constant FLAGS_TO_CLEAR = Project.GameState.MAX_FLAGS - Project.GameState.N_FLAGS

    if FLAGS_TO_CLEAR > 0 {
        constant OFFSET = (Project.GameState.N_FLAGS / 16) * 2

        if OFFSET < GameState.flagData.size {
            assert(GameState.flagData.size - OFFSET < 0xff)

            lda.w   #0xffff << (Project.GameState.N_FLAGS % 16)
            trb.w   GameState.flagData + OFFSET

            if OFFSET > 2 {
                ldx.b   #GameState.flagData.size - OFFSET - 2
                FlagLoop:
                    stz.w   GameState.flagData + OFFSET,x
                    dex
                    dex
                    bne     FlagLoop
            }
        }
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

