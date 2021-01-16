// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Scripting {
namespace Bytecode {



// Jumps to the unit test fail screen
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// INPUT: Y - program counter
// RETURN: Test.Fail
a8()
i16()
code()
constant Fail = Test.Fail



// Jump to unit test fail screen if the flag is not set
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode or Test.Fail
macro Assert_Flag_Set() {
    TestGameStateFlag_BranchIfSet(FlagSet)
        jmp     Test.Fail

FlagSet:
    jmp     GotoNextOpcode
}
Flag_Instruction(Assert_Flag_Set)



// Jump to unit test fail screen if the flag is not set
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
// PARAM: flagData - the flag data block for this opcode
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode or Test.Fail
macro Assert_Flag_Clear() {
    TestGameStateFlag_BranchIfClear(FlagClear)
        jmp     Test.Fail

FlagClear:
    jmp     GotoNextOpcode
}
Flag_Instruction(Assert_Flag_Clear)



// Asserts a GameState word is equal to a immediate value
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0, DB = 0x7e, Ah = 0
//
// INPUT: Y - program counter
// OUTPUT: Y - program counter after arguments
// RETURN: Jump to GotoNextOpcode or Test.Fail
a8()
i16()
code()
function Assert_Word_Equals___Word_ImmU16 {
    // first argument: Word index
    lda.w   scriptData,y
    iny
    asl
    tax

    rep     #$30
a16()

    // second argument: immediate u16 word
    lda.w   scriptData,y
    iny
    iny

    cmp.w   GameState.wordData,x
    beq     +
        jmp     Test.Fail
    +

    sep     #$20
a8()

    jmp     GotoNextOpcode
}


}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

