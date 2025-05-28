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

namespace Scenes {

namespace UnitTests {
    a16()
    i8()
    code()
    function SetupPpu_dp2100 {
        rts
    }

    a16()
    i8()
    code()
    function Process {
        rts
    }

    a16()
    i8()
    farCode()
    function VBlank_dp2100 {
        rts
    }
}


namespace Mode1_Bg2MT_NoImage {
    a16()
    i8()
    code()
    function SetupPpu_dp2100 {
        rts
    }

    a16()
    i8()
    code()
    function Process {
        rts
    }

    constant VBlank_dp2100 = VBlank_dp2100_AnimatedTileset_Bg2
}


namespace SettingSun {
    constant time      = sceneVar0
    constant yPosition = sceneVar1

    constant SETTING_SUN_DELAY = 5
    constant Y_POS_LIMIT = -160

    // REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0x2100
    a16()
    i8()
    code()
    function SetupPpu_dp2100 {
        lda.w   #-1
        sta.w   time
        sta.w   yPosition

        rts
    }

    // REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
    a16()
    i8()
    code()
    function Process {
        dec.w   time
        bpl     +
            lda.w   yPosition
            dec
            cmp.w   #Y_POS_LIMIT
            bmi     +
                sta.w   yPosition

                lda.w   #SETTING_SUN_DELAY - 1
                sta.w   time
        +

        rts
    }

    // REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0x2100
    a16()
    i8()
    farCode()
    function VBlank_dp2100 {
        ldx.w   yPosition
        stx.b   BG2VOFS

        ldx.w   yPosition + 1
        stx.b   BG2VOFS

        jmp     VBlank_dp2100_AnimatedTileset_Bg1
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

