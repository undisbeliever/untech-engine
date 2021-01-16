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


    // Reset currentScriptIndex and scriptProgramCounters to 0

    assert(Scripting.currentScriptIndex + 2 == Scripting.scriptProgramCounters)
    ldx.b   #(Scripting.N_SCRIPTS - 1) * 2 + 2
    Loop:
        stz.w   Scripting.currentScriptIndex,x
        dex
        dex
        bpl     Loop
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

        // Check if script is already active
        rep     #$20
    a16()
        lda.w   scriptProgramCounters,x
        bne     Skip

            // script id is valid and not active
            // Start script
            lda.w   Room.Format.Header.scripts,x
            sta.w   scriptProgramCounters,x

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
    sep     #$20
a8()

    ldx.w   #(N_SCRIPTS - 1) * 2

    Loop:
        ldy.w   scriptProgramCounters,x
        bne     ScriptActive

    ContinueLoop:
        dex
        dex
        bpl     Loop


    rep     #$30
a16()
i16()
    rts


a8()
i16()
ScriptActive:
// X = script index
// Y = program counter
    stx.w   currentScriptIndex

    bra     Bytecode.GotoNextOpcode


a8()
i16()
EndActiveScript:
    // The `End_Script` bytecode instruction jumps here

    ldx.w   currentScriptIndex

    // Deactivate script
    stz.w   scriptProgramCounters,x
    stz.w   scriptProgramCounters + 1,x

    bra     ContinueLoop
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

}

// vim: ft=bass-65816 ts=4 sw=4 et:

