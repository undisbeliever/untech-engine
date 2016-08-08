
// ::MAYDO catch infinite loops::
// ::: not sure if it worth the cost::

scope MetaSprite {
scope Animation {

// Sets the animation speed
//
// REQUITES: 8 bit A, 16 bit Index, DB = $7e
macro _Init() {
	assert8a()

    scope Animation: {
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
// Does not reset the animation pc if the animation Id is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
// INPUT: A = animation Id
// OUTPUT: C set on success
code()
a16()
i16()
scope SetAnimationId: {
    ldx.b   BaseEntity.MetaSprite.frameSet
    beq     ReturnFalse

    sep     #$20
a8()
    cmp.b   BaseEntity.MetaSprite.Animation.id
    beq     AnimationUnchanged

    cmp.l   MetaSprite.Format.FrameSet.nAnimations,x
    bcs     ReturnFalse

    sta.b   BaseEntity.MetaSprite.Animation.id

    rep     #$20
a16()
    and.w   #0x00ff
    asl
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x

    sta.b   BaseEntity.MetaSprite.Animation.pc

    // reset Animation delay
    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.delay

    sec
    rts

ReturnFalse:
    rep     #$31
a16()
    // Set Animation PC to NULL
    stz.b   BaseEntity.MetaSprite.Animation.pc

    // carry clear
    rts

AnimationUnchanged:
    rep     #$30
a16()
    sec
    rts
}



// Processes one display frame of the Entity's animation.
//
// ASSUMES: Entity has a frameSet
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: dp = Entity
code()
a16()
i16()
scope Process: {
    ldx.b   BaseEntity.MetaSprite.Animation.pc
    beq     Return

    txy
    lda.l   Data.AnimationBytecodeOffset,x
    and.w   #Bytecode.MASK
    tax
    jmp     (Bytecode.ResumeTable,x)

Continue:
    txy
    lda.l   Data.AnimationBytecodeOffset,x
    and.w   #Bytecode.MASK
    tax
    jmp     (Bytecode.StartTable,x)

Return:
    rts
}


// Bytecode is handled differently to normal execution
//
// These routines can exit by:
//   * jumping to Process.Continue if continuing next instruction
//   * RTS if instruction is in the wait state
scope Bytecode {

// Have two tables for speed purposes.
// Most of the time the instruction starts because the previous
// one finished.
// By having a start/resume table we save save CPU time not checking
// if the metaspite frame was set correctly.

scope StartTable: {
    dw  Stop
    dw  GotoStart
    dw  GotoAnimation
    dw  GotoOffset
    dw  SetFrameAndWait_Frame_Start
    dw  SetFrameAndWait_Time_Start
    dw  SetFrameAndWait_Vecl_Start
    dw  SetFrameAndWait_Vecl_Start

constant size(pc() - StartTable)
}
scope ResumeTable: {
    dw  Stop
    dw  GotoStart
    dw  GotoAnimation
    dw  GotoOffset
    dw  SetFrameAndWait_Frame_Continue
    dw  SetFrameAndWait_Time_Continue
    dw  SetFrameAndWait_xVecl_Continue
    dw  SetFrameAndWait_yVecl_Continue

constant size(pc() - ResumeTable)
}
// Ensure that table contains a power of 2 addresses
assert(ResumeTable.size & (ResumeTable.size - 1) == 0)
assert(StartTable.size == ResumeTable.size)
constant MASK(ResumeTable.size - 2)


// SET FRAME AND WAIT INSTRUCTIONS
// ===============================



// Sets the metasprite frame and waits n frames
// bytecode param 1: (uint8) frame Id
// bytecode param 2: (uint8) number of frames to wait
//
// Routine set Animation.delay to:
//      to 0xffff is cannot load the frame
//      to number of frames to wait on success
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// RETURN: `RTS`
code()
a16()
i16()
scope SetFrameAndWait_Frame_Start: {
    tyx
    lda.l   Data.AnimationBytecodeOffset + 1,x      // frame Id

    jsr     MetaSprite.SetFrame
    bcc     Failure

    ldx.b   BaseEntity.MetaSprite.Animation.pc
    lda.l   Data.AnimationBytecodeOffset + 2,x      // frames to wait
    and.w   #0x00ff
    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts

Failure:
    // Could Not set frame
    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts
}


// Continues a SET_FRAME_AND_WAIT_FRAMES instruction
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// OUTPUT: Y = animation PC
// RETURN: `_SetFrameAndWait_NextInstruction` if wait is over
// RETURN: `RTS` if still waiting
code()
a16()
i16()
scope SetFrameAndWait_Frame_Continue: {
    lda.b   BaseEntity.MetaSprite.Animation.delay
    bmi     SetFrameAndWait_Frame_Start

    beq     _SetFrameAndWait_NextInstruction
    dec
    beq     _SetFrameAndWait_NextInstruction

    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts
}


// Goes to the next Instruction after a set frame and wait instruction
//
// ASSUMES: all set frame and wait instructions are 3 bytes long
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// IN: Y = animation.pc
// OUT: new pc (old PC + 3)
// RETURN: Process.Continue
code()
a16()
i16()
scope _SetFrameAndWait_NextInstruction: {
    tyx
    inx
    inx
    inx
    stx.b   BaseEntity.MetaSprite.Animation.pc
    bra     Process.Continue
}


// Sets the metasprite frame and waits a given amount of time
// bytecode param 1: (uint8) frame Id
// bytecode param 2: (uint8) time to wait (1/75th of a second)
//
// Routine set Animation.delay to:
//      to 0xffff is cannot load the frame
//      to time to wait (in 1/300ths of a second)
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// RETURN: `RTS`
code()
a16()
i16()
scope SetFrameAndWait_Time_Start: {
    tyx
    lda.l   Data.AnimationBytecodeOffset + 1,x      // frame Id

    jsr     MetaSprite.SetFrame
    bcc     Failure

    ldx.b   BaseEntity.MetaSprite.Animation.pc
    lda.l   Data.AnimationBytecodeOffset + 2,x      // time to wait

    // convert from 1/75ths a second to 1/300ths of a second
    and.w   #0x00ff
    asl
    asl
    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts

Failure:
    // Could Not set frame
    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts
}


// Continues a SET_FRAME_AND_WAIT_TIME instruction
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// OUTPUT: Y = animation PC
// RETURN: `_SetFrameAndWait_NextInstruction` if wait is over
// RETURN: `RTS` if still waiting
code()
a16()
i16()
scope SetFrameAndWait_Time_Continue: {
    lda.b   BaseEntity.MetaSprite.Animation.delay
    bmi     SetFrameAndWait_Time_Start

    sec
    sbc     timerSpeed
    bcc     _SetFrameAndWait_NextInstruction

    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts
}



// Sets the metasprite frame and waits until entity has moved N pixels
//
// bytecode param 1: (uint8) frame Id
// bytecode param 2: (0:3:5 fixed point) distance to wait
//
// Routine set Animation.delay to:
//      to 0xffff is cannot load the frame
//      to distance on success
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// RETURN: `RTS`
code()
a16()
i16()
scope SetFrameAndWait_Vecl_Start: {
    tyx
    lda.l   Data.AnimationBytecodeOffset + 1,x      // frame Id

    jsr     MetaSprite.SetFrame
    bcc     Failure

    ldx.b   BaseEntity.MetaSprite.Animation.pc
    lda.l   Data.AnimationBytecodeOffset + 2,x      // distance to wait
    and.w   #0x00ff
    // convert from 0:3:5 to 1:7:8 fixed point
    asl
    asl
    asl
    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts

Failure:
    // Could Not set frame
    lda.w   #0xffff
    sta.b   BaseEntity.MetaSprite.Animation.delay
    rts
}


// Continues a SET_FRAME_AND_WAIT_XVECL instruction
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// OUTPUT: Y = animation PC
// RETURN: `_SetFrameAndWait_NextInstruction` if wait is over
// RETURN: `RTS` if still waiting
code()
a16()
i16()
scope SetFrameAndWait_xVecl_Continue: {
    // if delay & 0x8000
    //     goto SetFrameAndWait_Vecl_Start
    //
    // else:
    //     t = delay - abs(xVecl)
    //     if t < 0:
    //         goto next instruction
    //     else:
    //         delay = t
    //         wait

    lda.b   BaseEntity.MetaSprite.Animation.delay
    bmi     SetFrameAndWait_Vecl_Start

    lda.b   BaseEntity.xVecl.hw
    bmi     +
        // invert if positive
        eor.w   #0xffff
        inc
    +

    clc
    adc.b   BaseEntity.MetaSprite.Animation.delay
    bmi     _SetFrameAndWait_NextInstruction

    sta.b   BaseEntity.MetaSprite.Animation.delay

    rts
}


// Continues a SET_FRAME_AND_WAIT_YVECL instruction
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// OUTPUT: Y = animation PC
// RETURN: `_SetFrameAndWait_NextInstruction` if wait is over
// RETURN: `RTS` if still waiting
code()
a16()
i16()
scope SetFrameAndWait_yVecl_Continue: {
    lda.b   BaseEntity.MetaSprite.Animation.delay
    bmi     SetFrameAndWait_Vecl_Start

    lda.b   BaseEntity.yVecl.hw
    bmi     +
        eor.w   #0xffff
        inc
    +
    clc
    adc.b   BaseEntity.MetaSprite.Animation.delay
    bmi     _SetFrameAndWait_NextInstruction

    sta.b   BaseEntity.MetaSprite.Animation.delay

    rts
}


// CONTROL INSTRUCTIONS
// ====================

// Stop Bytecode interpreter
// REQUIRES: 16 bit A, 16 bit I, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// RETURN: RTS
code()
a16()
i16()
scope Stop: {
    stz.b   BaseEntity.MetaSprite.Animation.pc
    rts
}


// Jumps to the start of the animation
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// OUTPUT: X = new animation PC
// RETURN: `Process.Continue`
code()
a16()
i16()
scope GotoStart: {
    ldx.b   BaseEntity.MetaSprite.frameSet

    lda.b   BaseEntity.MetaSprite.Animation.id
    and.w   #0x00ff
    asl
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x

    sta.b   BaseEntity.MetaSprite.Animation.pc
    tax
    jmp     Process.Continue
}


// Jumps to a given animation id
// bytecode param: (uint8) animation Id
//
// Stops the interpreter if animation id is current or invalid.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// RETURN: `Process.Continue` (X = new animation PC)
// RETURN: `Bytecode.Stop` if animationId is invalid
code()
a16()
i16()
scope GotoAnimation: {
    tyx
    lda.l   Data.AnimationBytecodeOffset + 1,x  // animation id

    ldx.b   BaseEntity.MetaSprite.frameSet
    beq     Bytecode.Stop

    sep     #$20
a8()
    cmp.b   BaseEntity.MetaSprite.Animation.id
    beq     Bytecode.Stop

    cmp.l   MetaSprite.Format.FrameSet.nAnimations,x
    bcs     Bytecode.Stop

    sta.b   BaseEntity.MetaSprite.Animation.id

    rep     #$20
a16()
    and.w   #0x00ff
    asl
    adc.l   MetaSprite.Format.FrameSet.animationTable,x
    tax
    lda.l   MetaSprite.Data.AnimationList,x

    sta.b   BaseEntity.MetaSprite.Animation.pc
    beq     Bytecode.Stop

    tax
    jmp     Process.Continue
}



// Jumps to a given offset
// bytecode param: (int_ms8) byte offset
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = Entity
// INPUT:  Y = animation PC
// OUTPUT: X = new animation PC
// RETURN: `Process.Continue`
code()
a16()
i16()
scope GotoOffset: {
    tyx
    lda.l   Data.AnimationBytecodeOffset + 1,x

    // A int_ms8 format

    and.w   #0x00ff
    clc
    adc.b   BaseEntity.MetaSprite.Animation.pc
    sec
    sbc.w   #INT_MS8_OFFSET

    sta.b   BaseEntity.MetaSprite.Animation.pc

    tax
    jmp     Process.Continue
}

}
}
}

// vim: ft=asm ts=4 sw=4 et:

