// scripting/bytecodes.inc
// =======================
//
// Scripting bytecode instructions.
//
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


if {defined DEBUG_BUILD} {
    RegisterBreakId(INVALID_SCRIPTING_OPCODE)
}
Warnings.Register(INVALID_SCRIPTING_OPCODE, 16, 31, 16)


namespace Scripting {
namespace Bytecode {

constant N_FLAG_INSTRUCTIONS = GameState.flagData.size / (256 / 8)



CreateBgTaskInstruction(Spawn_Entity_Group, Room.EntityGroups.Spawn_Entity_Group__BgTask, EntityGroup)



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
inline Flag_Instruction(name, suffix) {
    evaluate __FI_n = 0

    while {__FI_n} < N_FLAG_INSTRUCTIONS {
        a8()
        i16()
        code()
        function {name}___Flag{__FI_n}{suffix} {
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

    // NOTE: If I modify this code, also modify the `__BranchIfFlag` macro

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
    if {defined DEBUG_BUILD} {
        break(INVALID_SCRIPTING_OPCODE)

    } else {
        Warnings.Warn(INVALID_SCRIPTING_OPCODE)
    }

    jmp     End_Script
}



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



// Conditional Opcodes
// ===================


// Conditional branch between a word variable and an immediate value.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
// PARAM: branch - branch instruction to use (`beq`, `bne`, `bcc` or `bcs`)
//
// INPUT: Y - program counter
// OUTPUT: Y - new program counter (either after this instruction or branch PC)
// RETURN: Jump to GotoNextOpcode
inline __BranchWordOpcode(name, branch) {
    a8()
    i16()
    code()
    function {name} {
        // first argument: Word index
        lda.w   scriptData,y
        iny
        asl
        tax

        rep     #$30
    a16()
        lda.w   GameState.wordData,x

        // second argument: immediate u16 word
        iny
        iny
        cmp.w   scriptData - 2,y

        sep     #$20
    a8()
        {branch} Goto___PC

        // skip third argument: Program Counter
        iny
        iny

        jmp     GotoNextOpcode
    }
}
__BranchWordOpcode(BranchIfWordEqual___Word_ImmU16_PC,              beq)
__BranchWordOpcode(BranchIfWordNotEqual___Word_ImmU16_PC,           bne)
__BranchWordOpcode(BranchIfWordLessThan___Word_ImmU16_PC,           bcc)
__BranchWordOpcode(BranchIfWordGreaterThanEqual___Word_ImmU16_PC,   bcs)



// Change Program Counter
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// OUTPUT: Y - program counter in instruction argument
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Goto___PC {
    // Argument: Program Counter
    ldx.w   scriptData,y
    txy

    jmp     GotoNextOpcode
}



// Branch if a flag is set or cleared
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
// PARAM: branch - branch instruction to use (`beq` or `bne`)
//
// INPUT: Y - program counter
// OUTPUT: Y - new program counter (either after this instruction or branch PC)
// RETURN: Jump to GotoNextOpcode
macro __BranchIfFlag(branch) {
    assert8a()
    assert16i()

    // NOTE: If I modify this code, also modify the `__TestGameStateFlag` macro

    // first argument: flag index
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
    {branch}    Goto___PC

    // skip second argument: Program Counter
    iny
    iny

    jmp     GotoNextOpcode
}



// Branch if a flag is set
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: Y - new program counter (either after this instruction or branch PC)
// RETURN: Jump to GotoNextOpcode
macro BranchIfFlagSet() {
    __BranchIfFlag(bne)
}
Flag_Instruction(BranchIfFlagSet, _PC)



// Branch if a flag is clear
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: Y - new program counter (either after this instruction or branch PC)
// RETURN: Jump to GotoNextOpcode
macro BranchIfFlagClear() {
    __BranchIfFlag(beq)
}
Flag_Instruction(BranchIfFlagClear, _PC)



// Default Instructions
// ====================


// Start a room script.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
//
// INPUT: Y - program counter
// OUTPUT: programCounter - program counter after arguments
// RETURN: Jump to GotoNextOpcode_LoadPC
a8()
i16()
code()
function Start_Script___RoomScript {
    // first argument: Room Script Id
    lda.w   scriptData,y
    iny
    sty.b   programCounter

    jsr     ActivateScript

    jmp     GotoNextOpcode_LoadPC
}



// Load a new room.
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode
a8()
i16()
code()
function Load_Room___Room_RoomEntrance {
    // first argument: Room id
    // second argument: Room Entrance
    assert(GameState.roomId + 1 == GameState.entranceId)
    ldx.w   scriptData,y
    iny
    iny
    stx.w   GameState.roomId

    lda.b   #0xff
    sta.w   Room.roomChangeRequested

    jmp     GotoNextOpcode
}



// Base Yielding Instructions
// ==========================


// The sleep instruction will pause script execution for a given number of animation ticks.
constant Sleep_AnimationTicks___ImmU16 = YieldInstruction__2ByteArg



// Decrement the sleep timer and continue sleeping if it is non-zero,
// otherwise continue script execution.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_Sleep_Y` or `ResumeInstruction_GotoNextOpcode_Y`
a8()
i16()
code()
function Sleep_AnimationTicks___ImmU16___Resume {
    rep     #$30
a16()

    // arg1 = timer
    lda.w   scripts.resumeArg1,y
    sec
    sbc.w   Resources.animationSpeed

    bcc     +
    beq     +
        sta.w   scripts.resumeArg1,y

        // ResumeInstruction_Sleep_Y is register size independent
        jmp     ResumeInstruction_Sleep_Y
    +

    sep     #$20
a8()
    jmp     ResumeInstruction_GotoNextOpcode_Y
}



// Flag Tables
// ===========

rodata()
SetFlagBits:
    variable _n = 0
    while _n < 8 {
        db  1 << _n
        _n = _n + 1
    }

rodata()
ClearFlagBits:
    variable _n = 0
    while _n < 8 {
        db  0xff ^ (1 << _n)
        _n = _n + 1
    }

}
}


// vim: ft=bass-65816 ts=4 sw=4 et:

