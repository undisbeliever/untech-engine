// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
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


assert({defined DEBUG_BUILD})
assert(!{defined RELEASE_BUILD})


// Process second controller debug commands
//
// REQUIRES: 8 bit A, 8 bit Index, DB = 0x7e, DP = 0
macro ProcessDebugCommands() {
    assert8a()
    assert8i()

    lda.w   Controller.Joy2.current + 1
    bit.b   #JOYH.y
    beq     +
        inc.w   Project.GameState.Words.playerHealth
    +


    lda.w   Controller.Joy2.pressed + 1
    bit.b   #JOYH.up | JOYH.down | JOYH.left | JOYH.right
    beq     NotDpad
        ldx.w   GameState.roomId

        bit.b   #JOYH.left
        beq     NotLeft
            txy
            beq     EndDirectionIf
                dex
            bra     EndDirectionIf
        NotLeft:

        bit.b   #JOYH.right
        beq     NotRight
            inx
            cpx.b   #Room.RoomList.count
            bcc     +
                dex
            +
            bra     EndDirectionIf
        NotRight:

        bit.b   #JOYH.up
        beq     NotUp
            ldx.b   #Project.RoomList._scratchpad
        NotUp:

    EndDirectionIf:
        stx.w   GameState.roomId

        stz.w   GameState.entranceId

        jmp     RoomChangeRequested
    NotDpad:


    lda.w   Controller.Joy2.pressed
    bit.b   #JOYL.x
    beq     +
        phd
        pea     Entity.playerEntity
        pld
        jsr     Entities.Players.Player.InvertGravity__a8iu
        phd
    +

    // Reset when L & R are pressed
    lda.w   Controller.Joy2.current
    and.b   #JOYL.l | JOYL.r
    cmp.b   #JOYL.l | JOYL.r
    bne     +
        lda.b   #Mode.TITLE_SCREEN
        sta.w   currentMode
    +
}


namespace DebugSave {

constant SNES_HEADER      = 0x80ffb0
constant SNES_HEADER_END  = 0x80ffdc
constant SNES_HEADER_SIZE = SNES_HEADER_END - SNES_HEADER

constant SaveRam.size = {CART_RAM_SIZE} * 1024 / 8
assert(SaveRam.size >= GameStateData.size * 2 + SNES_HEADER_SIZE)

if MEMORY_MAP.{MEMORY_MAP} == MEMORY_MAP.LOROM {
    constant SaveRam = 0x700000
    assert(SaveRam + SaveRam.size <= 0x708000)
} else {
    constant SaveRam = 0x306000
    assert(SaveRam + SaveRam.size <= 0x308000)
}

// Save RAM data format:
//  * A copy of the $ffb0 header, excluding the checksum
//  * 2 copies of `GameStateData`
constant SaveRam_header     = SaveRam
constant SaveRam_gamestate1 = SaveRam_header + SNES_HEADER_SIZE
constant SaveRam_gamestate2 = SaveRam_gamestate1 + GameStateData.size

constant SaveRamBank = SaveRam >> 16


// Save GameState data to Save-RAM.
// CAUTION: Must invoke `ConfirmSaveStateValid()` after the room is loaded.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro SaveGameState() {
    assert16a()
    assert16i()

    phb

    ldy.w   #DebugSave.SaveRam

    ldx.w   #DebugSave.SNES_HEADER
    lda.w   #DebugSave.SNES_HEADER_SIZE - 1
    mvn     DebugSave.SaveRamBank=0x80
// DB = SaveRamBank

    ldx.w   #GameStateData
    lda.w   #GameStateData.size - 1
    mvn     DebugSave.SaveRamBank=(GameStateData >> 16)

    ldx.w   #GameStateData
    lda.w   #GameStateData.size - 1
    mvn     DebugSave.SaveRamBank=(GameStateData >> 16)

    // Temporally invalidate save-RAM to break a BSOD reset loop
    // if loading a room causes a BRK BSOD.
    //
    // The gamestate is saved _before_ the room is loaded to ensure
    // a reset would result in an identical playing state
    // (as scripts/entities can modify the state on room load).
    inc.w   DebugSave.SaveRam_gamestate2

    plb
// DB = 0x7e
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro ConfirmGameStateValid() {
    assert16a()
    assert16i()

    // Undo the `SaveRam_gamestate2` modification
    // to mark the save-RAM as valid.
    lda.l   DebugSave.SaveRam_gamestate1
    sta.l   DebugSave.SaveRam_gamestate2
}


// Load GameState data to Save-RAM
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro LoadGameStateOrBranchIfNoSave(NoSaveLabel) {
    assert16a()
    assert16i()

    assert(DebugSave.SNES_HEADER_SIZE % 2 == 0)
    ldx.w   #DebugSave.SNES_HEADER_SIZE - 2
    HeaderLoop:
        lda.l   DebugSave.SaveRam_header,x
        cmp.l   DebugSave.SNES_HEADER,x
        bne     {NoSaveLabel}
        dex
        dex
        bpl     HeaderLoop

    // Verify the two gamestates copies match
    assert(GameStateData.size % 2 == 0)
    ldx.w   #GameStateData.size - 2
    GamestateLoop:
        lda.l   DebugSave.SaveRam_gamestate1,x
        cmp.l   DebugSave.SaveRam_gamestate2,x
        bne     {NoSaveLabel}
        dex
        dex
        bpl     GamestateLoop

    // Copy the save to GameStateData
    ldx.w   #DebugSave.SaveRam_gamestate1
    ldy.w   #GameStateData
    lda.w   #GameStateData.size - 1
    mvn     0x7e=DebugSave.SaveRamBank
}

}

