// scripting/scripts.inc
// =====================
//
// Script activation and processing routines.
//
// SPDX-FileCopyrightText: © 2021 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2021 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Scripting {

constant STARTUP_SCRIPT_ID = 0xff


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

    ldx.b   #VariablesSize & 0xfe
    Loop:
        dex
        dex
        stz.w   Scripting.__VariablesBegin,x
        bne     Loop


    if VariablesSize % 2 != 0 {
        assert16a()
        stz.w   Scripting.__VariablesEnd - 2
    }
}



// Execute all active scripts and the background task.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
a8()
i16()
code()
function ProcessScripts {
    sep     #$20
a8()
    BackgroundTasks._ProcessBackgroundTask()


    sep     #$30
a8()
i8()

    ldy.w   activeScriptsEnd
    beq     Return

    Loop:
        assert(scripts.INDEX_INCREMENT == 2)
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


        // Mark script as deactivated.
        // Bounds test is required as the startup script has no entry in the `activeScripts` array.
        assert(STARTUP_SCRIPT_ID > Room.N_ROOM_SCRIPTS)
        ldx.w   scripts.scriptId,y
        cpx.b   #Room.N_ROOM_SCRIPTS
        bcs     +
            stz.w   activeScripts,x
        +


        // Remove script data from scripts SoA by moving the last script to currentScriptIndex
        // (This is safe as we are iterating backwards)

        // activeScriptsEnd is non-zero
        ldx.w   activeScriptsEnd
        assert(scripts.INDEX_INCREMENT == 2)
        dex
        dex
        stx.w   activeScriptsEnd

        // X = source
        // Y = target
        rep     #$20
    a16()
        assert(scripts.scriptId + 1 == scripts.resumeOpcode)
        lda.w   scripts.scriptId,x
        sta.w   scripts.scriptId,y

        lda.w   scripts.programCounter,x
        sta.w   scripts.programCounter,y

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



// Process a room's startup script.
//
// After the startup script has ended, this macro will activate any scripts
// that was started by the startup script.
//
// ASSUMES this macro is called in the `Room.LoadRoom` routine and it is the
// first Scripting routine or macro called after `Scripting._Init`
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
//
// REQUIRES: Room loaded, player spawned, camera active
macro _ProcessRoomStartupScript() {
    assert16a()
    assert16i()

    sep     #$20
a8()

    // Notify system the Room Startup Script is active
    assert(Scripting.STARTUP_SCRIPT_ID != 0)
    lda.b   #Scripting.STARTUP_SCRIPT_ID
    sta.w   Scripting.startupScriptActive


    // Add startup script to active script list
    sta.w   Scripting.scripts.scriptId

    lda.b   #Scripting.SpecialResumeOpcodes.Process_Script
    sta.w   Scripting.scripts.resumeOpcode

    ldx.w   Room.Format.Header.startupScript
    stx.w   Scripting.scripts.programCounter

    lda.b   #Scripting.scripts.INDEX_INCREMENT
    sta.w   Scripting.activeScriptsEnd


    rep     #$30
a16()

    // Continue executing the Startup script until it has completed
    Loop:
        jsr     Scripting.ProcessScripts

        ldx.w   Scripting.activeScriptsEnd
        bne     Loop


    // Activate all scripts that were marked by `ActivateScript`
    sep     #$10
a16()
i8()

    ldy.b   #0
    sty.w   Scripting.startupScriptActive

    // Y = 0
    ActivateLoop:
        ldx.w   Scripting.activeScripts,y
        beq     NotActive
            tya
            asl
            tax

            // Determine if the script exists
            lda.w   Room.Format.Header.scripts,x
            beq     NotActive
                // Activate script
                ldx.w   Scripting.activeScriptsEnd

                sta.w   Scripting.scripts.programCounter,x

                // Set scriptId & resumeOpcode
                assert(Scripting.scripts.scriptId + 1 == Scripting.scripts.resumeOpcode)
                tya
                // Ah is 0
                ora.w   #Scripting.SpecialResumeOpcodes.Process_Script << 8
                sta.w   Scripting.scripts.scriptId,x

                assert(Scripting.scripts.INDEX_INCREMENT == 2)
                inx
                inx
                stx.w   Scripting.activeScriptsEnd

                // No need to bounds check `activeScriptsEnd`
                assert(Scripting.MAX_ACTIVE_SCRIPTS >= Room.N_ROOM_SCRIPTS)

    NotActive:
        iny
        cpy.b   #Room.N_ROOM_SCRIPTS
        bcc     ActivateLoop

    rep     #$30
a16()
i16()
}



// Activate a room script.
//
// This routine will do nothing if the *script Id* is invalid or the
// script is already active.
//
// Scripts will not be activated if `startupScriptActive` is set.
// Instead the script will be marked for delayed activation and will be activated
// after the room startup script ends.
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
        tay

        // MUST NOT MODIFY Y

        // Do not activate any scripts if the startup script is active
        ldx.w   startupScriptActive
        bne     StartupScriptIsActive

        // Check if the script is active
        ldx.w   activeScripts,y
        bne     Skip

            // Calculate index into room script header
            asl
            tax

            // Mark script active
            assert(MAX_ACTIVE_SCRIPTS < 0x80)
            inc
            // A is non-zero
            sta.w   activeScripts,y


            rep     #$20
        a16()

            // Read the script's program counter (and confirm the script exists)
            lda.w   Room.Format.Header.scripts,x
            beq     Skip
                ldx.w   activeScriptsEnd

                // Sanity check
                cpx.b   #MAX_ACTIVE_SCRIPTS * 2
                bcs     Skip
                    sta.w   scripts.programCounter,x

                    // Set scriptId & resumeOpcode
                    assert(Scripting.scripts.scriptId + 1 == Scripting.scripts.resumeOpcode)
                    // Y = scriptId
                    tya
                    // Ah is 0
                    ora.w   #Scripting.SpecialResumeOpcodes.Process_Script << 8
                    sta.w   Scripting.scripts.scriptId,x


                    assert(scripts.INDEX_INCREMENT == 2)
                    inx
                    inx
                    stx.w   activeScriptsEnd

au()
Skip:
    plp
    rts


a8()
i8()
StartupScriptIsActive:
    // Mark script so it can be activated after the startup script ends
    // (See `_ProcessRoomStartupScript`)

    // Y = scriptId (in bounds)
    lda.b   #0xff
    sta.w   activeScripts,y

    plp
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

    sty.b   utZpTmp0

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
    lda.b   utZpTmp0
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

    constant StartBgTask = 4
    constant WaitUntilBgTaskEnd = 6
}

assert(Project.BytecodeOpcodes.Yielding.N_SPECIAL_RESUME_OPCODES == 6)

constant Bytecode._Special___Resume_0  = Bytecode.End_Script___Resume
constant Bytecode._Special___Resume_2  = ResumeInstruction_GotoNextOpcode_Y // Process_Script
constant Bytecode._Special___Resume_4  = BackgroundTasks.StartBgTask___Resume
constant Bytecode._Special___Resume_6  = BackgroundTasks.WaitUntilBgTaskEnd___Resume
constant Bytecode._Special___Resume_8  = Bytecode.End_Script___Resume
constant Bytecode._Special___Resume_10 = Bytecode.End_Script___Resume

}

// vim: ft=bass-65816 ts=4 sw=4 et:

