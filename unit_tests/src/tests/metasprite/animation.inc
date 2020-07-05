// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace MetaSpriteTest {
namespace Animation {

allocateTmpWord(tmp0)
allocateTmpWord(tmp1)


a16()
i16()
code()
Test.add("MetaSprite.Animation.Process (Frame)")
function Process_Frame {
constant tmp_frame = tmp0

    lda.w   #MSEO.AnimationTest.Animations.Frame
    ldy.w   #MSEO.AnimationTest.Animations.Frame
    jsr     _Init
    bcc     Fail

    // Check that the first MS frame is displayed for 5 display frames

    jsr     MetaSprite.Animation.Process
    lda.b   BaseEntity.MetaSprite.currentFrame
    beq     Fail
    sta.w   tmp_frame

    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process

    // Check frame has not changed
    lda.b   BaseEntity.MetaSprite.currentFrame
    cmp.w   tmp_frame
    bne     Fail

    // Check this display frame changes the MS frame
    jsr     MetaSprite.Animation.Process
    lda.b   BaseEntity.MetaSprite.currentFrame
    cmp.w   tmp_frame
    beq     Fail

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("MetaSprite.Animation.Process (Time)")
function Process_Time {
constant tmp_frame = tmp0

    lda.w   #MSEO.AnimationTest.Animations.Time
    ldy.w   #MSEO.AnimationTest.Animations.Time
    jsr     _Init
    bcc     Fail

    // Force NTSC timings
    lda.w   #5
    sta.w   MetaSprite.Animation.timerSpeed

    // Check that the first MS frame is displayed for 1/3 second

    jsr     MetaSprite.Animation.Process
    lda.b   BaseEntity.MetaSprite.currentFrame
    beq     Fail
    sta.w   tmp_frame

    ldx.w   #25 * 60 / 75
    Loop:
        phx
        jsr     MetaSprite.Animation.Process
        plx
        dex
        bne     Loop

    // Check this display frame has not changed
    lda.b   BaseEntity.MetaSprite.currentFrame
    cmp.w   tmp_frame
    bne     Fail

    jsr     MetaSprite.Animation.Process

    // Check this display frame has changed
    lda.b   BaseEntity.MetaSprite.currentFrame
    cmp.w   tmp_frame
    beq     Fail

    sec
    rts

Fail:
    clc
    rts
}


inline DistanceTest(animation, axis) {
    a16()
    i16()
    code()
    Test.add("MetaSprite.Animation.Process ({animation})")
    function Process_{animation} {
    constant tmp_frame = tmp0
        // 79 * MOMENTUM parameter means >=2.5px travel

        lda.w   #MSEO.AnimationTest.Animations.{animation}
        ldy.w   #MSEO.AnimationTest.Animations.{animation}
        jsr     _Init
        bcc     Fail

        // Clear X and Y momentum
        assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)
        stz.b   BaseEntity.xMomentum.sx
        stz.b   BaseEntity.xMomentum.sx + 2
        stz.b   BaseEntity.xMomentum.sx + 4

        jsr     MetaSprite.Animation.Process

        // Check that the frame will not advance on a 0 momentum
        lda.b   BaseEntity.MetaSprite.currentFrame
        beq     Fail
        sta.w   tmp_frame

        ldx.w   #30
        -
            phx
                jsr     MetaSprite.Animation.Process
            plx
            dex
            bne     -

        lda.b   BaseEntity.MetaSprite.currentFrame
        cmp.w   tmp_frame
        bne     Fail

        // Set momentum to 0.5 pixels/frame
        lda.w   #0x8000
        sta.b   BaseEntity.{axis}Momentum.sx

        jsr     MetaSprite.Animation.Process
        jsr     MetaSprite.Animation.Process
        jsr     MetaSprite.Animation.Process
        jsr     MetaSprite.Animation.Process

        // travelled 2.0 pixels, frame has not changed
        lda.b   BaseEntity.MetaSprite.currentFrame
        cmp.w   tmp_frame
        bne     Fail

        jsr     MetaSprite.Animation.Process

        // travelled 2.5 pixels, frame should have changed
        lda.b   BaseEntity.MetaSprite.currentFrame
        cmp.w   tmp_frame
        beq     Fail

        sec
        rts

    Fail:
        clc
        rts
    }
}
DistanceTest(DistanceVertical, y)
DistanceTest(DistanceHorizontal, x)


a16()
i16()
code()
Test.add("MetaSprite.Animation.Process (OneShot)")
function Process_OneShot {
    lda.w   #MSEO.AnimationTest.Animations.OneShot
    ldy.w   #0xff
    jsr     _Init
    bcc     Fail

    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process

    // Test that there is no animation
    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.Animation.id
    cmp.b   #0xff
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
Test.add("MetaSprite.Animation.Process (Change Animation)")
function Process_ChangeAnimation {
    // Change animation contains 4 single frames, then changes to
    // ChangeAnimation2.

    lda.w   #MSEO.AnimationTest.Animations.ChangeAnimation
    ldy.w   #MSEO.AnimationTest.Animations.ChangeAnimation2
    jsr     _Init
    bcc     Fail

    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process

    // Test that the animation id is still ChangeAnimation
    lda.b   BaseEntity.MetaSprite.Animation.id
    and.w   #0xff
    cmp.w   #MSEO.AnimationTest.Animations.ChangeAnimation
    bne     Fail

    jsr     MetaSprite.Animation.Process

    // Test the animation id changed
    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.Animation.id
    cmp.b   #MSEO.AnimationTest.Animations.ChangeAnimation2
    bne     Fail

    // Test that nextAnimation changed (ie, set to 0xff)
    lda.b   BaseEntity.MetaSprite.Animation.nextAnimation
    cmp.b   #0xff
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
Test.add("BaseEntity.MetaSprite.Animation.nextAnimation")
function Change_nextAnimation {
    // Change animation contains 4 single frames, then changes to
    // ChangeAnimation2.

    lda.w   #MSEO.AnimationTest.Animations.ChangeAnimation
    ldy.w   #MSEO.AnimationTest.Animations.ChangeAnimation2
    jsr     _Init
    bcc     Fail

    // Change nextAnimation to Time Animation
    sep     #$20
a8()
    lda.b   #MSEO.AnimationTest.Animations.Time
    sta.b   BaseEntity.MetaSprite.Animation.nextAnimation

    rep     #$30
a16()

    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process

    // Test that the animation id is still ChangeAnimation
    lda.b   BaseEntity.MetaSprite.Animation.id
    and.w   #0xff
    cmp.w   #MSEO.AnimationTest.Animations.ChangeAnimation
    bne     Fail

    jsr     MetaSprite.Animation.Process

    // Test the animation id changed to Time
    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.Animation.id
    cmp.b   #MSEO.AnimationTest.Animations.Time
    bne     Fail

    // Test that nextAnimation changed to Time
    lda.b   BaseEntity.MetaSprite.Animation.nextAnimation
    cmp.b   #MSEO.AnimationTest.Animations.Time
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}


// Preps the test
//
//  * Loads the Entity
//  * Sets the animation
//  * Tests that nextAnimation is set correctly
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT:  A = Animation Id
//         Y = expected nextAnimation
// OUTPUT: DP = Entity address
// OUTPUT: C set on success
a16()
i16()
code()
function _Init {
constant tmp_animation     = Test.tmp
constant tmp_nextAnimation = Test.tmp + 2

    sta.w   tmp_animation
    sty.w   tmp_nextAnimation

    jsr     _Init_MetaSpriteModule

    lda.w   #EntityPool.entity0
    tcd

    lda.w   #MSFS.FST_Animations
    ldy.w   #0

    MetaSprite._InitEntity()

    jsr     MetaSprite.Activate
    bcc     Fail

    lda.w   tmp_animation
    jsr     MetaSprite.Animation.SetAnimationId

    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.Animation.id
    cmp.w   tmp_animation
    bne     Fail

    lda.b   BaseEntity.MetaSprite.Animation.nextAnimation
    cmp.w   tmp_nextAnimation
    bne     Fail

    rep     #$30
a16()

    sec
    rts

Fail:
    rep     #$31
    // c clear
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

