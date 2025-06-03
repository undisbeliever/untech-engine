// audio/control.inc
// =================
//
// Terrific Audio Driver control and query subroutines
//
//
// This file is is an untech port from the Terrific Audio Driver ca65 API.
//
// SPDX-FileCopyrightText: © 2024 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2024 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.  In
// no event will the authors be held liable for any damages arising from the use of
// this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//         claim that you wrote the original software. If you use this software in
//         a product, an acknowledgment in the product documentation would be
//         appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not be
//         misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//


namespace Audio {

// Adds a command to the queue if the queue is empty.
//
// The command queue can only hold 1 command.
// Returns true if the command was added to the queue.
//
// MUST NOT be used to send a play-sound-effect command.
//
// IN: A = `TadCommand` value
// IN: X = First command parameter (if required). Only the lower 8 bits will be sent to the Audio Driver.
// IN: Y = Second command parameter (if required). Only the lower 8 bits will be sent to the Audio Driver.
//
// OUT: Carry set if the command was added to the queue
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function QueueCommand {
    bit.w   TadPrivate_nextCommand_id
    bpl     ReturnFalse
        // command queue is empty
    WriteCommand:
        and.b   #TadIO_ToDriver.COMMAND_MASK
        sta.w   TadPrivate_nextCommand_id

        txa
        sta.w   TadPrivate_nextCommand_parameter0

        tya
        sta.w   TadPrivate_nextCommand_parameter1

        // return true
        sec
        rts

ReturnFalse:
    clc
    rts
}


// Adds a command to the queue, overriding any previously unsent commands.
//
// MUST NOT be used to send a play-sound-effect command.
// The command queue can only hold 1 command.
//
// IN: A = `TadCommand` value
// IN: X = First command parameter (if required). Only the lower 8 bits will be sent to the Audio Driver.
// IN: Y = Second command parameter (if required). Only the lower 8 bits will be sent to the Audio Driver.
//
// A8
// I unknown
// DB access lowram
a8()
iu()
constant QueueCommandOverride = QueueCommand.WriteCommand



// Queue the next sound effect to play, with panning.
//
// NOTE: Only 1 sound effect can be played at a time
// NOTE: Lower sound effect IDs take priority over higher sound effect IDs.
//
// IN: A = sfx id (as determined by the sound effect export order in the project file)
// IN: X = pan (only the lower 8 bits are used.  If `pan > TAD_MAX_PAN`, the sound effect will use center pan)
//
// A8
// I unknown
// DB access lowram
// KEEP: Y, X
a8()
iu()
code()
function QueuePannedSoundEffect {
    cmp.b   sfxQueue_sfx
    bcs     EndIf
        sta.b   sfxQueue_sfx

        txa
        sta.b   sfxQueue_pan

EndIf:
    rts
}



// Queue the next sound effect to play with center pan (TAD_MAX_PAN/2).
//
// NOTE: Only 1 sound effect can be played at a time.
// NOTE: Lower sound effect IDs take priority over higher sound effect IDs.
//
// IN: A = sfx id (as determined by the sound effect export order in the project file)
//
// A8
// I unknown
// DB access lowram
// KEEP: Y, X
a8()
iu()
code()
function QueueSoundEffect {
    cmp.b   sfxQueue_sfx
    bcs     EndIf
        sta.b   sfxQueue_sfx

        lda.b   #TAD_CENTER_PAN
        sta.b   sfxQueue_pan
EndIf:
    rts
}



// Disables the audio driver, starts the loader and queues a song or common-audio-data transfer.
//
// This subroutine will clear the `TadFlags::RELOAD_COMMON_AUDIO_DATA` flag.
//
// CAUTION: `LoadSong` will switch the state to `WAITING_FOR_LOADER_*`.  `LoadAudioData` will
// not be called until `Process` is called **and** the audio-driver has switched to the loader.
// While the state remains `WAITING_FOR_LOADER_*`, no audio data will be transferred and calling
// `FinishLoadingData` will not transfer any audio data.
//
// CAUTION: The audio driver starts in the paused state if the `TadFlags::PLAY_SONG_IMMEDIATELY`
// flag is clear when `Process` receives the ready signal from the loader.
//
// IN: A = 0 - play a blank (silent song)
// IN: A >= 1 - Play song number `A`
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function LoadSong {
    assert(TAD__FIRST_LOADING_SONG_STATE > TadState.NULL)
    assert(TAD__FIRST_LOADING_SONG_STATE > TadState.WAITING_FOR_LOADER_COMMON)
    assert(TAD__FIRST_LOADING_SONG_STATE > TadState.WAITING_FOR_LOADER_SONG)
    assert(TAD__FIRST_LOADING_SONG_STATE > TadState.LOADING_COMMON_AUDIO_DATA)


    sta.w   TadPrivate_nextSong

    lda.b   #TadFlags.RELOAD_COMMON_AUDIO_DATA
    trb.w   flags
    beq     SongRequested
        // Common audio data requested
        lda.b   #TadState.WAITING_FOR_LOADER_COMMON
        bra     SetStateAndSwitchToLoader

SongRequested:
    lda.w   TadPrivate_state
    cmp.b   #TAD__FIRST_LOADING_SONG_STATE
    bcc     Return
        // TadState is not NULL, WAITING_FOR_LOADER_* or LOADING_COMMON_AUDIO_DATA

        lda.b   #TadState.WAITING_FOR_LOADER_SONG

    SetStateAndSwitchToLoader:
        sta.w   TadPrivate_state

        // Assert it is safe to send a switch-to-loader command when the loader is waiting for a READY signal
        assert(TadIO_ToDriver.SWITCH_TO_LOADER != TadIO_Loader_Init.LOADER_READY_H)
        assert(TadIO_ToDriver.SWITCH_TO_LOADER_PORT == TadIO_Loader_Init.READY_PORT_H)

        // Send a *switch-to-loader* command to the audio-driver or loader
        lda.b   #TadIO_ToDriver.SWITCH_TO_LOADER
        sta.l   TadIO_ToDriver.SWITCH_TO_LOADER_PORT
Return:
    rts
}


// Calls `LoadSong` if `A` != the song_id used in the last `LoadSong` call.
//
// See `LoadSong` for details about how songs are loaded into the audio driver.
//
// IN: A = 0 - play a blank (silent song)
// IN: A >= 1 - Play song number `A`
//
// OUT: Carry set if the song_id changed and `LoadSong` was called.
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function LoadSongIfChanged {
    cmp.w   TadPrivate_nextSong
    beq     +
        jsr     LoadSong
        sec
        rts
    +
    clc
    rts
}


// Returns the current song id in the A register.
//
// OUT: A = The song_id used in the last `LoadSong` call.
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function GetSong {
    // `TadPrivate_nextSong` is only written to in `Init` and `LoadSong`.
    lda.w   TadPrivate_nextSong
    rts
}


// Sets the `TadFlags::RELOAD_COMMON_AUDIO_DATA` flag
//
// If this subroutine is called, the *common audio data* will be reloaded into Audio-RAM.
// This will not take effect until the next song is loaded into Audio-RAM.
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function ReloadCommonAudioData {
    lda.b   #TadFlags.RELOAD_COMMON_AUDIO_DATA
    tsb.w   flags
    rts
}


// Sets the `TadFlags::PLAY_SONG_IMMEDIATELY` flag
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function SongsStartImmediately {
    lda.b   #TadFlags.PLAY_SONG_IMMEDIATELY
    tsb.w   flags
    rts
}


// Clears the `TadFlags::PLAY_SONG_IMMEDIATELY` flag
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function SongsStartPaused {
    lda.b   #TadFlags.PLAY_SONG_IMMEDIATELY
    trb.w   flags
    rts
}


// Sets the `TadFlags::RESET_GLOBAL_VOLUMES_ON_SONG_START` flag.
//
// The global music and sound-effect volumes will be reset when a song starts.
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function GlobalVolumesResetOnSongStart {
    lda.b   #TadFlags.RESET_GLOBAL_VOLUMES_ON_SONG_START
    tsb.w   flags
    rts
}


// Clears the `TadFlags::RESET_GLOBAL_VOLUMES_ON_SONG_START` flag.
//
// The global music and sound-effect volumes will persist when a song or common-audio-data
// is loaded into audio-RAM.
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function GlobalVolumesPersist {
    lda.b   #TadFlags.RESET_GLOBAL_VOLUMES_ON_SONG_START
    trb.w   flags
    rts
}


// Sets the number of bytes to transfer to Audio-RAM per `Process` call.
//
// The value will be clamped from `TAD_MIN_TRANSFER_PER_FRAME` to `TAD_MAX_TRANSFER_PER_FRAME`.
//
// IN: X = number of bytes to transfer on every `Process` call.
//
// A unknown
// I16
// DB access lowram
au()
i16()
code()
function SetTransferSize {
    cpx.w   #TAD_MAX_TRANSFER_PER_FRAME
    bcc     +
        ldx.w   #TAD_MAX_TRANSFER_PER_FRAME
    +
    cpx.w   #TAD_MIN_TRANSFER_PER_FRAME
    bcs     +
        ldx.w   #TAD_MIN_TRANSFER_PER_FRAME
    +
    stx.w   TadPrivate_bytesToTransferPerFrame

    rts
}



// OUT: Carry set if the loader is still using data returned by `LoadAudioData` (state == `LOADING_*`)
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function IsLoaderActive {
    TadPrivate_IsLoaderActive()
    rts
}



// OUT: Carry set if the song is loaded into audio-RAM (state is `PAUSED`, `PLAYING_SFX` or `PLAYING`)
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function IsSongLoaded {
    assert(TadState.PLAYING_SFX > TadState.PAUSED)
    assert(TadState.PLAYING > TadState.PAUSED)
    // Assumes PLAYING is the last state

    lda.w   TadPrivate_state
    cmp.b   #TadState.PAUSED
    rts
}



// OUT: Carry set if the song is loaded into audio-RAM and sound effects are not paused
//      (state is `PLAYING_SFX` or `PLAYING`)
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function IsSfxPlaying {
    assert(TadState.PLAYING > TadState.PLAYING_SFX)
    // Assumes PLAYING is the last state

    lda.w   TadPrivate_state
    cmp.b   #TadState.PLAYING_SFX
    rts
}



// OUT: Carry set if the song is playing (state is `PLAYING`)
//
// A8
// I unknown
// DB access lowram
a8()
iu()
code()
function IsSongPlaying {
    // Assumes PLAYING is the last state

    lda.w   TadPrivate_state
    cmp.b   #TadState.PLAYING
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

