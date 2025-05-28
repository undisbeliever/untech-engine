// room/script-triggers.inc
// ====================----
//
// Room Script Triggers.
//
// SPDX-FileCopyrightText: © 2021 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2021 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Room {
namespace ScriptTriggers {


assert(Room.ScriptTriggers.N_SCRIPT_TRIGGERS == Room.N_SCRIPT_TRIGGERS)


// Load and verify the room script trigger header data.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _Load() {
    assert16a()
    assert16i()

    sep     #$10
i8()
    // Mark playerTileIndex and prevTriggerIndex unused
    assert(MetaTiles.MAP_HEIGHT_LARGE * MetaTiles.MAP_MAX_WIDTH < 0x8000)
    ldx.b   #0xff
    stx.w   ScriptTriggers.playerTileIndex + 1
    stx.w   ScriptTriggers.prevTriggerIndex


    // Clear ScriptTriggers variables
    ldx.b   #Format.Header.ScriptTriggers.LAST_INDEX
    ClearLoop:
        assert(ScriptTriggers.Triggers.top + 1 == ScriptTriggers.Triggers.bottom)
        stz.w   ScriptTriggers.Triggers.top,x
        dex
        dex
        bpl     ClearLoop


    sep     #$30
a8()
i8()

    // Find the last valid entry
    ldx.b   #0
    SizeLoop:
        assert(MetaTiles.MAP_HEIGHT_LARGE * MetaTiles.MAP_MAX_WIDTH < 0x8000)
        lda.w   Format.Header.ScriptTriggers.bottomRight + 1,x
        bmi     EndSizeLoop

        assert(Format.Header.ScriptTriggers.INDEX_INCREMENT == 2)
        inx
        inx

        cpx.b   #Format.Header.ScriptTriggers.LAST_INDEX + 2
        bcc     SizeLoop
EndSizeLoop:

    // X = index after the last script trigger
    dex
    dex
    stx.w   ScriptTriggers.lastTriggerIndex

    assert(Format.Header.ScriptTriggers.LAST_INDEX < 0x80)
    bmi     SkipCalcLoop


    // Calculate `Triggers.top` and `Triggers.bottom` values
    CalcLoop:
        lda.w   Format.Header.ScriptTriggers.topLeft,x
        and.w   MetaTiles.map.dataIndexYmask
        sta.w   ScriptTriggers.Triggers.top,x

        lda.w   Format.Header.ScriptTriggers.bottomRight,x
        dec
        and.w   MetaTiles.map.dataIndexYmask
        inc
        sta.w   ScriptTriggers.Triggers.bottom,x

        assert(Format.Header.ScriptTriggers.INDEX_INCREMENT == 2)
        dex
        dex
        bpl     CalcLoop
SkipCalcLoop:


    rep     #$30
a16()
i16()
}



// Process the script triggers.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _Process() {
    assert16a()
    assert16i()


    sep     #$10
i8()

    // Only test script triggers if the player touched a script-trigger tile.
    assert(MetaTiles.MAP_HEIGHT_LARGE * MetaTiles.MAP_MAX_WIDTH < 0x8000)
    lda.w   Room.ScriptTriggers.playerTileIndex
    bpl     PlayerTouchTrigger
        // Invalidate `prevTriggerIndex` if the player did not touch a script-trigger tile in this frame.
        ldx.b   #0xff
        stx.w   Room.ScriptTriggers.prevTriggerIndex
        bra     NoScriptTriggers
PlayerTouchTrigger:


    // Goto EndLoop if `prevTriggerIndex` valid and `playerTileIndex` inside the previous trigger.
    ldx.w   Room.ScriptTriggers.prevTriggerIndex
    bmi     NoPreviousTrigger
        cmp.w   Room.Format.Header.ScriptTriggers.topLeft,x
        bcc     DepartedPreviousTrigger

        cmp.w   Room.Format.Header.ScriptTriggers.bottomRight,x
        bcs     DepartedPreviousTrigger

            sep     #$20
        a8()
            tay

            and.w   MetaTiles.map.dataIndexYmask

            cmp.w   Room.ScriptTriggers.Triggers.top,x
            bcc     DepartedPreviousTrigger_a8

            cmp.w   Room.ScriptTriggers.Triggers.bottom,x
            bcc     EndLoop

        a8()
        DepartedPreviousTrigger_a8:
            tya
            // A = lastTriggerIndex

            rep     #$20
        a16()
        DepartedPreviousTrigger:
NoPreviousTrigger:


    ldx.w   Room.ScriptTriggers.lastTriggerIndex
    bmi     NoScriptTriggers

    Loop:
        cmp.w   Room.Format.Header.ScriptTriggers.topLeft,x
        bcc     NextTrigger

        cmp.w   Room.Format.Header.ScriptTriggers.bottomRight,x
        bcs     NextTrigger

            sep     #$20
        a8()
            tay

            and.w   MetaTiles.map.dataIndexYmask

            cmp.w   Room.ScriptTriggers.Triggers.top,x
            bcc     NextTrigger_a8

            cmp.w   Room.ScriptTriggers.Triggers.bottom,x
            bcs     NextTrigger_a8
                // Trigger found
                stx.w   Room.ScriptTriggers.prevTriggerIndex

                assert(Room.Format.Header.ScriptTriggers.script.ONCE_FLAG == 0x80)
                lda.w   Room.Format.Header.ScriptTriggers.script,x
                bmi     RunOnce
                    jsr     Scripting.ActivateScript

                    jmp     EndLoop


                RunOnce:
                    // Set the trigger's script value to an invalid scriptId so it can never be activated again.
                    // ::ANNOY no `sty addr,x` instruction::
                    tay
                    constant InvalidScriptId = Room.Format.Header.ScriptTriggers.script.SCRIPT_ID_MASK
                    assert(InvalidScriptId > Room.N_ROOM_SCRIPTS + 1)
                    assert(InvalidScriptId < 0x80)
                    lda.b   #InvalidScriptId
                    sta.w   Room.Format.Header.ScriptTriggers.script,x
                    tya


                    // A = ScriptTriggers.script
                    and.b   #~Room.Format.Header.ScriptTriggers.script.ONCE_FLAG
                    jsr     Scripting.ActivateScript

                    jmp     EndLoop

        a8()
        NextTrigger_a8:
            tya
            // A = lastTriggerIndex

            rep     #$20
        a16()

    NextTrigger:
        dex
        dex
        bpl     Loop


    // No triggers found, invalidate `prevTriggerIndex`
    ldx.b   #0xff
    stx.w   Room.ScriptTriggers.prevTriggerIndex


au()
i8()
EndLoop:
    // Mark playerTileIndex as unused
    assert(MetaTiles.MAP_HEIGHT_LARGE * MetaTiles.MAP_MAX_WIDTH < 0x8000)
    ldx.b   #0xff
    stx.w   ScriptTriggers.playerTileIndex + 1

NoScriptTriggers:
    rep     #$30
a16()
i16()
}



// Set playerTileIndex when the player has touched a script-trigger metaTile.
//
// This routine should be called by an InteractiveTiles function.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: Y = map data index
a16()
i16()
code()
function _SetPlayerTileIndex {
    // Only save playerTileIndex on the first trigger on every frame
    assert(MetaTiles.MAP_HEIGHT_LARGE * MetaTiles.MAP_MAX_WIDTH < 0x8000)
    ldx.w   Room.ScriptTriggers.playerTileIndex
    bpl     +
       sty.w    Room.ScriptTriggers.playerTileIndex
    +

    rts
}

}
}


// An interactive tile function that enables a script trigger when a player
// enters it.
//
// NOTE: As the engine will only preform one trigger check per frame, this code
//       assumes the interactive triggers are executed in this order:
//  * PlayerOriginCollision
//  * PlayerAirCollision
namespace InteractiveTiles {
namespace PlayerScriptTrigger {

constant EntityCollision = InteractiveTiles.NoTileInteraction.NullFunction
constant EntityAirCollision = InteractiveTiles.NoTileInteraction.NullFunction

constant PlayerLeftRightCollision = InteractiveTiles.NoTileInteraction.NullFunction

constant PlayerOriginCollision = Room.ScriptTriggers._SetPlayerTileIndex
constant PlayerAirCollision = Room.ScriptTriggers._SetPlayerTileIndex

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

