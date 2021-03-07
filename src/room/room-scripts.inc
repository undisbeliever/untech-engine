// room/room-scripts.inc
// =====================
//
// Room Script routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

RegisterBreakId(INVALID_ROOM_SCRIPT)


namespace Room {
namespace RoomScripts {


// Load and verifies the size of the Room Script data
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X = address of the room script data
// OUTPUT: X = address after room script data
macro _Load() {
    assert16a()
    assert16i()

    // Confirm this is an Room Entrances data block
    sep     #$20
a8()
    lda.w   0,x
    cmp.b   #Format.RoomScripts.IDENTIFIER
    beq     +
    MissingRoomScriptsIdentifier:
        break(INVALID_ROOM_DATA)
    +

    rep     #$30
a16()

    // Calculate the indexes of the start and end of the script data block
    // and store them in `_tmp` and `_tmpEnd`
    txa
    sec
    sbc.w   #roomData - Format.RoomScripts.size - 1
    sta.b   _tmp
    clc
    adc.w   Format.RoomScripts.blockSize + 1,x
    sta.b   _tmpEnd


    // Calculate the address of the end of the script block
    clc
    adc.w   #roomData
    tax

    // X = address at end of the block
    // MUST NOT MODIFY X


    // Verify each script in the header is valid
    ldy.w   #(N_ROOM_SCRIPTS - 1) * 2
    Loop:
        // Skip script if it is unused
        lda.w   Format.Header.scripts,y
        beq     EndIf

            // Break if the script's program counter is < _tmp or >= _tmpEnd
            cmp.b   _tmp
            bcc     InvalidRoomScriptPC

            cmp.b   _tmpEnd
            bcc     EndIf
            InvalidRoomScriptPC:
                break(INVALID_ROOM_SCRIPT)

    EndIf:

        dey
        dey
        bpl     Loop
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

