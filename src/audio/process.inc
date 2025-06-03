// audio/process.inc
// =================
//
// Terrific Audio Driver Initialisation and processing subroutines
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


// ==================
// Loader subroutines
// ==================


// Transfer and execute Loader using the IPL
//
// REQUIRES: S-SMP reset and no data has been written to it yet
//
// This macro MUST only be called once.  There is no way to reset the S-SMP and restart the IPL.
//
// A8
// I16
// DB access registers
macro TadPrivate_Loader_TransferLoaderViaIpl() {
    assert8a()
    assert16i()

constant APUIO0 = $2140
constant APUIO1 = $2141
constant APUIO2 = $2142
constant APUIO3 = $2143

    // Clear start command port (just in case APUIO0 has $cc in it)
    // SOURCE: `blarggapu.s` from lorom-template, originally written by blargg (Shay Green)
    stz.w   APUIO0

    // Wait for ready signal
    ldy.w   #$bbaa
    -
        cpy.w   APUIO0
        bne     -

    ldx.w   #TAD_LOADER_ARAM_ADDR
    lda.b   #$cc
    stx.w   APUIO2              // destination ARAM address
    sta.w   APUIO1              // non-zero = write data to address
    sta.w   APUIO0              // New data command (non-zero and APUIO0 + more than 2, or $cc on the first transfer)

    // Wait for a response from the IPL
    -
        cmp.w   APUIO0
        bne     -


    // Transfer the data
    sep     #$30
i8()
    ldx.b   #0
    IplLoop:
        // Send the next byte to the IPL
        lda.l   Tad_Loader_Bin,x
        sta.w   APUIO1

        // Tell the IPL the next byte is ready
        stx.w   APUIO0

        // Wait for a response form the IPL
        -
            cpx.w   APUIO0
            bne     -

        inx
        cpx.b   #Tad_Loader_Bin.size
        bcc     IplLoop

    rep     #$10
i16()

    // Send an execute program command to the IPL
    ldx.w   #TAD_LOADER_ARAM_ADDR
    stx.w   APUIO2                  // A-RAM address
    stz.w   APUIO1                  // zero = execute program at A-RAM address
    lda.b   #Tad_Loader_Bin.size + 2
    sta.w   APUIO0                  // New data command (must be +2 the previous APUIO0 write)
}



// Sends a TadLoaderDataType byte to the loader if the loader is ready
//
// Assumes loader just started OR a `SWITCH_TO_LOADER` message was sent to the audio driver/loader.
//
// IN: A = TadLoaderDataType value
// OUT: carry = loader is ready and TadLoaderDataType sent
//
a8()
i16()
// DB access registers
farCode()
function TadPrivate_Loader_CheckReadyAndSendLoaderDataType {
    // Test if the loader is ready
    ldx.w   #TadIO_Loader_Init.LOADER_READY_HL
    cpx.w   TadIO_Loader_Init.READY_PORT_HL
    bne     ReturnFalse
        // Send the ready signal and the TadLoaderDataType
        sta.w   TadIO_Loader_Init.LOADER_DATA_TYPE_PORT

        lda.b   #TadIO_Loader_Init.LOADER_READY_L
        sta.w   TadIO_Loader_Init.READY_PORT_L

        lda.b   #TadIO_Loader_Init.LOADER_READY_H
        sta.w   TadIO_Loader_Init.READY_PORT_H

        // The S-CPU must wait for the loader to write 0 to the spinlock before transferring data.
        stz.w   TadPrivate_dataToTransfer_prevSpinLock

        // return true
        sec
        rts

ReturnFalse:
    clc
    rts
}



// Set the data transfer queue
//
// IN: A:X = far address
// IN: Y = size
a8()
i16()
// DB access registers
farCode()
function TadPrivate_Loader_SetDataToTransfer {
    stx.w   TadPrivate_dataToTransfer_addr
    sta.w   TadPrivate_dataToTransfer_bank
    sty.w   TadPrivate_dataToTransfer_size

    rts
}



// Transfer data to the audio loader.
//
// ASSUMES: `check_ready_and_send_loader_data_type` and `set_data_to_transfer` were previously called.
//
// NOTE: This function may read one byte past the end of the transfer queue.
//
// OUT: carry set if all data in the transfer queue was sent to Audio-RAM.
//
a8()
i16()
// DB access lowram
farCode()
function TadPrivate_Loader_TransferData {
    // Early exit if the loader is not ready
    //
    // This test doubles as a lock for the previous transfer.
    //
    // This also prevents a freeze in `process()` if the loader has crashed/glitched.
    // (`finish_loading_data()` will freeze if the loader has crashed/glitched.
    lda.w   TadPrivate_dataToTransfer_prevSpinLock
    cmp.l   TadIO_Loader.SPINLOCK_PORT
    bne     ReturnFalse

    phd
    phb

    rep     #$30
a16()

    // Calculate number of words to read
    lda.w   TadPrivate_dataToTransfer_size
    cmp.w   TadPrivate_bytesToTransferPerFrame
    bcc     +
        lda.w   TadPrivate_bytesToTransferPerFrame
    +
    inc     // required
    lsr

    // Prevent corrupting all of Audio-RAM if number of words == 0
    bne     +
        inc
    +
    // Store word to read in X
    tax

    // Reverse subtract TadPrivate_dataToTransfer_size (with clamping)
    asl                             // convert number of words to number of bytes
    eor.w   #$ffff
    sec
    adc.w   TadPrivate_dataToTransfer_size
    bcs     +
        lda.w   #0
    +
    sta.w   TadPrivate_dataToTransfer_size


    lda.w   #$2100
    tcd
// D = $2100

    sep     #$20
a8()

    lda.w   TadPrivate_dataToTransfer_bank
    ldy.w   TadPrivate_dataToTransfer_addr

    pha
    plb
// DB = TadPrivate_dataToTransfer_bank

    Loop:
        // x = number of words remaining
        // y = data address (using y to force addr,y addressing mode)

        lda.w   0,y
        sta.b   TadIO_Loader.DATA_PORT_L & $ff

        // The bank overflow test must be done here as `TadPrivate_dataToTransfer_addr` might point to an odd memory address.
        iny
        beq     BankOverflow_1
    BankOverflow_1_Resume:

        lda.w   0,y
        sta.b   TadIO_Loader.DATA_PORT_H & $ff

        // Increment this spinloack value
        //
        // The upper 4 bits of the spinlock must be clear'
        // Cannot be 0.  Zero is used to spinlock the loader init before this loop starts
        //               (see Loader Step 3 in `terrific-audio-driver/audio-driver/src/io-commands.wiz`)

        assert(($ffff & 7) + 1 < TadIO_Loader.SPINLOCK_MASK)
        tya             // y = address of data, it should always increment by 2
        and.b   #7
        inc
        sta.b   TadIO_Loader.SPINLOCK_PORT & $ff

        iny
        beq     BankOverflow_2
    BankOverflow_2_Resume:

        dex
        beq     EndLoop

        // Spinloop until the S-SMP has acknowledged the data
        -
            cmp.b   TadIO_Loader.SPINLOCK_PORT & $ff
            bne     -

        bra     Loop
EndLoop:

    plb
    pld
// DB restored
// D = 0

    sty.w   TadPrivate_dataToTransfer_addr
    sta.w   TadPrivate_dataToTransfer_prevSpinLock


    ldy.w   TadPrivate_dataToTransfer_size
    bne     ReturnFalse
        // End of data transfer

        // Wait for Loader to acknowledge the last write
        -
            cmp.l   TadIO_Loader.SPINLOCK_PORT
            bne     -

        // No more data to transfer
        lda.b   #TadIO_Loader.SPINLOCK_COMPLETE
        sta.l   TadIO_Loader.SPINLOCK_PORT

        sec
        rts

ReturnFalse:
    clc
    rts


BankOverflow_1:
    jsr     TadPrivate_Loader_GotoNextBank
    bra     BankOverflow_1_Resume

BankOverflow_2:
    // Must save/restore A, it holds the spinlock
    pha
        jsr     TadPrivate_Loader_GotoNextBank
    pla
    bra     BankOverflow_2_Resume
}


// Advance to the next bank
//
// MUST only be called to TadPrivate_Loader_TransferData
//
// ASSUMES: Y = 0 (Y addr overflowed to 0)
//
// IN: Y = 0
// IN: DB = TadPrivate_dataToTransfer_bank
//
// OUT: Y = new address
// OUT: DB = new bank
//
// KEEP: X
a8()
i16()
// DB = TadPrivate_dataToTransfer_bank
farCode()
function TadPrivate_Loader_GotoNextBank {
    phb
    pla

    inc
    sta.l   TadPrivate_dataToTransfer_bank

    pha
    plb
// DB = new TadPrivate_dataToTransfer_bank value

    // MUST NOT CHANGE X

    // Y = 0
    if MEMORY_MAP.{MEMORY_MAP} == MEMORY_MAP.LOROM {
        and.b   #$fe
        cmp.b   #$7e
        beq     +
            // Bank is not Work-RAM
            ldy.w   #$8000
        +
    } else if MEMORY_MAP.{MEMORY_MAP} == MEMORY_MAP.HIROM {
        and.b   #$7f
        cmp.b   #$40
        bcs     +
            // Bank is a register bank
            // set Y to the first ROM address
            ldy.w   #$8000
        +
    } else {
        error "Unknown memory map"
    }

    // Y = 0 or $8000
    rts
}


// OUT: carry set if state is LOADING_*
// A8
macro TadPrivate_IsLoaderActive() {
    assert8a()

    assert(TadState.NULL < TAD__FIRST_LOADING_STATE)
    assert(TadState.WAITING_FOR_LOADER_COMMON < TAD__FIRST_LOADING_STATE)
    assert(TadState.WAITING_FOR_LOADER_SONG < TAD__FIRST_LOADING_STATE)
    assert((TadState.PAUSED & $7f) < TAD__FIRST_LOADING_STATE)
    assert((TadState.PLAYING & $7f) < TAD__FIRST_LOADING_STATE)

    lda.w   TadPrivate_state
    and.b   #$7f
    cmp.b   #TAD__FIRST_LOADING_STATE
}


// ==========
// Public API
// ==========

// -------------------------------
// TAD_PROCESS_SEGMENT subroutines
// -------------------------------


// Initialises the audio driver:
//
//  * Loads the loader into Audio-RAM
//  * Loads the audio driver into Audio-RAM
//  * Sets the song to 0 (silence)
//  * Resets variables
//  * Initialises `flags` and `audioMode`
//  * Queues a common audio data transfer
//
// This function will require multiple frames of execution time.
//
// REQUIRES: S-SMP reset
//
// TIMING:
//  * Should be called more than 40 scanlines after reset
//  * MUST be called ONCE
//     * Calling `Init` more than once will hardlock.
//  * MUST be called with INTERRUPTS DISABLED
//  * MUST be called while the S-SMP is running the IPL.
//  * MUST be called after the `LoadAudioData` callback is setup (if necessary)
//  * `Init` MUST be called before any other TAD subroutine.
//
// Called with JSL long addressing (returns with RTL).
// A8
// I16
// DB unknown
a8()
i16()
// DB unknown
// DP = 0
farCode()
function Init__far {
    phb

    lda.b   #$80
    pha
    plb
// DB = $80

    TadPrivate_Loader_TransferLoaderViaIpl()


    // Set default settings
    assert((TAD_DEFAULT_FLAGS) & TadFlags.RELOAD_COMMON_AUDIO_DATA == 0)
    assert((TAD_DEFAULT_FLAGS) & TadFlags._ALL_FLAGS == (TAD_DEFAULT_FLAGS))
    assert((TAD_DEFAULT_AUDIO_MODE) >= 0 && (TAD_DEFAULT_AUDIO_MODE) < TAD_N_AUDIO_MODES)

    assert(flags + 1 == audioMode)
    ldx.w   #(TAD_DEFAULT_FLAGS) | ((TAD_DEFAULT_AUDIO_MODE) << 8)
    stx.w   flags

    ldx.w   #TAD_DEFAULT_TRANSFER_PER_FRAME
    stx.w   TadPrivate_bytesToTransferPerFrame


    lda.b   #Tad_AudioDriver_Bin >> 16
    ldx.w   #Tad_AudioDriver_Bin & $ffff
    ldy.w   #Tad_AudioDriver_Bin.size
    jsr     TadPrivate_Loader_SetDataToTransfer

    lda.b   #$ff
    sta.w   TadPrivate_nextCommand_id
    sta.b   sfxQueue_sfx

    stz.w   TadPrivate_nextSong

    DataTypeLoop:
        lda.b   #TadLoaderDataType.CODE
        jsr     TadPrivate_Loader_CheckReadyAndSendLoaderDataType
        bcc     DataTypeLoop

    TransferLoop:
        jsr     TadPrivate_Loader_TransferData
        bcc     TransferLoop

    lda.b   #TadState.WAITING_FOR_LOADER_COMMON
    sta.w   TadPrivate_state

    plb
// DB restored
    rtl
}


// Sends a command to the audio driver.
//
// REQUIRES: state == PAUSED or state == PLAYING.
// REQUIRES: The previous command has been processed by the audio-driver.
// REQUIRES: `TadPrivate_nextCommand_id` is not a play-sound-effect command.
// REQUIRES: `TadPrivate_nextCommand_id` is a valid comma.
//
// IN: Y = TadPrivate_nextCommand_id
a8()
i8()
// DB access lowram
macro TadPrivate_Process_SendCommand() {
    assert8a()
    assert8i()

    lda.w   TadPrivate_nextCommand_parameter0
    sta.l   TadIO_ToDriver.PARAMETER0_PORT

    lda.w   TadPrivate_nextCommand_parameter1
    sta.l   TadIO_ToDriver.PARAMETER1_PORT

    lda.w   TadPrivate_previousCommand
    and.b   #TadIO_ToDriver.COMMAND_I_MASK    // Clear the non i bits of the command
    eor.b   #TadIO_ToDriver.COMMAND_I_MASK    // Flip the i bits
    ora.w   TadPrivate_nextCommand_id          // Set the c bits
    sta.l   TadIO_ToDriver.COMMAND_PORT
    sta.w   TadPrivate_previousCommand

    cpy.b   #TadCommand.UNPAUSE + 1
    bcs     NotPauseOrPlay
        // Change state if the command is a pause or play command
        assert(TadCommand.PAUSE == 0)
        assert(TadCommand.PAUSE_MUSIC_PLAY_SFX == 2)
        assert(TadCommand.UNPAUSE == 4)
        assert((TadCommand.PAUSE >> 1) & 3 | $80 == TadState.PAUSED)
        assert((TadCommand.PAUSE_MUSIC_PLAY_SFX >> 1) & 3 | $80 == TadState.PLAYING_SFX)
        assert((TadCommand.UNPAUSE >> 1) & 3 | $80 == TadState.PLAYING)
        lsr
        and.b   #3
        ora.b   #$80
        sta.w   TadPrivate_state
NotPauseOrPlay:

    // Reset command queue
    lda.b   #$ff
    sta.w   TadPrivate_nextCommand_id
}



// Send a play-sound-effect command to the audio driver.
//
// REQUIRES: state == PLAYING
// REQUIRES: The previous command has been processed by the audio-driver.
//
// IN: A = sfxQueue_sfx
//
// A8
// I8
// DB access lowram
// DP = 0
macro TadPrivate_Process_SendSfxCommand() {
    assert8a()
    assert8i()

    // parameter 0 = sfx_id
    sta.l   TadIO_ToDriver.PARAMETER0_PORT

    // parameter 1 = pan
    lda.b   sfxQueue_pan
    cmp.b   #TAD_MAX_PAN + 1
    bcc     +
        lda.b   #TAD_CENTER_PAN
    +
    sta.l   TadIO_ToDriver.PARAMETER1_PORT

    // Send play-sound-effect command
    lda.w   TadPrivate_previousCommand
    and.b   #TadIO_ToDriver.COMMAND_I_MASK            // Clear the non i bits of the command
    eor.b   #TadIO_ToDriver.COMMAND_I_MASK            // Flip the i bits
    ora.b   #TadCommand.PLAY_SOUND_EFFECT             // Set the c bits

    sta.l   TadIO_ToDriver.COMMAND_PORT
    sta.w   TadPrivate_previousCommand

    // Reset the SFX queue
    ldy.b   #$ff
    sty.b   sfxQueue_sfx
    sty.b   sfxQueue_pan
}



// Processes the next queue.
//
// This function will do one of the following, depending on the state:
//  * Transfer data to the Audio-RAM
//  * Wait for the loader and call the `LoadAudioData` callback when the loader is ready to receive new data
//  * Send a command to the audio driver
//  * Send a play-sound effect command to the audio driver
//
// NOTE: The command and sound-effect queues will be reset after a new song is loaded into Audio-RAM.
//
// TIMING:
//  * MUST be called after `Init`.
//  * Should be called once per frame.
//  * MUST NOT be called in an interrupt ISR.
//
// Called with JSL long addressing (returns with RTL).
// A8
// I16
// DB access lowram
a8()
i16()
// DB access lowram
// DP = 0
farCode()
function Process__far {
    assert(TadState.PAUSED == $80)
    assert(TadState.PLAYING > $80)
    lda.w   TadPrivate_state
    bpl     NotLoaded
        // Playing or paused state
        sep     #$10
    i8()
        tax

        lda.w   TadPrivate_previousCommand
        cmp.l   TadIO_ToScpu.COMMAND_ACK_PORT
        bne     Return_I8
            // Previous command has been processed

            // Check command queue
            ldy.w   TadPrivate_nextCommand_id
            bpl     SendCommand

            // X = TadPrivate_state
            assert(TadState.PAUSED < $81)
            assert(TadState.PLAYING >= $81)
            assert(TadState.PLAYING_SFX >= $81)
            dex
            bpl     Return_I8
                // Playing state
                lda.b   sfxQueue_sfx
                cmp.b   #$ff
                beq     Return_I8
                    TadPrivate_Process_SendSfxCommand()

        Return_I8:
            rep     #$10
        i16()
            rtl

        a8()
        i8()
        SendCommand:
            TadPrivate_Process_SendCommand()
            rep     #$10
        i16()
            rtl

    NotLoaded:
        // Song is not loaded into Audio-RAM

        // Test if state is WAITING_FOR_LOADER_* or LOADING_*
        assert(TAD__FIRST_LOADING_STATE > TAD__FIRST_WAITING_STATE)
        assert(TAD__FIRST_LOADING_STATE == TadState.WAITING_FOR_LOADER_SONG + 1)
        cmp.b   #TAD__FIRST_LOADING_STATE
        bcs     TadPrivate_Process_Loading
        cmp.b   #TAD__FIRST_WAITING_STATE
        bcs     TadPrivate_Process_WaitingForLoader

    // TadState is null
    rtl
}



// Process the WAITING_FOR_LOADER_* states
//
// return using RTL
a8()
i16()
// DB access lowram
farCode()
function TadPrivate_Process_WaitingForLoader { // RTL
    phb

    // Setting DB to access registers as it:
    //  * Simplifies `TadPrivate_Loader_CheckReadyAndSendLoaderDataType`
    //  * Ensures `LoadAudioData` is called with a fixed data bank
    //    (NOTE: `LoadAudioData` is tagged `DB access registers`)
    lda.b   #$80
    pha
    plb
// DB = $80

    lda.w   TadPrivate_state
    cmp.b   #TadState.WAITING_FOR_LOADER_COMMON
    bne     SongData
        // Common audio data
        lda.b   #TadLoaderDataType.COMMON_DATA
        jsr     TadPrivate_Loader_CheckReadyAndSendLoaderDataType
        bcc     Return

        lda.b   #TadState.LOADING_COMMON_AUDIO_DATA
        pha

        lda.b   #0
        bra     LoadData

    SongData:
        // Songs

        // flags MUST NOT have the stereo/surround loader flag set
        assert(TadFlags._ALL_FLAGS & TadLoaderDataType.STEREO_FLAG == 0)
        assert(TadFlags._ALL_FLAGS & TadLoaderDataType.SURROUND_FLAG == 0)

        // SONG_DATA_FLAG must always be sent and it also masks the RELOAD_COMMON_AUDIO_DATA flag in TadLoaderDataType
        assert(TadFlags.RELOAD_COMMON_AUDIO_DATA == TadLoaderDataType.SONG_DATA_FLAG)

        assert(TadFlags.PLAY_SONG_IMMEDIATELY == TadLoaderDataType.PLAY_SONG_FLAG)
        assert(TadFlags.RESET_GLOBAL_VOLUMES_ON_SONG_START == TadLoaderDataType.RESET_GLOBAL_VOLUMES_FLAG)

        // Clear unused TAD flags
        lda.b   #$ff ^ TadFlags._ALL_FLAGS
        trb.w   flags

        // Convert `audioMode` to TadLoaderDataType and combine with TadFlags
        assert(((0 + 1) & 3) == TadLoaderDataType.SURROUND_FLAG)
        assert(((1 + 1) & 3) == TadLoaderDataType.STEREO_FLAG)
        assert(((2 + 1) & 3) == TadLoaderDataType.STEREO_FLAG | TadLoaderDataType.SURROUND_FLAG)
        lda.w   audioMode
        inc
        and.b   #3

        ora.w   flags
        ora.b   #TadLoaderDataType.SONG_DATA_FLAG
        jsr     TadPrivate_Loader_CheckReadyAndSendLoaderDataType
        bcc     Return

        // Determine next state
        assert(TadFlags.PLAY_SONG_IMMEDIATELY == $40)
        assert(TadState.LOADING_SONG_DATA_PAUSED + 1 == TadState.LOADING_SONG_DATA_PLAY)
        lda.w   flags
        asl
        asl
        lda.b   #0
        // carry = PLAY_SONG_IMMEDIATELY flag
        adc.b   #TadState.LOADING_SONG_DATA_PAUSED
        pha

        // Load next song
        lda.w   TadPrivate_nextSong
        beq     UseBlankSong

LoadData:
    jsl     LoadAudioData
    bcs     +
        // LoadAudioData returned false
    UseBlankSong:
        // The blank song is a single zero byte.
        // ::HACK use the 3rd byte of `ldy #1` (which is `0x00`) for the blank song data::
        ldy.w   #1
        constant _BlankSongData = pc() - 1
        lda.b   #_BlankSongData >> 16
        ldx.w   #_BlankSongData
    +

    // STACK holds next state
    // A:X = data address
    // Y = data size
    jsr     TadPrivate_Loader_SetDataToTransfer

    // Must set state AFTER the `LoadAudioData` call.
    // `LoadAudioData` might call `FinishLoadingData`.
    pla
    sta.w   TadPrivate_state

Return:
    plb
// DB restored
    rtl
}



// Process the LOADING_* states
//
// return using RTL
a8()
i16()
// DB access lowram
// DP = 0
farCode()
function TadPrivate_Process_Loading { // RTL
    jsr     TadPrivate_Loader_TransferData
    bcc     Return
        // Data loaded successfully
        lda.w   TadPrivate_state
        cmp.b   #TadState.LOADING_COMMON_AUDIO_DATA
        bne     Song
            // Common audio data was just transferred
            // Loader is still active
            lda.b   #TadState.WAITING_FOR_LOADER_SONG
            bra     EndIf

        Song:
            // song data was loaded into Audio-RAM
            // Loader has finished, audio driver is now active

            stz.w   TadPrivate_previousCommand

            // Reset command and SFX queues
            lda.b   #$ff
            sta.w   TadPrivate_nextCommand_id
            sta.b   sfxQueue_sfx
            sta.b   sfxQueue_pan

            // Use `TadPrivate_state` to determine if the song is playing or paused.
            // Cannot use `flags` as it may have changed after the `TadLoaderDataType` was sent to
            // the loader (while the song was loaded).
            assert(((TadState.LOADING_SONG_DATA_PAUSED & 1) << 1) | $80 == TadState.PAUSED)
            assert(((TadState.LOADING_SONG_DATA_PLAY & 1) << 1) | $80 == TadState.PLAYING)
            lda.w   TadPrivate_state
            and.b   #1
            asl
            ora.b   #$80

        // A = new state
    EndIf:
        sta.w   TadPrivate_state

Return:
    rtl
}



// Finish loading the data into audio-RAM.
//
// `FinishLoadingData` will not transfer data if the state is `WAITING_FOR_LOADER`.
// It will only transfer data if the loader is in the middle of transferring data
// (when `IsLoaderActive` returns true).
//
// This function can be safely called by `LoadAudioData`.
//
// This function may require multiple frames of execution time.
//
// Called with JSL long addressing (returns with RTL).
// A8
// I16
// DB access lowram
a8()
i16()
// DB access lowram
farCode()
function FinishLoadingData__far {
    Loop:
        TadPrivate_IsLoaderActive()
        bcc     EndLoop
            jsl     TadPrivate_Process_Loading
        bra     Loop
    EndLoop:

    rtl
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

