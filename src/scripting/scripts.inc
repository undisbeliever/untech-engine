// scripting/scripts.inc
// =====================
//
// Script activation and processing routines.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Scripting {


// ::TODO use a linked list to iterate through the active scripts ::


// Initialize and reset the scripting subsystem
//
// Called when a room is loaded
//
// REQUIRES: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
macro _Init() {
    assert16a()
    assert8i()


    // Reset currentScriptIndex and script variables to 0

    assert(Scripting.currentScriptIndex + 2 == Scripting.scripts.programCounter)
    assert(Scripting.scripts.size + 2 < 0xff)
    ldx.b   #Scripting.scripts.size + 2
    Loop:
        stz.w   Scripting.currentScriptIndex,x
        dex
        dex
        bne     Loop
}



// Activate a room script.
//
// This routine will do nothing if the *script Id* is invalid or the script is
// already active.
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
    cmp.b   #N_SCRIPTS
    bcs     Skip
        asl
        tax

        // Check if script is not active active
        assert(Project.BytecodeOpcodes.Yielding.End_Script == 0)
        lda.w   scripts.resumeOpcode,x
        bne     Skip

            lda.b   #Project.BytecodeOpcodes.Yielding.Process_Script
            sta.w   scripts.resumeOpcode,x

            rep     #$20
        a16()
            // Set Program Counter
            lda.w   Room.Format.Header.scripts,x
            sta.w   scripts.programCounter,x

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

    ldy.b   #(N_SCRIPTS - 1) * 2

    Loop:
        assert(Project.BytecodeOpcodes.Yielding.End_Script == 0)
        ldx.w   scripts.resumeOpcode,y
        bne     ScriptActive

        // Y = currentScriptIndex
    ContinueLoop:
        dey
        dey
        bpl     Loop


    rep     #$30
a16()
i16()
    rts


a8()
i8()
ScriptActive:
// Y = script index
// X = resumeOpcode

    sty.w   currentScriptIndex

    rep     #$10
i16()
    jmp     (BytecodeResumeFunctionTable,x)


au()
iu()
YieldScript:
    sep     #$30
i8()
a8()
    ldy.w   currentScriptIndex

    bra     ContinueLoop


au()
iu()
YieldScript_YcurrentScriptIndex:
    // Y = currentScriptIndex
    sep     #$30
i8()
a8()
    bra     ContinueLoop


au()
iu()
EndActiveScript:
    // The `End_Script` bytecode instruction jumps here

    sep     #$30
i8()
a8()
    ldy.w   currentScriptIndex

    // Deactivate script
    lda.b   #Project.BytecodeOpcodes.Yielding.End_Script
    sta.w   scripts.resumeOpcode,y

    bra     ContinueLoop
}



// Bytecode Instruction control
// ============================


// Used to indicate a script is not active.
// Execution should never enter reach this resume instruction.
constant Bytecode.End_Script___Resume = ProcessScripts.ContinueLoop




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
    jmp     ProcessScripts.ContinueLoop
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
    jmp     ProcessScripts.ContinueLoop
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
    jmp     ProcessScripts.ContinueLoop
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
    jmp     ProcessScripts.ContinueLoop
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
    jmp     ProcessScripts.ContinueLoop
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
    jmp     ProcessScripts.ContinueLoop
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
constant ResumeInstruction_Sleep_Y = ProcessScripts.YieldScript_YcurrentScriptIndex



// Put a resume instruction to sleep and start executing the next active script.
//
// This code MUST ONLY be JMPed to inside a `BytecodeResumeFunctionTable` routine.
//
// REQUIRES: DB = 0x7e, DP = 0
constant ResumeInstruction_Sleep = ProcessScripts.YieldScript



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



// Misc
// ====

// This resume instruction is executed when a script is activated.
constant Bytecode._Process_Script___Resume = ResumeInstruction_GotoNextOpcode_Y

}

// vim: ft=bass-65816 ts=4 sw=4 et:

