// scripting/scripts.inc
// =====================
//
// Script activation and processing routines.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Scripting {


// Initialize and reset the scripting subsystem
//
// Called when a room is loaded
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
macro _Init() {
    assert16a()
    assert8i()

    // Reset scripting variables to 0

    constant VariablesSize = Scripting.__VariablesEnd - Scripting.__VariablesBegin

    assert(VariablesSize < 0xff)
    assert(VariablesSize % 2 == 0)
    ldx.b   #VariablesSize
    Loop:
        dex
        dex
        stz.w   Scripting.currentScriptIndex,x
        bne     Loop
}



// Activate a room script.
//
// This routine will do nothing if the *script Id* is invalid or the
// script is already active.
//
// REQUIRES: DB = 0x7e
//
// INPUT: A - script id
au()
iu()
code()
function ActivateScript {
    // NOTE: DP is unknown

    php
    sep     #$30
a8()
i8()
    // Check script id is valid
    cmp.b   #Room.N_ROOM_SCRIPTS
    bcs     Skip

        // Check if the script is active
        tay
        ldx.w   activeScripts,y
        bne     Skip

            // Sanity check
            ldx.w   activeScriptsEnd
            cpx.b   #MAX_ACTIVE_SCRIPTS * 2
            bcs     Skip

                // X = script index
                // A = scriptId
                // Y = scriptId

                sta.w   scripts.scriptId,x

                // Mark script active
                assert(MAX_ACTIVE_SCRIPTS < 0x80)
                inc
                // A is non-zero
                sta.w   activeScripts,y

                asl
                tay
                // Y = index into room scripts list + 2

                // Set script resume function
                lda.b   #SpecialResumeOpcodes.Process_Script
                sta.w   scripts.resumeOpcode,x

                rep     #$20
            a16()
                // Set Program Counter
                // (The minus two offsets `inc` above)
                lda.w   Room.Format.Header.scripts - 2,y
                sta.w   scripts.programCounter,x

                inx
                inx
                stx.w   activeScriptsEnd

au()
Skip:
    plp
    rts
}



// Execute all active scripts.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e
a8()
i16()
code()
function ProcessScripts {
    sep     #$30
a8()
i8()

    ldy.w   activeScriptsEnd
    beq     Return

    Loop:
        dey
    Loop_AfterDEY:
        dey
        sty.w   currentScriptIndex

        ldx.w   scripts.resumeOpcode,y

        rep     #$10
    i16()
        jmp     (BytecodeResumeFunctionTable,x)

        // Execution will eventually jump to NextScript_Y, NextScript_Yunknown or RemoveScript


au()
iu()
NextScript_Y:
    // Y = currentScriptIndex
        sep     #$30
    i8()
    a8()
        assert(MAX_ACTIVE_SCRIPTS * 2 < 0x80)
        dey
        // If currentScriptIndex is 0 this is negative
        bpl     Loop_AfterDEY

Return:
    rep     #$30
a16()
i16()
    rts


au()
iu()
NextScript_Yunknown:
    // Y = unknown
        sep     #$30
    i8()
    a8()
        ldy.w   currentScriptIndex
        bne     Loop


    rep     #$30
a16()
i16()
    rts


au()
iu()
RemoveScript:
    // The `End_Script` bytecode instruction jumps here
        sep     #$30
    i8()
    a8()
        ldy.w   currentScriptIndex


        // Mark script as deactivated
        ldx.w   scripts.scriptId,y
        stz.w   activeScripts,x


        // Remove script data from scripts SoA by moving the last script to currentScriptIndex
        // (This is safe as we are iterating backwards)

        // activeScriptsEnd is non-zero
        ldx.w   activeScriptsEnd
        dex
        dex
        stx.w   activeScriptsEnd

        // X = source
        // Y = target
        rep     #$20
    a16()
        assert(scripts.resumeOpcode + 1 == scripts.scriptId)
        lda.w   scripts.resumeOpcode,x
        sta.w   scripts.resumeOpcode,y

        lda.w   scripts.programCounter,x
        sta.w   scripts.programCounter,x

        lda.w   scripts.resumeArg1,x
        sta.w   scripts.resumeArg1,y

        lda.w   scripts.resumeArg2,x
        sta.w   scripts.resumeArg2,y

        sep     #$30
    a8()

        // Process the next script
        assert(MAX_ACTIVE_SCRIPTS * 2 < 0x80)
        dey
        // If currentScriptIndex is 0 this is negative
        bpl     Loop_AfterDEY


    rep     #$30
a16()
    rts
}



// Bytecode Instruction control
// ============================


// Stops the script
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
constant Bytecode.End_Script = Scripting.ProcessScripts.RemoveScript



// Used to indicate a script is not active.
// Execution should never enter reach this resume instruction.
constant Bytecode.End_Script___Resume = ProcessScripts.RemoveScript




// Reads the opcode byte from the script data and executes it.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: Y - program counter
macro __ProcessNextOpcode() {
    assert8a()
    assert16i()

    // Clear high byte of A
    // REQUIRED to allow a `tax` to work in 8 bit A inside a bytecode instruction
    tdc

    lda.w   scriptData,y
    iny

    assert(Project.BytecodeFunctionTable.size == 256)
    asl
    tax
    jmp     (Project.BytecodeFunctionTable,x)
}



// Execute the next bytecode instruction at the current script's Program Counter.
//
// MUST ONLY BE EXECUTED by a bytecode instruction (see `BytecodeFunctionTable`)
//
// See `Bytecode.GotoNextOpcode` for more details.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e
// INPUT: programCounter - programCounter of script
a8()
i16()
code()
function Bytecode.GotoNextOpcode_LoadPC {
    ldy.b   programCounter

Fallthrough:
}



// Execute the next bytecode instruction at the current script's Program Counter
//
// MUST ONLY BE EXECUTED by a bytecode instruction (see `BytecodeFunctionTable`)
//
// The program counter (Y) must point to the index (within `scriptData`) of the
// next instruction to execute.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e
// INPUT: Y - program counter
a8()
i16()
code()
function Bytecode.GotoNextOpcode {
    assert(pc() == Bytecode.GotoNextOpcode_LoadPC.Fallthrough)

    __ProcessNextOpcode()
}



// Yielding Instructions control
// =============================


// A generic yield instruction that does not consume any arguments puts the current script to sleep.
//
// This routine will also reset `resumeArg1` and `resumeArg2` to 0.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
// You can also `constant` declare a bytecode instruction as this function (see `Sleep_AnimationTicks___ImmU16`).
//
// NOTE: The program counter MUST point to the start of the next instruction.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X - resume instruction opcode (index into `BytecodeFunctionTable` or `BytecodeResumeFunctionTable`)
// INPUT: Y - script program counter
a8()
i16()
code()
function YieldInstruction__NoArg {
    // ::ANNOY no `sty addr,x` instruction::

    // Bytecode instructions and Resume instructions share opcodes
    txa

    ldx.w   currentScriptIndex

    sta.w   scripts.resumeOpcode,x

    rep     #$30
a16()
    tya
    sta.w   scripts.programCounter,x

    stz.w   scripts.resumeArg1,x
    stz.w   scripts.resumeArg2,x


    sep     #$30
a8()
i8()
    txy
    jmp     ProcessScripts.NextScript_Y
}



// A generic yield instruction that consumes two bytes of instruction arguments puts the current script to sleep.
//
// The instruction argument will be stored in `resumeArg1`. `resumeArg2` will be set to 0.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
// You can also `constant` declare a bytecode instruction as this function (see `Sleep_AnimationTicks___ImmU16`).
//
// NOTE: The next instruction MUST start 2 bytes after the program counter.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X - resume instruction opcode (index into `BytecodeFunctionTable` or `BytecodeResumeFunctionTable`)
// INPUT: Y - script program counter at instruction arguments
a8()
i16()
code()
function YieldInstruction__2ByteArg {
    // (function suffix does not contain "word" to prevent confusion with the GameState Word arg type)

    // ::ANNOY no `sty addr,x` instruction::

    // Bytecode instructions and Resume instructions share opcodes
    txa

    ldx.w   currentScriptIndex

    sta.w   scripts.resumeOpcode,x

    rep     #$31
a16()
    tya
    // carry clear
    adc.w   #2
    sta.w   scripts.programCounter,x

    // first argument - word value
    lda.w   scriptData,y
    sta.w   scripts.resumeArg1,x

    stz.w   scripts.resumeArg2,x


    sep     #$30
a8()
i8()
    txy
    jmp     ProcessScripts.NextScript_Y
}



// A generic yield instruction that consumes four bytes of instruction arguments puts the current script to sleep.
//
// The instruction argument will be stored in `resumeArg1` and `resumeArg2`.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
// You can also `constant` declare a bytecode instruction as this function (see `Sleep_AnimationTicks___ImmU16`).
//
// NOTE: The next instruction MUST start 4 bytes after the program counter.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X - resume instruction opcode (index into `BytecodeFunctionTable` or `BytecodeResumeFunctionTable`)
// INPUT: Y - script program counter at instruction arguments
a8()
i16()
code()
function YieldInstruction__4ByteArg {
    // (function suffix does not contain "word" to prevent confusion with the GameState Word arg type)

    // ::ANNOY no `sty addr,x` instruction::

    // Bytecode instructions and Resume instructions share opcodes
    txa

    ldx.w   currentScriptIndex

    sta.w   scripts.resumeOpcode,x

    rep     #$31
a16()
    tya
    // carry clear
    adc.w   #4
    sta.w   scripts.programCounter,x

    // first argument - word value
    lda.w   scriptData,y
    sta.w   scripts.resumeArg1,x

    // second argument - word value
    lda.w   scriptData + 2,y
    sta.w   scripts.resumeArg2,x

    sep     #$30
a8()
i8()
    txy
    jmp     ProcessScripts.NextScript_Y
}



// Sets the script's resume function and puts the current script to sleep.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = resume instruction opcode (See Project.BytecodeOpcodes.Yielding)
// INPUT: programCounter = program counter at the start of the next instruction
a8()
i16()
code()
function Yield__Aopcode {
    ldy.b   programCounter

Fallthrough:
}



// Sets the script's resume function and puts the current script to sleep.
//
// This routine will reset `resumeArg1` and `resumeArg2` to 0.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = resume instruction opcode (See Project.BytecodeOpcodes.Yielding)
// INPUT: Y = program counter at the start of the next instruction
a8()
i16()
code()
function Yield__Ypc_Aopcode {
    assert(pc() == Yield__Aopcode.Fallthrough)

    // ::ANNOY no `stx addr,y` instruction::

    tyx

    ldy.w   currentScriptIndex

    sta.w   scripts.resumeOpcode,y

    rep     #$30
a16()
    // X = program counter
    txa
    sta.w   scripts.programCounter,y

    tdc
    // A = 0
    sta.w   scripts.resumeArg1,y
    sta.w   scripts.resumeArg2,y


    sep     #$30
a8()
i8()
    jmp     ProcessScripts.NextScript_Y
}



// Sets the script's resume function and `resumeArg1`, then puts the current script to sleep.
//
// This routine will reset `resumeArg2` to 0.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = resume instruction opcode (See Project.BytecodeOpcodes.Yielding)
// INPUT: Y = program counter at the start of the next instruction
// INPUT: X = value to store in resumeArg1
a8()
i16()
code()
function Yield__Xarg_Ypc_Aopcode {
    sty.b   programCounter

Fallthrough:
}



// Sets the script's resume function and `resumeArg1`, then puts the current script to sleep.
//
// This routine will reset `resumeArg2` to 0.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = resume instruction opcode (See Project.BytecodeOpcodes.Yielding)
// INPUT: X = value to store in `resumeArg1`
// INPUT: programCounter = program counter at the start of the next instruction
a8()
i16()
code()
function Yield__Xarg_Aopcode {
    assert(pc() == Yield__Xarg_Ypc_Aopcode.Fallthrough)

    // ::ANNOY no `sty addr,x` instruction::

    ldy.w   currentScriptIndex

    sta.w   scripts.resumeOpcode,y

    rep     #$30
a16()
    lda.b   programCounter
    sta.w   scripts.programCounter,y

    // X = first argument
    txa
    sta.w   scripts.resumeArg1,y

    tdc
    // A = 0
    sta.w   scripts.resumeArg2,y


    sep     #$30
a8()
i8()
    jmp     ProcessScripts.NextScript_Y
}



// Sets the script's resume function, sets `resumeArg1` and `resumeArg2`, then puts the current script to sleep.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = resume instruction opcode (See Project.BytecodeOpcodes.Yielding)
// INPUT: X = value to store in `resumeArg1`
// INPUT: Y = value to store in `resumeArg2`
// INPUT: programCounter = program counter at the start of the next instruction
a8()
i16()
code()
function Yield__XYarg_Aopcode {
    // ::ANNOY no `sty addr,x` instruction::

    sty.b   utDpTmp0

    ldy.w   currentScriptIndex

    sta.w   scripts.resumeOpcode,y

    rep     #$30
a16()
    lda.b   programCounter
    sta.w   scripts.programCounter,y

    // X = first argument
    txa
    sta.w   scripts.resumeArg1,y

    // Y = second argument
    lda.b   utDpTmp0
    sta.w   scripts.resumeArg2,y

    sep     #$30
a8()
i8()
    jmp     ProcessScripts.NextScript_Y
}



// Bytecode Resume Functions control
// =================================


// Put a resume instruction to sleep and start executing the next active script.
//
// This code MUST ONLY be JMPed to inside a bytecode instruction (see `BytecodeFunctionTable`).
//
// REQUIRES: DB = 0x7e, DP = 0
//
// INPUT: Y = currentScriptIndex
constant ResumeInstruction_Sleep_Y = ProcessScripts.NextScript_Y



// Put a resume instruction to sleep and start executing the next active script.
//
// This code MUST ONLY be JMPed to inside a `BytecodeResumeFunctionTable` routine.
//
// REQUIRES: DB = 0x7e, DP = 0
constant ResumeInstruction_Sleep = ProcessScripts.NextScript_Yunknown



// End a resume instruction and continue script execution.
//
// This code MUST ONLY be JMPed to inside a `BytecodeResumeFunctionTable` routine.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
a8()
i16()
code()
function ResumeInstruction_GotoNextOpcode {
    ldy.w   currentScriptIndex

Fallthrough:
}



// End a resume instruction and continue script execution.
//
// This code MUST ONLY be JMPed to inside a `BytecodeResumeFunctionTable` routine.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y = currentScriptIndex
a8()
i16()
code()
function ResumeInstruction_GotoNextOpcode_Y {
    assert(pc() == ResumeInstruction_GotoNextOpcode.Fallthrough)

    ldx.w   scripts.programCounter,y
    txy

    __ProcessNextOpcode()
}



// Special Resume Functions
// ========================

namespace SpecialResumeOpcodes {
    constant End_Script = 0
    constant Process_Script = 2
}

assert(Project.BytecodeOpcodes.Yielding.N_SPECIAL_RESUME_OPCODES == 6)

constant Bytecode._Special___Resume_0  = Bytecode.End_Script___Resume
constant Bytecode._Special___Resume_2  = ResumeInstruction_GotoNextOpcode_Y // Process_Script
constant Bytecode._Special___Resume_4  = Bytecode.End_Script___Resume
constant Bytecode._Special___Resume_6  = Bytecode.End_Script___Resume
constant Bytecode._Special___Resume_8  = Bytecode.End_Script___Resume
constant Bytecode._Special___Resume_10 = Bytecode.End_Script___Resume

}

// vim: ft=bass-65816 ts=4 sw=4 et:

