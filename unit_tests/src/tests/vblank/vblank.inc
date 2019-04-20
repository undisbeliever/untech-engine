// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace VBlankTests {

a16()
i16()
code()
Test.add("WaitFrame (frameCounter)")
function WaitFrame_FrameCounter {
constant _oldFrameCounter = Test.tmp

    jsr     WaitFrame

    lda.w   frameCounter
    sta.w   _oldFrameCounter

    // Simulate 2 lag frames
    wai
    wai

    jsr     WaitFrame

    // 2 lag frames + 1 waitFrame = 3 frames

    lda.w   frameCounter
    sec
    sbc.w   _oldFrameCounter
    cmp.w   #3
    bne     Fail

    // Test frameCounter is incremented again
    // on next WaitFrame
    jsr     WaitFrame

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

    jsr     WaitFrame

    // Simulate lag frames
    wai
    jsr     WaitFrame

    // assert freeCycles is 0
    lda.w   freeCycles
    bne     Fail

    // non lag frame, freeCycles should be non-zero
    jsr     WaitFrame

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

    jsr     WaitFrame

    lda.w   frameCounter
    sta.w   _oldFrameCounter

    jsr     ForceBlank

    Dma.ForceBlank.AssertForceBlank()

    // test that ForceBlank called WaitFrame
    // (frameCounter is incremented by one)
    lda.w   frameCounter
    sec
    sbc.w   _oldFrameCounter
    cmp.w   #1
    bne     Fail

    jsr     ForceBlank

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

