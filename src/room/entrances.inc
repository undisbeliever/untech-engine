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
// INPUT: entranceId = entrance to load
// INPUT: playerEntityDataId = player entity data id
a16()
i16()
code()
function SpawnPlayer {
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
    adc.w   dataAddr
    tax

    // X = RoomEntrance address

    // ::TODO hard code player address::
    // ::TODO support multiple players::

    // Convert three byte position to entity position
    lda.w   Format.RoomEntrance.position,x
    and.w   #Format.RoomEntrance.POSITION_MASK
    clc
    adc.w   #MetaTiles.map.LEFT
    sta.w   Entity.Spawn.xPos

    assert(Format.RoomEntrance.POSITION_SHIFT == 8 + 4)
    lda.w   Format.RoomEntrance.position + 1,x
    lsr
    lsr
    lsr
    lsr
    and.w   #Format.RoomEntrance.POSITION_MASK
    clc
    adc.w   #MetaTiles.map.TOP
    sta.w   Entity.Spawn.yPos


    lda.w   Format.RoomEntrance.orientation,x
    and.w   #Format.RoomEntrance.ORIENTATION_MASK
    pha

        // Spawn player entity
        lda.w   Room.playerEntityDataId
        and.w   #0xff
        ldy.w   #0
        jsr     Entity.Spawn

        // Y = entity address

    // Set player orientation
    // ::TODO move into spawn player code::
    plx
    sep     #$20
a8()
    lda.l   OrientationTable,x
    sta.w   BaseEntity.movementState,y

    rep     #$30
a16()

    // Center camera on player
    // ::TODO define multiple cameras and select active camera::
    tyx
    jmp     Camera.CenterOnEntity_X


rodata(rom0)
OrientationTable:
    db  Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.MOVING_DOWN_FLAG // DOWN_RIGHT
    db  Entity.MovementState.MOVING_DOWN_FLAG                                          // DOWN_LEFT
    db  Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.UP_GRAVITY_FLAG  // UP_RIGHT
    db  Entity.MovementState.FACING_RIGHT_FLAG | Entity.MovementState.UP_GRAVITY_FLAG  // UP_LEFT

assert(pc() - OrientationTable == Format.RoomEntrance.ORIENTATION_MASK + 1)
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

