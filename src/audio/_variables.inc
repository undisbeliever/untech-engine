// audio/_variables.inc
// ====================
//
// Terrific Audio Driver 65816 API variables.
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


namespace Audio {


assert(TAD_IO_VERSION == 20)


// =========
// CONSTANTS
// =========


// Address to store the loader (in Audio-RAM).
// Address (in Audio-RAM) to execute after loading the Loader.
// MUST match LOADER_ADDR in `audio-driver/src/common_memmap.wiz`.
constant TAD_LOADER_ARAM_ADDR = $0200


// Minimum transfer size accepted by `SetTransferSize`
//
// MUST BE > 0
constant TAD_MIN_TRANSFER_PER_FRAME = 32

// Maximum transfer size accepted by `SetTransferSize`
//
// The loader can transfer ~849 bytes per 60Hz frame SlowROM or FastROM
constant TAD_MAX_TRANSFER_PER_FRAME = 800



// ========
// IO Ports
// ========

// IO communication protocol version.
//
// Used by `tad-compiler ca65-export` to verify the IO protocol in `tad-audio.s` matches the audio-driver.
//
// This constant MUST be increased if `LOADER_ADDR` or the IO Communication protocol changes.
constant TAD_IO_VERSION = 20


// MUST match `audio-driver/src/io-commands.wiz`
namespace TadCommand {
    constant PAUSE = 0
    constant PAUSE_MUSIC_PLAY_SFX = 2
    constant UNPAUSE = 4
    constant PLAY_SOUND_EFFECT = 6
    constant STOP_SOUND_EFFECTS = 8
    constant SET_MAIN_VOLUME = 10
    constant SET_MUSIC_CHANNELS = 12
    constant SET_SONG_TIMER = 14
    constant SET_GLOBAL_MUSIC_VOLUME = 16
    constant SET_GLOBAL_SFX_VOLUME = 18
    constant SET_GLOBAL_VOLUMES = 20
}


// Maximum pan value (100% to the right)
constant TAD_MAX_PAN = 128

// Center pan value (default if pan is not specified)
constant TAD_CENTER_PAN = TAD_MAX_PAN / 2

// Minimum tick clock value for the `TadCommand::SET_SONG_TIMER` command.
constant TAD_MIN_TICK_CLOCK = 64



// MUST match `audio-driver/src/io-commands.wiz`
namespace TadIO_ToDriver {
    // The command to execute.
    //
    //      iiicccci
    //         cccc = command
    //            i = command id, MUST be different on every command.
    //                Used to detect when a new command has been sent to the driver.
    //
    // NOTES:
    //  * The command will only be execute if the `command` byte has changed.
    //  * This value MUST be written last.
    //  * The command and parameter bytes MUST NOT change unless the previous command
    //    has been acknowledged.
    constant COMMAND_PORT = $2140 // APUIO0

    constant COMMAND_MASK   = %00011110
    constant COMMAND_I_MASK = %11100001

    // The first command parameter port
    constant PARAMETER0_PORT = $2141 // APUIO1

    // The second command parameter port
    constant PARAMETER1_PORT = $2142 // APUIO2


    // Writing `SWITCH_TO_LOADER` to this port should stop execution and start the loader.
    //
    // If the audio-driver is running; if the `SWITCH_TO_LOADER_BIT` is set,
    // the audio driver will stop and execute the loader.
    //
    // If the loader is in the middle of a transfer and both the `SWITCH_TO_LOADER_BIT`
    // and MSB (bit 7) bits are set, the loader will restart.
    constant SWITCH_TO_LOADER_PORT = $2143 // APUIO3

    constant SWITCH_TO_LOADER_BIT = 5
    constant SWITCH_TO_LOADER = $80 | (1 << SWITCH_TO_LOADER_BIT)
}


// MUST match `audio-driver/src/io-commands.wiz`
namespace TadIO_ToScpu {
    // Audio driver command acknowledgment.
    //
    // Acknowledgment of the `ToDriver.command` byte.  Not used in the loader.
    //
    // After the command has been processed, the `IO.ToDriver.command` value will be written to this port.
    constant COMMAND_ACK_PORT = $2140 // APUIO0


    // The mode the S-SMP is currently executing.
    //
    // Used by both the loader and the audio-driver.
    //
    // NOTE: The IPL sets this value after at has cleared the zero-page.
    //       Do not read this value immediately after reset.
    //       Make sure enough time has passed for the IPL to set IO Port 1
    //       to $bb before reading this port.
    constant MODE_PORT = $2141 // APUIO1

    // The S-SMP is at the start of the IPL, waiting for the ready signal.
    constant MODE_IPL = $bb

    // The S-SMP is running the loader.
    constant MODE_LOADER = $4c // 'L', Loader.LOADER_READY_L

    // The S-SMP is running the audio-driver.
    constant MODE_AUDIO_DRIVER = $61 // 'a'
}


// MUST match `audio-driver/src/io-commands.wiz`
namespace TadLoaderDataType {
    constant CODE        = 0
    constant COMMON_DATA = 1

    constant SONG_DATA_FLAG            = 1 << 7
    constant PLAY_SONG_FLAG            = 1 << 6
    constant RESET_GLOBAL_VOLUMES_FLAG = 1 << 5

    constant STEREO_FLAG               = 1 << 1
    constant SURROUND_FLAG             = 1 << 0
}


// MUST match `audio-driver/src/io-commands.wiz`
namespace TadIO_Loader_Init {
    constant LOADER_DATA_TYPE_PORT = $2141 // APUIO1
    constant READY_PORT_L          = $2142 // APUIO2
    constant READY_PORT_H          = $2143 // APUIO3

    constant READY_PORT_HL         = $2142 // APUIO2 & APUIO3

    constant LOADER_READY_L = %01001100  // 'L'
    constant LOADER_READY_H = %01000100  // 'D'
    constant LOADER_READY_HL = LOADER_READY_L | (LOADER_READY_H << 8)
}


// MUST match `audio-driver/src/io-commands.wiz`
namespace TadIO_Loader {
    constant DATA_PORT_L   = $2141 // APUIO1
    constant DATA_PORT_H   = $2142 // APUIO2
    constant SPINLOCK_PORT = $2143 // APUIO3

    // The spinlock value when the audio driver starts playing a song
    constant SPINLOCK_INIT_VALUE = 0

    // Only the lower 4 bits of the spinlock should be set while sending data to the loader
    constant SPINLOCK_MASK = $0f

    // Signal to the loader that the transfer has completed.
    constant SPINLOCK_COMPLETE = $80

    // If this value is written to the spinlock, the loader will restart;
    constant SPINLOCK_SWITCH_TO_LOADER = TadIO_ToDriver.SWITCH_TO_LOADER
}


namespace TadState {
    constant NULL                                = $00
    // Waiting for loader to send the ready signal before loading common-audio-data
    constant WAITING_FOR_LOADER_COMMON           = $7b
    // Waiting for loader to send the ready signal before loading song data
    constant WAITING_FOR_LOADER_SONG             = $7c
    // Loading common audio data.
    constant LOADING_COMMON_AUDIO_DATA           = $7d
    // Loading a song and the TadLoaderDataType.PLAY_SONG_FLAG was clear.
    constant LOADING_SONG_DATA_PAUSED            = $7e
    // Loading a song and the TadLoaderDataType.PLAY_SONG_FLAG was set.
    constant LOADING_SONG_DATA_PLAY              = $7f
    // Song is loaded into Audio-RAM and the audio driver is paused.
    // No play-sound-effect commands will be sent when the driver is paused.
    constant PAUSED                              = $80
    // Song is loaded into Audio-RAM and the audio driver is playing sfx (song paused).
    constant PLAYING_SFX                         = $81
    // Song is loaded into Audio-RAM and the audio driver is playing the song.
    constant PLAYING                             = $82
}
constant TAD__FIRST_WAITING_STATE      = TadState.WAITING_FOR_LOADER_COMMON
constant TAD__FIRST_LOADING_STATE      = TadState.LOADING_COMMON_AUDIO_DATA
constant TAD__FIRST_LOADING_SONG_STATE = TadState.LOADING_SONG_DATA_PAUSED


namespace TadFlags {
    constant RELOAD_COMMON_AUDIO_DATA = 1 << 7
    constant PLAY_SONG_IMMEDIATELY = 1 << 6
    constant RESET_GLOBAL_VOLUMES_ON_SONG_START = 1 << 5

    // A mask for the flags that are sent to the loader
    constant _ALL_FLAGS = RELOAD_COMMON_AUDIO_DATA | PLAY_SONG_IMMEDIATELY | RESET_GLOBAL_VOLUMES_ON_SONG_START
}

namespace TadAudioMode {
    constant MONO = 0
    constant STEREO = 1
    constant SURROUND = 2
}

constant TAD_N_AUDIO_MODES = 3


// Default values
// ==============

// Using a single symbol to enable custom defaults as I am unable to detect if a `.define`
// exists using an if statement.
//
// I recommend using a `.define` for custom defaults so `TadFlags` and `TadAudioMode` values
// can be referenced before they are defined.
if !{defined TAD_CUSTOM_DEFAULTS} {
    // Default TAD flags
    // MUST NOT set RELOAD_COMMON_AUDIO_DATA
    constant TAD_DEFAULT_FLAGS = TadFlags.PLAY_SONG_IMMEDIATELY

    // Starting audio mode
    constant TAD_DEFAULT_AUDIO_MODE = TadAudioMode.MONO

    // Default number of bytes to transfer to Audio-RAM per `Process` call.
    //
    // MUST be between the TAD_MIN_TRANSFER_PER_FRAME and TAD_MAX_TRANSFER_PER_FRAME
    constant TAD_DEFAULT_TRANSFER_PER_FRAME = 256
}



// =========
// Variables
// =========


    // The current audio driver state
    // (`TadState` enum)
    allocate(TadPrivate_state, lowram, 1)

    // `TadFlags` bitfield
    // (see `TadFlags` namespace)
    allocate(flags, lowram, 1)

    // Mono/Stereo/Surround audio mode
    // (`TadAudioMode` enum)
    allocate(audioMode, lowram, 1)

    // Number of bytes to transfer per `Process` call
    //
    // MUST be > 0
    allocate(TadPrivate_bytesToTransferPerFrame, lowram, 2)

    // The previous `TadIO_ToScpu.COMMAND_PORT` sent to the S-SMP audio driver.
    allocate(TadPrivate_previousCommand, lowram, 1)


// ---------------------------------------------------
// Queue 1 - remaining data to transfer into Audio-RAM
// ---------------------------------------------------
    // A far pointer to the remaining data to transfer
    allocate(TadPrivate_dataToTransfer_addr, lowram, 2)
    allocate(TadPrivate_dataToTransfer_bank, lowram, 1)

    // The remaining number of bytes to transfer
    allocate(TadPrivate_dataToTransfer_size, lowram, 2)

    // The previous value written to the loader spinLock
    allocate(TadPrivate_dataToTransfer_prevSpinLock, lowram, 1)


// ----------------------------------------------
// Queue 2 - The next song to load into Audio-RAM
// ----------------------------------------------
    // The next song to load into Audio-RAM
    // Used by the `WAITING_FOR_LOADER_*` states
    // If this value is 0 or an invalid song, a blank silent song will be loaded instead.
    allocate(TadPrivate_nextSong, lowram, 1)


// ------------------------------------------------------
// Queue 3 - The next command to send to the audio driver
// ------------------------------------------------------
    // The next `TadCommand` to send to the audio driver.
    // If this value is negative, the queue is empty.
    allocate(TadPrivate_nextCommand_id, lowram, 1)

    // The two parameters of the next command (if any)
    allocate(TadPrivate_nextCommand_parameter0, lowram, 1)
    allocate(TadPrivate_nextCommand_parameter1, lowram, 1)


// ---------------------------------------
// Queue 4 - The next sound effect to play
// ---------------------------------------
    // see tad-audio.inc
    allocate(sfxQueue_sfx, zeropage, 1)
    allocate(sfxQueue_pan, zeropage, 1)

}

// vim: ft=bass-65816 ts=4 sw=4 et:

