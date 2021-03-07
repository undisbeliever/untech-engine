// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace GameStateTests {

constant _wordBefore = Test.dpTmp
constant _wordAfter  = Test.dpTmp + 2


// Fills GameStateData with 0xff (and the words before/after GameStateData with 0x77)
//
// Also: Saves a backup of the words before/after GameStateData
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function _FillGameState {
    assert(GameStateData - 2 > STACK_TOP)
    assert(GameStateData - 2 > STACK_BOTTOM)


    lda.w   GameStateData - 2
    sta.b   _wordBefore

    lda.w   GameStateData + GameStateData.size
    sta.b   _wordAfter


    lda.w   #0x7777
    sta.w   GameStateData - 2
    sta.w   GameStateData + GameStateData.size


    lda.w   #0xffff
    ldx.w   #GameStateData.size - 2
    Loop:
        sta.w   GameStateData,x
        dex
        dex
        bpl     Loop

    rts
}



// Tests and restores the bytes before and after GameStateData
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// OUTPUT: carry set
a16()
i16()
code()
function _EndTest {
    // Confirm words before/after GameStateData have not changed
    lda.w   GameStateData - 2
    cmp.w   #0x7777
    bne     Fail

    lda.w   GameStateData + GameStateData.size
    cmp.w   #0x7777
    bne     Fail


    // Restore words before/after GameStateData
    lda.b   _wordBefore
    sta.w   GameStateData - 2

    lda.b   _wordAfter
    sta.w   GameStateData + GameStateData.size

    sec
    rts


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("LoadInitialGameState")
function LoadInitialGameState {

    jsr     _FillGameState

    jsr     GameState.LoadInitialGameState


    // Confirm initial game state
    lda.w   Project.GameState.Words.first
    cmp.w   #1
    bne     Fail

    lda.w   Project.GameState.Words.second
    cmp.w   #2
    bne     Fail

    lda.w   Project.GameState.Words.sixtyth
    cmp.w   #60
    bne     Fail

    // Confirm last and final words is 0
    lda.w   Project.GameState.Words.last
    bne     Fail

    lda.w   GameState.wordData + GameState.wordData.size - 2
    bne     Fail


    // Confirm flag data is 0
    lda.w   GameState.flagData
    bne     Fail

    lda.w   GameState.flagData + 2
    bne     Fail

    lda.w   GameState.flagData + Project.GameState.MAX_FLAGS / 8 - 1
    bne     Fail


    // Confirm initial room settings
    assert(GameState.roomId + 1 == GameState.entranceId)
    lda.w   GameState.roomId
    cmp.w   #Project.RoomList.EmptyRoom | (0 << 8)
    bne     Fail

    assert(GameState.playerDataId - 1 == GameState.entranceId)
    assert(Project.PlayerIds.Blank != 0)
    lda.w   GameState.playerDataId - 1
    cmp.w   #0 | (Project.PlayerIds.Blank << 8)
    bne     Fail


    jmp     _EndTest


Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("ClearRoomState")
function ClearRoomStateTest {

    jsr     _FillGameState


    sep     #$10
i8()
    GameState._ClearRoomState()

    rep     #$10
i16()

    lda.w   #0xffff

    // Confirm room is unchanged
    assert(GameState.roomId + 1 == GameState.entranceId)
    cmp.w   GameState.roomId
    bne     Fail

    assert(GameState.playerDataId + 1 == GameState.flagData)
    assert(Project.GameState.N_FLAGS > 8)
    cmp.w   GameState.playerDataId
    bne     Fail

    // Confirm data after wordData is unchanged
    // A = 0xffff
    cmp.w   GameState.wordData + GameState.wordData.size
    bne     Fail

    // Confirm data after flagData is unchanged
    // A = 0xffff
    cmp.w   GameState.flagData + GameState.flagData.size
    bne     Fail

    // Confirm last bytes in GameStateData is unchanged
    // A = 0xffff
    cmp.w   GameStateData + GameStateData.size - 2
    bne     Fail

    // Confirm global words is unchanged
    // A = 0xffff
    cmp.w   Project.GameState.Words.first
    bne     Fail

    cmp.w   Project.GameState.Words.sixtyth
    bne     Fail

    cmp.w   Project.GameState.Words.last
    bne     Fail

    // Confirm global flag data is unchanged
    // A = 0xffff
    cmp.w   GameState.flagData
    bne     Fail

    assert(Project.GameState.N_FLAGS > 4 * 8)
    cmp.w   GameState.flagData + 2
    bne     Fail

    assert(Project.GameState.N_FLAGS > 7 * 8)
    cmp.w   GameState.flagData + 5
    bne     Fail


    // Confirm flags after the last global flag is 0
    assert(Project.GameState.N_FLAGS % 8 != 0)
    lda.w   GameState.flagData + (Project.GameState.N_FLAGS / 8)
    cmp.w   #0xff >> (8 - (Project.GameState.N_FLAGS % 8))
    bne     Fail

    // Confirm word after last global flag is zero
    lda.w   GameState.flagData + (Project.GameState.N_FLAGS / 8) + 1
    bne     Fail

    // Confirm last flag is zero
    assert(Project.GameState.N_FLAGS < Project.GameState.MAX_FLAGS - 17)
    lda.w   GameState.flagData + GameState.flagData.size - 2
    bne     Fail


    // Confirm word after last global word is zero
    lda.w   Project.GameState.Words.last + 2
    bne     Fail

    // Confirm last global word is zero
    lda.w   GameState.wordData + GameState.wordData.size - 2
    bne     Fail


    jmp     _EndTest


Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

