// Test to confirm the bass and 64tass APIs are identical (64tass side)
//
// This test has been ported to bass and untech memory macros

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

architecture wdc65816-strict

define MEMORY_MAP = LOROM
define ROM_SIZE = 1
define ROM_SPEED = slow
define REGION = US
define VERSION = 0

include "../../engine/common/assert.inc"
include "../../engine/common/cpu.inc"
include "../../engine/common/memory.inc"


createCodeBlock(code,       0x808000, 0x80ff8f)
createCodeBlock(farCode,    0x818000, 0x81ff8f)

createRamBlock(zeropage,    0x000000, 0x0000ff)
createRamBlock(lowram,      0x7e0200, 0x7e1fff)


constant TAD_IO_VERSION = 20

include "../../engine/audio/_variables.inc"
include "../../engine/audio/process.inc"
include "../../engine/audio/control.inc"


namespace Audio {
    // 64tass will auto-remove unreferenced `.proc` blocks
    //
    // Referencing them in this function table will unsure they are included in the output ROM
    code()
    TadSubroutines:
        dl Init__far
        dl Process__far
        dl FinishLoadingData__far
        dl QueueCommand
        dl QueueCommandOverride
        dl QueuePannedSoundEffect
        dl QueueSoundEffect
        dl LoadSong
        dl LoadSongIfChanged
        dl GetSong
        dl ReloadCommonAudioData
        dl SongsStartImmediately
        dl SongsStartPaused
        dl GlobalVolumesResetOnSongStart
        dl GlobalVolumesPersist
        dl SetTransferSize
        dl IsLoaderActive
        dl IsSongLoaded
        dl IsSfxPlaying
        dl IsSongPlaying

    TadConstants:
        dw TAD_MAX_PAN
        dw TAD_CENTER_PAN
        dw TAD_MIN_TICK_CLOCK

        db TadCommand.PAUSE
        db TadCommand.PAUSE_MUSIC_PLAY_SFX
        db TadCommand.PLAY_SOUND_EFFECT
        db TadCommand.STOP_SOUND_EFFECTS
        db TadCommand.SET_MAIN_VOLUME
        db TadCommand.SET_MUSIC_CHANNELS
        db TadCommand.SET_SONG_TIMER
        db TadCommand.SET_GLOBAL_MUSIC_VOLUME
        db TadCommand.SET_GLOBAL_SFX_VOLUME
        db TadCommand.SET_GLOBAL_VOLUMES

        db TadAudioMode.MONO
        db TadAudioMode.STEREO
        db TadAudioMode.SURROUND

        db TadFlags.RELOAD_COMMON_AUDIO_DATA
        db TadFlags.PLAY_SONG_IMMEDIATELY
        db TadFlags.RESET_GLOBAL_VOLUMES_ON_SONG_START
}

constant Tad_Loader_Bin          =   $1234
constant Tad_AudioDriver_Bin     =   $9abc
constant LoadAudioData           = $80def0

constant Tad_Loader_Bin.size     =  116
constant Tad_AudioDriver_Bin.size= 2048



