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


namespace Healthbar {


constant MAX_HEALTH = 6

constant SHIELD_X = 1
constant SHIELD_Y = 1

// ::TODO draw shield tiles::
constant SHIELD_TILE = 0x48
constant NO_SHIELD_TILE = 0x4b

constant playerHealth = Project.GameState.Words.playerHealth
constant shieldBuffer = Text.buffer + (Text.Console.MARGIN_TOP + Healthbar.SHIELD_Y) * Text.Console.BUFFER_WIDTH + Text.Console.MARGIN_LEFT + Healthbar.SHIELD_X


// Health visible in the UI
allocate(uiHealth, wram7e, 1)


// The tile to write to VRAM on the next VBlank
// If this byte is 0, no tile is copied
// (byte)
allocate(vblank_healthTile, lowram, 1)

// The VRAM word address of the tile to write to VRAM on the next VBlank
// CAUTION: The lowest byte is reset to 0 in the VBlank code.
allocate(vblank_healthVramWaddr, lowram, 2)



// DB = 0x7e
macro Init() {
    assert16a()
    assert16i()

    Text.Console.SetCursor(Healthbar.SHIELD_X, Healthbar.SHIELD_Y - 1)
    Text.Console.PrintConstString("SHIELD")

    jsr     Healthbar.RedrawHealthBar
}


a16()
i16()
// DB = 0x7e
code()
function RedrawHealthBar {
    sep     #$30
a8()
i8()
    lda.w   playerHealth
    bpl     +
        lda.b   #0
    +
    cmp.b   #MAX_HEALTH
    bcc     +
        lda.b   #MAX_HEALTH
    +
    sta.w   playerHealth
    sta.w   uiHealth

    tay
    tax

    // Draw shield tiles
    beq     +
        lda.b   #SHIELD_TILE
        -
            dex
            sta.w   shieldBuffer,x
            bne     -
    +

    // Draw empty shield tiles
    cpy.b   #MAX_HEALTH
    bcs     +
        lda.b   #NO_SHIELD_TILE
        -
            sta.w   shieldBuffer,y
            iny
            cpy.b   #MAX_HEALTH
            bcc     -
    +


    stz.w   vblank_healthTile

    lda.w   Text.tilemapVramWaddr + 1
    sta.w   Healthbar.vblank_healthVramWaddr + 1


    rep     #$30
a16()
i16()
    rts
}



// Update the healthbar if the player's health has changed
//
// DB = 0x7e
macro Process() {
    assert8a()
    assert8i()

    // Validate health
    lda.w   Healthbar.playerHealth
    bpl     +
        lda.b   #0
    +
    cmp.b   #Healthbar.MAX_HEALTH + 1
    bcc     +
        lda.b   #Healthbar.MAX_HEALTH
    +
    sta.w   Healthbar.playerHealth


    // Do not update the GUI if GUI update is pending
    ldx.w   Healthbar.vblank_healthTile
    bne     Skip

        // Assumes `uiHealth < MAX_HEALTH`
        cmp.w   Healthbar.uiHealth
        beq     HealthUnchanged
            // Health changed
            bcc     Decrement
                // Health increased

                Entities.QueueSoundEffect(playerHeal)

                ldx.w   Healthbar.uiHealth
                inc.w   Healthbar.uiHealth

                lda.b   #Healthbar.SHIELD_TILE
                bra     EndIf

            Decrement:
                // Health decremented
                ldx.w   Healthbar.uiHealth
                dex
                stx.w   Healthbar.uiHealth

                lda.b   #Healthbar.NO_SHIELD_TILE
            EndIf:

            sta.w   Healthbar.shieldBuffer,x
            sta.w   Healthbar.vblank_healthTile

            txa
            clc
            adc.b   #(Text.Console.MARGIN_TOP + Healthbar.SHIELD_Y) * Text.Console.BUFFER_WIDTH + Text.Console.MARGIN_LEFT + Healthbar.SHIELD_X
            sta.w   Healthbar.vblank_healthVramWaddr
    HealthUnchanged:
Skip:
}



// DB = 0x80
// DP = $2100
macro VBlank_dp2100() {
    assert16a()
    assert8i()

    ldx.w   Healthbar.vblank_healthTile
    beq     +
        lda.w   Healthbar.vblank_healthVramWaddr
        sta.b   VMADD

        stx.b   VMDATAL

        // safe to override next byte after vblank_healthTile
        assert(Healthbar.vblank_healthTile + 1 == Healthbar.vblank_healthVramWaddr)
        stz.w   Healthbar.vblank_healthTile
    +
}

}

