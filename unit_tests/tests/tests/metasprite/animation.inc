
scope MetaSpriteTest {
scope Animation {

allocateTmpWord(tmp0)
allocateTmpWord(tmp1)

code()
a16()
i16()
Test.add("MetaSprite.Animation.Process (STOP)")
scope Process_STOP: {
    lda.w   #MSEO.AnimationTest.Animations.Stop
    jsr     _Init
    bcc     Fail

    jsr     MetaSprite.Animation.Process

    // Test that stop sets the PC to NULL
    ldx.b   BaseEntity.MetaSprite.Animation.pc
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}

code()
a16()
i16()
Test.add("MetaSprite.Animation.Process (GOTO_ANIMATION)")
scope Process_GOTO_ANIMATION: {
constant tmp_pc(tmp0)

    lda.w   #MSEO.AnimationTest.Animations.GotoAnimation
    jsr     _Init
    bcc     Fail

    lda.b   BaseEntity.MetaSprite.Animation.pc
    sta     tmp_pc

    jsr     MetaSprite.Animation.Process

    // Tests that the Animation pc has changed
    lda.w   tmp_pc
    cmp.b   BaseEntity.MetaSprite.Animation.pc
    beq     Fail

    // Test the animation id changed
    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.Animation.id
    cmp.b   #MSEO.AnimationTest.Animations.SetFrameWaitFrames
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.Animation.Process (GOTO_OFFSET)")
scope Process_GOTO_OFFSET: {
    // The animation data contains many jumps that eventually
    // result in a jump to a STOP instruction

    lda.w   #MSEO.AnimationTest.Animations.GotoOffset
    jsr     _Init
    bcc     Fail

    jsr     MetaSprite.Animation.Process

    ldx.b   BaseEntity.MetaSprite.Animation.pc
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}


code()
a16()
i16()
Test.add("MetaSprite.Animation.Process (SET_FRAME_AND_WAIT_FRAMES)")
scope Process_SET_FRAME_AND_WAIT_FRAMES: {
constant tmp_frame(tmp0)

    lda.w   #MSEO.AnimationTest.Animations.SetFrameWaitFrames
    jsr     _Init
    bcc     Fail

    // Check that the first MS frame is displayed for 5 display frames

    jsr     MetaSprite.Animation.Process
    lda.b   BaseEntity.MetaSprite.currentFrame
    sta.w   tmp_frame

    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Animation.Process
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


code()
a16()
i16()
Test.add("MetaSprite.Animation.Process (SET_FRAME_AND_WAIT_TIME)")
scope Process_SET_FRAME_AND_WAIT_TIME: {
constant tmp_frame(tmp0)

    lda.w   #MSEO.AnimationTest.Animations.SetFrameWaitTime
    jsr     _Init
    bcc     Fail

    // Force NTSC timings
    lda.w   #5
    sta.w   MetaSprite.Animation.timerSpeed

    // Check that the first MS frame is displayed for 1/3 second

    jsr     MetaSprite.Animation.Process
    lda.b   BaseEntity.MetaSprite.currentFrame
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


code()
a16()
i16()
Test.add("MetaSprite.Animation.Process (SET_FRAME_AND_WAIT_*VECL)")
scope Process_SET_FRAME_AND_WAIT_VECL: {
constant tmp_frame(tmp0)
    // 79 *VECL parameter means >=2.5px travel

    lda.w   #MSEO.AnimationTest.Animations.SetFrameWaitVecl
    jsr     _Init
    bcc     Fail

    stz.b   BaseEntity.xVecl
    stz.b   BaseEntity.yVecl

    jsr     MetaSprite.Animation.Process

    macro test(vecl) {
        // Check that the frame will not advance on a 0 velocity
        lda.b   BaseEntity.MetaSprite.currentFrame
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

        // Increment the velocity and try again
        lda.w   #0x0080
        sta.b   BaseEntity.{vecl}

        jsr     MetaSprite.Animation.Process
        jsr     MetaSprite.Animation.Process
        jsr     MetaSprite.Animation.Process
        jsr     MetaSprite.Animation.Process

        // travelled 2.0 pixels, frame is the same
        lda.b   BaseEntity.MetaSprite.currentFrame
        cmp.w   tmp_frame
        bne     Fail

        jsr     MetaSprite.Animation.Process

        // travelled 2.5 pixels, frame should have changed
        lda.b   BaseEntity.MetaSprite.currentFrame
        cmp.w   tmp_frame
        beq     Fail
    }

    test(xVecl)
    test(yVecl)

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
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT:  A = Animation Id
// OUTPUT: DP = Entity address
// OUTPUT: C set on success
code()
a16()
i16()
scope _Init: {
constant tmp_animation(Test.tmp)

    sta.w   tmp_animation

    jsr     Dma.Init

    MetaSprite.Init()

    lda.w   #Entities.entity0
    tcd

    lda.w   #MSFS.FST_Animations
    ldy.w   #0

    MetaSprite._InitEntity()

    jsr     MetaSprite.Activate
    bcc     Fail

    lda.w   tmp_animation
    jsr     MetaSprite.SetAnimation
    bcc     Fail

    sec
    rts

Fail:
    clc
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

