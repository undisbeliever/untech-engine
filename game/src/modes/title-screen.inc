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

namespace TitleScreen {

constant _shipHOffset = Scenes.TitleScreen.shipHOffset
constant _shipVOffset = Scenes.TitleScreen.shipVOffset



constant SHIP_TARGET_H_OFFSET = -64
constant SHIP_START_H_OFFSET = SHIP_TARGET_H_OFFSET - 64

constant SHIP_V_OFFSET = -112

constant ANIMATION_INTERVAL = 10


a16()
i16()
// DB = 0x7e
code()
function Run {
constant _frameCounter = zpTmp0
constant _frame = zpTmp1

    jsr     GameState.LoadInitialGameState


    jsr     FadeOut

    sep     #$20
a8()

    lda.b   #Song.TitleTheme
    jsr     Audio.LoadSong

    rep     #$30
a16()

    lda.w   #Project.Scenes.title_screen
    jsr     Resources.LoadScene
    jsr     Resources.TransferToPpu


    lda.w   #SHIP_V_OFFSET
    sta.w   _shipVOffset

    lda.w   #SHIP_START_H_OFFSET
    sta.w   _shipHOffset


    jsr     FadeIn


    ScrollLeft:
        jsr     WaitFrameAndCheckStart

        inc.w   _shipHOffset
        lda.w   _shipHOffset
        cmp.w   #SHIP_TARGET_H_OFFSET
        bcc     ScrollLeft


    lda.w   #10
    sta.b   _frameCounter

    lda.w   #0
    Shake:
        adc.w   #3

        pha
        and.w   #15
        clc
        adc.w   #SHIP_V_OFFSET - 8
        sta.w   _shipVOffset

        jsr     WaitFrameAndCheckStart

        pla

        dec.b   _frameCounter
        bpl     Shake


    sep     #$20
a8()
    lda.b   #SHIP_V_OFFSET
    sta.w   _shipVOffset

    stz.b   _frameCounter
    stz.b   _frame

    HelpLoop:
        dec.b   _frameCounter
        bpl     NoAnimation
            lda.b   #ANIMATION_INTERVAL - 1
            sta.b   _frameCounter

            lda.b   _frame
            inc
            cmp.b   #3
            bcc     +
                lda.b   #0
            +
            sta.b   _frame

            lsr
            rol.w   _shipHOffset + 1

            lsr
            rol.w   _shipVOffset + 1
        NoAnimation:

        jsr     WaitFrameAndCheckStart
        bra     HelpLoop
}


au()
iu()
// DB = 0x7e
code()
function WaitFrameAndCheckStart {
    php

    rep     #$20
a16()
    jsr     Resources.ProcessAnimations

    jsl     WaitFrame__far

    lda.w   Controller.Joy1.pressed
    bit.w   #JOY.start
    beq     +
        // Safe - SwitchMode will reset the stack pointer
        lda.w   #Mode.GAME
        jmp     SwitchMode
    +

    plp
    rts
}


}

