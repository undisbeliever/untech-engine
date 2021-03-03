// room/_common.inc
// =================
//
// Room Management routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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
// INPUT: A = room Id
//
// INPUT: entranceId = entrance to load
// INPUT: playerDataId = player entity data id
a16()
i16()
code()
function LoadRoom {
constant _tmp    = utDpTmp0
constant _tmpEnd = utDpTmp1
constant _tmpX   = utDpTmp2
constant _count  = utDpTmp3

    sep     #$20
a8()
    cmp.b   #RoomList.count
    bcc     +
    InvalidRoomId:
        break(INVALID_ROOM_ID)
    +
    sta.w   GameState.roomId

    rep     #$20
    sep     #$10
a16()
i8()
    GameState._ClearRoomState()

    Scripting._Init()


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

