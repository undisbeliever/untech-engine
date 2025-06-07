// room/_common.inc
// =================
//
// Room Management routines
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


RegisterBreakId(INVALID_ROOM_ID)
RegisterBreakId(INVALID_ROOM_DATA)


namespace Room {


// Load a room into memory
//
// NOTE: This routine will not setup the PPU or load the map into VRAM.
//       Please call `TransferToPpu` after calling this routine to
//       upload decompressed graphics and transfer the on-screen map to
//       the PPU.
//
// NOTE: This routine will reset the following subsystems:
//        * Resources
//        * MetaTile subsystem
//        * Text Console
//        * MetaSprite buffer
//        * Entity Subsystem
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: GameState.roomId = room Id
// INPUT: GameState.entranceId = entrance to load
// INPUT: GameState.playerDataId = player entity data id
a16()
i16()
code()
function LoadRoom {
constant _tmp    = zpTmp0
constant _tmpEnd = zpTmp1
constant _tmpX   = zpTmp2
constant _count  = zpTmp3

    sep     #$10
i8()
    ldx.w   GameState.roomId
    cpx.b   #RoomList.count
    bcc     +
    InvalidRoomId:
        break(INVALID_ROOM_ID)
    +


    GameState._ClearRoomState()

    Scripting._Init()


    sep     #$20
a8()
    stz.w   roomChangeRequested

    rep     #$30
a16()
i16()

    Maps._Init()
    EntityGroups._Init()


    jsr     Entity.Init
    // Also initializes DMA, MetaSprites, camera

    // NOTE: LoadScene will initialize MetaTiles subsystem



    // Decompress room data
    lda.w   GameState.roomId
    and.w   #0xff
    sta.b   _tmp
    asl
    // carry clear
    adc.b   _tmp
    tax

    assertZeroPage(LZ4.ptr)
    lda.l   RoomList,x
    sta.b   LZ4.ptr
    lda.l   RoomList + 1,x
    sta.b   LZ4.ptr + 1

    ldx.w   #roomData
    lda.w   #roomData >> 16
    ldy.w   #roomData.size

    jsr     LZ4.Decompress


    // Load and verify room data

    // NOTE: These macros use the DP temporary variables
    //       They MUST NOT be accessed beyond this point.

    Maps._Load()
    // X = address of data after map

    Entrances._Load()
    // X = address of data after room entrances

    EntityGroups._Load()
    // X = address at end of entity groups

    RoomScripts._Load()
    // X = address at end of room scripts


    // Check footer (and confirm all data is read with the correct offsets/sizes)
    assert(Format.FOOTER_SIZE == 4)
    lda.w   0,x
    cmp.w   #Format.FOOTER & 0xffff
    bne     InvalidFooter
    lda.w   2,x
    cmp.w   #(Format.FOOTER >> 16) & 0xffff
    beq     +
    InvalidFooter:
        break(INVALID_ROOM_DATA)
    +


    // Script Triggers does not need to hold X
    ScriptTriggers._Load()


    jsr     Camera.Init


    Room.Entrances._SpawnPlayer()

    // Center camera on player
    // ::TODO define multiple cameras and select active camera::
    jsr     Camera.CenterOnPlayer


    Scripting._ProcessRoomStartupScript()


    lda.w   Format.Header.sceneId
    jmp     Resources.LoadScene
}


// Process Room subsystem.
//
// This function will:
//   * Check and activate script triggers
//
// This function should be called at the end of the game loop.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function Process {
    Room.ScriptTriggers._Process()

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

