// scripting/bytecodes.inc
// =======================
//
// Scripting bytecode instructions.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


Warnings.Register(INVALID_SCRIPTING_OPCODE, 16, 31, 16)


namespace Scripting {
namespace Bytecode {

constant N_FLAG_INSTRUCTIONS = GameState.flagData.size / (256 / 8)


// A simple macro to generate flag instruction opcode routines.
//
// Flag instructions have multiple opcodes, one for each block of 256 flags.
//
// PARAM: name - the name of the instruction
inline Flag_Instruction(name) {
    evaluate __FI_n = 0

    while {__FI_n} < N_FLAG_INSTRUCTIONS {
        a8()
        i16()
        code()
        function {name}___Flag{__FI_n} {
            constant flagData = GameState.flagData + (256 / 8) * {__FI_n}
            {name}()
        }

        evaluate __FI_n = {__FI_n} + 1
    }
}



// Reads a flag argument and tests a GameState Flag
//
// NOTE: This is not a bytecode instruction, it MUST ONLY be called by the
//       `TestGameStateFlag_BranchIfSet()` and `TestGameStateFlag_BranchIfClear` macros.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// OUTPUT: Y = Y + 1
// OUTPUT: zero flag clear if flag set
macro __TestGameStateFlag() {
    assert8a()
    assert16i()

    // Argument: flag index
    lda.w   scriptData,y
    iny
    sty.b   programCounter

    tay
    and.b   #0x7
    tax

    tya
    lsr
    lsr
    lsr
    tay
    lda.w   flagData,y

    ldy.b   programCounter

    // A = flagData bits
    and.l   SetFlagBits,x

    // zero flag clear if flag set
    rts
}
Flag_Instruction(__TestGameStateFlag)



// Reads a flag bytecode argument and branches to `label` if the flag is clear.
//
// PARAM: label - the label to branch to
// PARAM: flagData - the flags that this instruction operates upon
//
// INPUT: Y - program counter at flag argument
// OUTPUT: Y - program counter after flag argument
// BRANCH: to `label` if flag is clear
macro TestGameStateFlag_BranchIfClear(label) {
    evaluate n = (flagData - GameState.flagData) / (256 / 8)
    jsr     Scripting.Bytecode.__TestGameStateFlag___Flag{n}
    beq     {label}
}


// Reads a flag bytecode argument and branches to `label` if the flag is set.
//
// PARAM: label - the label to branch to
// PARAM: flagData - the flags that this instruction operates upon
//
// INPUT: Y - program counter at flag argument
// OUTPUT: Y - program counter after flag argument
// BRANCH: to `label` if flag is set
macro TestGameStateFlag_BranchIfSet(label) {
    evaluate n = (flagData - GameState.flagData) / (256 / 8)
    jsr     Scripting.Bytecode.__TestGameStateFlag___Flag{n}
    bne     {label}
}



// Invalid Opcode (Stops script execution)
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
a8()
i16()
code()
function InvalidOpcode {
    Warnings.Warn(INVALID_SCRIPTING_OPCODE)
    jmp     Scripting.ProcessScripts.EndActiveScript
}



// Stops the script
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
constant End_Script = Scripting.ProcessScripts.EndActiveScript



// Set flag opcodes
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: programCounter - program counter after arguments
// RETURN: Jump to GotoNextOpcode_LoadPC
macro Set_Flag() {
    assert8a()
    assert16i()

    // Argument: flag index
    lda.w   scriptData,y
    iny
    sty.b   programCounter

    tay
    and.b   #0x7
    tax

    tya
    lsr
    lsr
    lsr
    tay

    lda.l   SetFlagBits,x
    ora.w   flagData,y
    sta.w   flagData,y

    jmp     GotoNextOpcode_LoadPC
}
Flag_Instruction(Set_Flag)



// Clear flag opcodes
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: programCounter - program counter after arguments
// RETURN: Jump to GotoNextOpcode_LoadPC
macro Clear_Flag() {
    assert8a()
    assert16i()

    // Argument: flag index
    lda.w   scriptData,y
    iny
    sty.b   programCounter

    tay
    and.b   #0x7
    tax

    tya
    lsr
    lsr
    lsr
    tay

    lda.l   ClearFlagBits,x
    and.w   flagData,y
    sta.w   flagData,y

    jmp     GotoNextOpcode_LoadPC
}
Flag_Instruction(Clear_Flag)



// Set GameState word opcode
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
//
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Set_Word___Word_ImmU16 {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    rep     #$30
a16()
    // Second argument: word value
    lda.w   scriptData,y
    iny
    iny

    sta.w   GameState.wordData,x

    sep     #$20
a8()

    jmp     GotoNextOpcode
}



// Add to GameState word opcode
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
//
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Add_To_Word___Word_ImmU16 {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    rep     #$31
a16()
    // Second argument: word value
    lda.w   scriptData,y
    iny
    iny

    // carry clear
    adc.w   GameState.wordData,x
    bcc     +
        lda.w   #0xffff
    +
    sta.w   GameState.wordData,x

    sep     #$20
a8()

    jmp     GotoNextOpcode
}



// Subtract From GameState word opcode
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
//
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Subtract_From_Word___Word_ImmU16 {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    rep     #$30
a16()

    lda.w   GameState.wordData,x
    sec
    // Second argument: word value
    sbc.w   scriptData,y
    bcs     +
        tdc
    +
    sta.w   GameState.wordData,x

    iny
    iny

    sep     #$20
a8()

    jmp     GotoNextOpcode
}



// Increment GameState word opcode
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
//
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Increment_Word___Word {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    inc.w   GameState.wordData,x
    bne     +
        inc.w   GameState.wordData + 1,x
        bne     +
            lda.b   #0xff
            sta.w   GameState.wordData,x
            sta.w   GameState.wordData + 1,x
    +

    jmp     GotoNextOpcode
}



// Decrement GameState word opcode
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
//
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Decrement_Word___Word {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    rep     #$30
a16()

    lda.w   GameState.wordData,x
    beq     +
        dec
        sta.w   GameState.wordData,x
    +

    sep     #$20
a8()

    jmp     GotoNextOpcode
}



rodata(rom0)
SetFlagBits:
    variable _n = 0
    while _n < 8 {
        db  1 << _n
        _n = _n + 1
    }

rodata(rom0)
ClearFlagBits:
    variable _n = 0
    while _n < 8 {
        db  0xff ^ (1 << _n)
        _n = _n + 1
    }

}
}


// vim: ft=bass-65816 ts=4 sw=4 et:

