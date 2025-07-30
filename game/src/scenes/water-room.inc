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

namespace Scenes {
namespace WaterRoom {

constant waterPosition = Project.GameState.Words.waterPosition

constant waterHOffset   = sceneVar0
constant waterVOffset   = sceneVar1


constant WATER_LEVEL_V_OFFSET = 256
constant MAX_V_OFFSET         = 512 - Camera.DISPLAY_HEIGHT - 1


a16()
i8()
// DB = 0x7e
// DP = 0x2100
code()
function SetupPpu_dp2100 {
    // BG2, BG3 & sprites on main screen
    // BG1 on subscreen
    assert(TS == TM + 1)
    lda.w   #(0b10110) | (0b00001 << 8)
    sta.b   TM

    ldx.b   #CGWSEL.addSubscreen
    stx.b   CGWSEL

    ldx.b   #CGADSUB.halfColor | CGADSUB.enable.bg2 | CGADSUB.enable.backdrop
    stx.b   CGADSUB

    rts
}


a16()
i8()
// DB = 0x7e
code()
function Process {
    inc.w   waterHOffset

    lda.w   Camera.yPos
    clc
    adc.w   #WATER_LEVEL_V_OFFSET
    sec
    sbc.w   waterPosition
    bcs     +
        lda.w   #0
    +
    cmp.w   #MAX_V_OFFSET
    bcc     +
        lda.w   #MAX_V_OFFSET
    +
    sta.w   waterVOffset

    rts
}


a16()
i8()
// DB = 0x80
// DP = 0x2100
farCode()
function VBlank_dp2100 {
    Healthbar.VBlank_dp2100()

    ldy.w   waterHOffset
    sty.b   BG1HOFS
    ldy.b   #0
    sty.b   BG1HOFS

    ldy.w   waterVOffset
    sty.b   BG1VOFS
    ldy.w   waterVOffset + 1
    sty.b   BG1VOFS

    jmp     VBlank_dp2100_AnimatedTileset_Bg2
}


}
}

