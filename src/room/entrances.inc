// room/entrances.inc
// ====================
//
// Room Entrances routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

RegisterBreakId(INVALID_ROOM_ENTRANCE)


namespace Room {
namespace Entrances {


// Load and verifies the size of the Room Entrances data
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X = address of the room entrances data
// OUTPUT: X = address after room entrances data
macro _Load() {
    assert16a()
    assert16i()

    // Confirm this is an Room Entrances data block
    sep     #$20
a8()
    lda.w   0,x
    cmp.b   #Format.RoomEntrance.IDENTIFIER
    beq     +
    MissingEntityGroupIdentifier:
        break(INVALID_ROOM_DATA)
    +
    inx

    stx.w   Entrances.dataAddr


    // Advance pointer to the end of the Room Entrances data block
    lda.w   Format.Header.nEntrances

    rep     #$30
a16()
    and.w   #0xff

    assert(Format.RoomEntrance.size == 1 << 2)
    asl
    asl
    sta.b   _tmp

    txa
    clc
    adc.b   _tmp
    tax
}


// Spawn player at the entrance `entranceId`
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: Room loaded
//
// ASSUMES: MetaSprite Palette/VRAM slots empty
// ASSUMES: Dma Queue empty
//
// INPUT: entranceId = entrance to load
// INPUT: playerDataId = player entity data id
macro _SpawnPlayer() {
    assert16a()
    assert16i()

    sep     #$20
a8()
    lda.w   entranceId
    cmp.w   Format.Header.nEntrances
    bcc     +
    InvalidRoomEntrance:
        break(INVALID_ROOM_ENTRANCE)
        // ::TODO turn into warning and load entrance 0::
    +

    rep     #$30
a16()
    and.w   #0xff
    assert(Format.RoomEntrance.size == 1 << 2)
    asl
    asl
    // carry clear
    adc.w   Entrances.dataAddr
    tax

    // X = RoomEntrance address

    // ::TODO support multiple players::

    // Convert three byte position to entity position
    lda.w   Format.RoomEntrance.position,x
    and.w   #Format.RoomEntrance.POSITION_MASK
    clc
    adc.w   #MetaTiles.map.LEFT
    sta.w   Entity.Player.xPos.px

    assert(Format.RoomEntrance.POSITION_SHIFT == 8 + 4)
    lda.w   Format.RoomEntrance.position + 1,x
    lsr
    lsr
    lsr
    lsr
    and.w   #Format.RoomEntrance.POSITION_MASK
    clc
    adc.w   #MetaTiles.map.TOP
    sta.w   Entity.Player.yPos.px


    lda.w   Format.RoomEntrance.orientation,x
    and.w   #Format.RoomEntrance.ORIENTATION_MASK
    tax
    lda.l   Room.Entrances.OrientationTable,x

    Entity._SpawnPlayer()
}


rodata(rom0)
OrientationTable:
namespace OrientationTable {
    constant LEFT = 0
    constant RIGHT = Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG

    db  RIGHT | Entity.MovementState.MOVING_DOWN_FLAG // DOWN_RIGHT
    db  LEFT  | Entity.MovementState.MOVING_DOWN_FLAG // DOWN_LEFT
    db  RIGHT | Entity.MovementState.UP_GRAVITY_FLAG  // UP_RIGHT
    db  LEFT  | Entity.MovementState.UP_GRAVITY_FLAG  // UP_LEFT
}
assert(pc() - OrientationTable == Format.RoomEntrance.ORIENTATION_MASK + 1)

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

