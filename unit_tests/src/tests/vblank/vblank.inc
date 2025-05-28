// SPDX-FileCopyrightText: © 2019 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2019 Marcus Rowe <undisbeliever@gmail.com>
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

namespace VBlankTests {

// Tests that NmiHandler does not modify register state
// and increments NmiHandler.counter
a16()
i16()
code()
Test.add("NmiHandler")
function NmiHandler {
allocateTmpWord(_oldStack)

    macro doTest(evaluate a, evaluate x, evaluate y, evaluate dp, evaluate s, evaluate db, evaluate p) {
        assert16a()
        assert16i()

        assert({s} >> 16 == 0x7e)
        assert({s} & 0xffff < 0x2000)
        assert({s} > Test.tmp + 8)
        assert({s} < Test.tmp + Test.TMP_SIZE)

        if {p} & 0x10 {
            // If i8 then ensure x & y are byte values
            assert({x} < 0x100)
            assert({y} < 0x100)
        }

        // Save stack
        tsa
        sta.l   _oldStack

        // Load values into registers
        rep     #$ff
    a16()
    i16()

        lda.w   #{dp}
        tcd
        lda.w   #{s}
        tas
        lda.w   #{a}
        ldx.w   #{x}
        ldy.w   #{y}

        pea     ({p} << 8) | {db}
        plb
        plp
    au()
    iu()

        // Wait for interrupt
        wai

        // Store DB, P and A and S in {s}
        php
        phb

        rep     #$ff
    a16()
    i16()
        pha

        tsa
        pha

        // Restore DB
        pea     0x7e7e
        plb
        plb

        // Restore old stack
        lda.w   _oldStack
        tas

        // Test registers match

        lda.w   {s} - 1     // P & DB
        cmp.w   #({p} << 8) | {db}
        bne     Fail{#}

        lda.w   {s} - 5     // S
        cmp.w   #{s} - 4
        bne     Fail{#}
        // Test S still inside Test.tmp
        cmp.w   #Test.tmp + 2
        bcc     Fail{#}

        tdc                 // DP
        cmp.w   #{dp}
        bne     Fail{#}

        cpy.w   #{y}        // Y
        bne     Fail{#}

        cpx.w   #{x}        // X
        bne     Fail{#}

        lda.w   {s} - 3     // A
        cmp.w   #{a}
        bne     Fail{#}

        bra     Pass{#}

        Fail{#}:
            jmp     Fail
    Pass{#}:
    }

    sep     #$20
a8()
    stz.w   NmiHandler.counter

    rep     #$30
a16()
i16()

    //     a,      x,      y,      dp,     s,             db,   p
    doTest(0x1234, 0x56,   0x78,   0x9abc, Test.tmp + 10, 0x77, 0x30)  // a8,  i8
    doTest(0xdef0, 0x11,   0x22,   0x3344, Test.tmp + 18, 0x42, 0x11)  // a16, i8
    doTest(0x5566, 0x7788, 0x99aa, 0xbbcc, Test.tmp + 20, 0x89, 0x22)  // a8,  i16
    doTest(0xddee, 0xff00, 0x1ee7, 0x7e57, Test.tmp + 28, 0xdd, 0x03)  // a16, i16

    // Confirm correct number of tests occurred
    sep     #$20
a8()
    lda.w   NmiHandler.counter
    cmp.b   #4
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("WaitFrame (frameCounter)")
function WaitFrame_FrameCounter {
constant _oldFrameCounter = Test.tmp

    jsl     WaitFrame__far

    lda.w   frameCounter
    sta.w   _oldFrameCounter

    // Simulate 2 lag frames
    wai
    wai

    jsl     WaitFrame__far

    // 2 lag frames + 1 waitFrame = 3 frames

    lda.w   frameCounter
    sec
    sbc.w   _oldFrameCounter
    cmp.w   #3
    bne     Fail

    // Test frameCounter is incremented again
    // on next WaitFrame
    jsl     WaitFrame__far

    lda.w   frameCounter
    sec
    sbc.w   _oldFrameCounter
    cmp.w   #4
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

a16()
i16()
code()
Test.add("WaitFrame (freeCycles 0 on lag frame)")
function WaitFrame_FreeCyclesZeroOnLagFrame {

    jsl     WaitFrame__far

    // Simulate lag frames
    wai
    jsl     WaitFrame__far

    // assert freeCycles is 0
    lda.w   freeCycles
    bne     Fail

    // non lag frame, freeCycles should be non-zero
    jsl     WaitFrame__far

    lda.w   freeCycles
    beq     Fail

    sec
    rts

Fail:
    clc
    rts
}


// Test that calling ForceBlank while already in force blank
// will return immediate and not call WaitFrame
a16()
i16()
code()
Test.add("ForceBlank (called twice)")
function ForceBlankTest_CallTwice {
constant _oldFrameCounter = Test.tmp

    jsl     WaitFrame__far

    lda.w   frameCounter
    sta.w   _oldFrameCounter

    jsl     ForceBlank__far

    Dma.ForceBlank.AssertForceBlank()

    // test that ForceBlank called WaitFrame
    // (frameCounter is incremented by one)
    lda.w   frameCounter
    sec
    sbc.w   _oldFrameCounter
    cmp.w   #1
    bne     Fail

    jsl     ForceBlank__far

    // test ForceBlank did not call WaitFrame
    // (frameCounter is not incremented)
    lda.w   frameCounter
    sec
    sbc.w   _oldFrameCounter
    cmp.w   #1
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

