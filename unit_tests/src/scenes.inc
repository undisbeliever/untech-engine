// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

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
    code()
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
    code()
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

