// metasprite/animation.inc
// ========================
//
// MetaSprite Animation Routines.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


namespace MetaSprite {
namespace Animation {

// Initializes the MetaSprite Animation Module
//
// REQUITES: 8 bit A, 16 bit Index, DB = $7e
macro _Init() {
	assert8a()

    // Set animation timer speed
    lda.l   STAT78
    and.b   #STAT78.pal
    beq     +
        // pal
        lda.b   #6
        bra     ++
    +
        // ntsc
        lda.b   #5
    +

    sta.w   MetaSprite.Animation.timerSpeed
    stz.w   MetaSprite.Animation.timerSpeed + 1
}


// Loads animation data into the BaseEntity struct.
//
// NOTE: Does not set BaseEntity.MetaSprite.Animation.nextFrameTime
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp: entity
//         X: frameSet id
//         A: animation Id (must be valid)
//
// OUTPUT: X: animation data index
macro __LoadAnimationData() {
    assert16a()
    assert16i()

    // Set animation.Id to A and animation.pos to 0
    assert(BaseEntity.MetaSprite.Animation.id + 1 == BaseEntity.MetaSprite.Animation.pos)
    and.w   #0xff
    sta.b   BaseEntity.MetaSprite.Animation.id

    // get animation ptr
    // A = animationId
    // X = frameSet
    asl
    // c clear
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x
    tax

    // Set nextAnimation and durationFormat
    assert(MetaSprite.Format.Animation.nextAnimation + 1 == MetaSprite.Format.Animation.durationFormat)
    assert(BaseEntity.MetaSprite.Animation.nextAnimation + 1 == BaseEntity.MetaSprite.Animation.durationFormat)
    lda.l   MetaSprite.Format.Animation.nextAnimation,x
    sta.b   BaseEntity.MetaSprite.Animation.nextAnimation
}



// Sets the entity's animation.
//
// Will not reset the animation if the animation Id is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
// INPUT: Y = animation Id
a16()
i16()
code()
function SetAnimationId_Y {
    tya

Fallthrough:
}



// Sets the entity's animation.
//
// Will not reset the animation if the animation Id is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
// INPUT: A = animation Id
a16()
i16()
code()
function SetAnimationId {
assert(pc() == SetAnimationId_Y.Fallthrough)

    ldx.b   BaseEntity.MetaSprite.frameSet

    sep     #$20
a8()
    cmp.b   BaseEntity.MetaSprite.Animation.id
    beq     AnimationUnchanged

    cmp.l   MetaSprite.Format.FrameSet.nAnimations,x
    bcs     InvalidAnimation

    rep     #$30
a16()
    __LoadAnimationData()

    // Upload frame on next MetaSprite.Animation.Process call
    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime

    rts


a8()
AnimationUnchanged:
    rep     #$30
a16()
    rts

a8()
InvalidAnimation:
    rep     #$31

FallThrough:
}



// Disables the entity's animation.
//
// REQUIRES: 16 bit A
// INPUT: dp: Entity
a16()
iu()
code()
function DisableAnimation {
    assert(pc() == SetAnimationId.FallThrough)

    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.id

    rts
}



// Processes one display frame of the MetaSprite animation.
//
// ASSUMES: entity has a MetaSprite FrameSet if the animation is enabled
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: dp = Entity
a16()
i16()
code()
function Process {
    assert(BaseEntity.MetaSprite.Animation.durationFormat + 1 == BaseEntity.MetaSprite.Animation.id)

    lda.b   BaseEntity.MetaSprite.Animation.durationFormat
    cmp.w   #0xff00
    bcs     AnimationDisabled

    and.w   #MetaSprite.Format.AnimationDurationFormat._MASK
    tax

    lda.b   BaseEntity.MetaSprite.Animation.nextFrameTime
    bmi     LoadCurrentFrame

    jmp     (UpdateNextFrameTime.Table,x)


LoadCurrentFrame:
    // get animation ptr
    ldx.b   BaseEntity.MetaSprite.frameSet
    lda.b   BaseEntity.MetaSprite.Animation.id
    and.w   #0xff
    asl
    // c clear
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x
    tax

    sep     #$20
a8()
    bra     _LoadAnimationFrame


AnimationDisabled:
    rts


    // Decrement nextFrameTime
    //
    // REQUIRES: 16 bit A, 16 bit Index
    // INPUT: A = BaseEntity.MetaSprite.Animation.nextFrameTime
    // RETURN: `rts` if nextFrameTime > 0
    // RETURN: `ProcessNextFrame` animation frame is completed
    a16()
    i16()
    namespace UpdateNextFrameTime {
        Table:
            dw  Frame
            dw  Time
            dw  DistanceVertical
            dw  DistanceHorizontal
        constant Table.size = pc() - Table

        assertPowerOfTwo(Table.size)
        assert(MetaSprite.Format.AnimationDurationFormat._MASK + 2 == Table.size)

        // nextFrameTime = display frames to wait
        function Frame {
            dec
            beq     ProcessNextFrame

            sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime
            rts
        }

        // nextFrameTime = 1/300th of a second
        function Time {
            sec
            sbc.w   timerSpeed
            bcc     ProcessNextFrame

            sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime
            rts
        }

        // nextFrameTime = distance entity travels vertically (0:7:8 fixed point)
        function DistanceVertical {
            // Confirm entity momentum is a 24 bit value
            assert(BaseEntity.yMomentum.px - BaseEntity.xMomentum.px == 3)

            lda.b   BaseEntity.MetaSprite.Animation.nextFrameTime
            sec
            sbc.b   BaseEntity.yMomentum.px - 1
            bmi     ProcessNextFrame

            sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime

            rts
        }

        // nextFrameTime = distance entity travels horizontally (0:7:8 fixed point)
        function DistanceHorizontal {
            // Confirm entity momentum is a 24 bit value
            assert(BaseEntity.yMomentum.px - BaseEntity.xMomentum.px == 3)

            lda.b   BaseEntity.MetaSprite.Animation.nextFrameTime
            sec
            sbc.b   BaseEntity.xMomentum.px - 1
            bmi     ProcessNextFrame

            sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime

            rts
        }
    }


ProcessNextFrame:
    // get animation ptr
    ldx.b   BaseEntity.MetaSprite.frameSet
    lda.b   BaseEntity.MetaSprite.Animation.id
    and.w   #0xff
    asl
    // c clear
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x
    tax


    // Advance to the next frame, or if all frames are displayed, the next animation
    sep     #$20
a8()
    lda.b   BaseEntity.MetaSprite.Animation.pos
    inc
    inc
    cmp.l   MetaSprite.Format.Animation.frameTableSize,x
    bcs     _GotoNextAnimation

    sta.b   BaseEntity.MetaSprite.Animation.pos

    // _LoadAnimationFrame
End:
}



// Loads the animation frame specified by current animation position
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// MODIFIES: 16 bit A
//
// INPUT: dp = entity
//         X = MetaSprite Animation ptr
//         BaseEntity.MetaSprite.Animation.pos = animation position
a8()
i16()
code()
function _LoadAnimationFrame {
assert(Process.End == pc())

// nextFrameTime is invalid - use it as a tmp variable
constant _tmp = BaseEntity.MetaSprite.Animation.nextFrameTime

    stx.b   _tmp

    lda.b   BaseEntity.MetaSprite.Animation.pos
    rep     #$30
a16()
    and.w   #0xfe
    clc
    adc.b   _tmp
    tax


    // Load frameId and duration
    assert(MetaSprite.Format.Animation.Frames.frameId + 1 == MetaSprite.Format.Animation.Frames.duration)

    lda.l   MetaSprite.Format.Animation.Frames.frameId,x
    sta.b   _tmp

    jsr     MetaSprite.SetFrame
    bcc     UnableToSetFrame


    // Frame was loaded successfully
    // Convert duration to format used by Process.UpdateNextFrameTime

    lda.b   BaseEntity.MetaSprite.Animation.durationFormat
    and.w   #MetaSprite.Format.AnimationDurationFormat._MASK
    tax

    lda.b   _tmp + 1
    and.w   #0x00ff
    bne     +
        // duration must never be 0
        inc
    +

    // A = frame duration value
    jmp     (ConvertDurationTable,x)

function ConvertDurationTable {
    // 1:1
    dw  Frame

    // convert from 1/75ths a second to 1/300ths of a second
    dw  Time

    // convert from 0:3:5 fixed point to 1:7:8 fixed point
    dw  Distance
    dw  Distance

assert(pc() - ConvertDurationTable == Process.UpdateNextFrameTime.Table.size)
}

Distance:
    asl
Time:
    asl
    asl

Frame:
    sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime
    rts


UnableToSetFrame:
    // MetaSprite Frame was not loaded, try again next display frame.

    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime
    rts
}



// Advances to the next animation and loads the new animation's first frame.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// MODIFIES: 16 bit A
//
// INPUT: dp = entity
a8()
i16()
code()
function _GotoNextAnimation {
    ldx.b   BaseEntity.MetaSprite.frameSet

    lda.b   BaseEntity.MetaSprite.Animation.nextAnimation
    cmp.l   MetaSprite.Format.FrameSet.nAnimations,x
    bcs     InvalidAnimation

    rep     #$20
a16()

    __LoadAnimationData()

    sep     #$20
a8()
    bra     _LoadAnimationFrame


a8()
InvalidAnimation:
    // Disable animation
    lda.b   #0xff
    sta.b   BaseEntity.MetaSprite.Animation.id

    rep     #$30
a16()
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

