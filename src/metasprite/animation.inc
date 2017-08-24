// metasprite/animation.inc
// ========================
//
// MetaSprite Animation Routines.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {
namespace Animation {

// Initializes the MetaSprite Animation Module
//
// REQUITES: 8 bit A, 16 bit Index, DB = $7e
macro _Init() {
	assert8a()

    // Set animation timer speed
    function Animation {
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
}



// Sets the entity's animation.
//
// Will not reset the animation if the animation Id is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
// INPUT: A = animation Id
// OUTPUT: C set on success
a16()
i16()
code()
function SetAnimationId {
    ldx.b   BaseEntity.MetaSprite.frameSet
    beq     InvalidAnimation

    sep     #$20
a8()
    cmp.b   BaseEntity.MetaSprite.Animation.id
    beq     AnimationUnchanged

    cmp.l   MetaSprite.Format.FrameSet.nAnimations,x
    bcs     InvalidAnimation

    rep     #$30
a16()
    and.w   #0xff

    // Set animation.Id to A and animation.pos to 0
    assert(BaseEntity.MetaSprite.Animation.id + 1 == BaseEntity.MetaSprite.Animation.pos)
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

    sep     #$20
a8()
    lda.l   MetaSprite.Format.Animation.durationFormat,x
    sta.b   BaseEntity.MetaSprite.Animation.durationFormat

    // Upload frame on next MetaSprite.Animation.Process call
    lda.b   #0xff
    sta.b   BaseEntity.MetaSprite.Animation.nextFrameTime + 1


AnimationUnchanged:
    rep     #$30
a16()
    sec
    rts

InvalidAnimation:
    rep     #$31
a16()

DisableAnimation:
    // Disable animation
    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.id

    // carry clear
    rts
}



// Disables the entity's animation.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
constant DisableAnimation = SetAnimationId.DisableAnimation



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
        function Table {
            dw  Frame
            dw  Time
            dw  DistanceVertical
            dw  DistanceHorizontal

        constant size = pc() - Table
        }
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
            lda.b   BaseEntity.yVecl + 1
            bra     DistanceHorizontal.AfterLoadVecl
        }

        // nextFrameTime = distance entity travels horizontally (0:7:8 fixed point)
        function DistanceHorizontal {
            lda.b   BaseEntity.xVecl + 1
        AfterLoadVecl:
            bmi     +
                // negate if positive
                eor.w   #0xffff
                inc
            +

            // A = -abs(velocity)
            clc
            adc.b   BaseEntity.MetaSprite.Animation.nextFrameTime
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
//         X = MetaSprite Animation ptr
a8()
i16()
code()
function _GotoNextAnimation {
    lda.l   MetaSprite.Format.Animation.nextAnimation,x

    ldx.b   BaseEntity.MetaSprite.frameSet
    beq     InvalidAnimation

    cmp.l   MetaSprite.Format.FrameSet.nAnimations,x
    bcs     InvalidAnimation

    sta.b   BaseEntity.MetaSprite.Animation.id
    stz.b   BaseEntity.MetaSprite.Animation.pos


    rep     #$30
a16()
    // get new Animation ptr
    // A = new animation id
    // X = frameSet
    and.w   #0x00ff
    asl
    // c clear
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x
    tax

    sep     #$20
a8()
    lda.l   MetaSprite.Format.Animation.durationFormat,x
    sta.b   BaseEntity.MetaSprite.Animation.durationFormat

    bra     _LoadAnimationFrame


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

