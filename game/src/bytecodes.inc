// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
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
namespace Bytecode {

CreateBgTasksFunctionTables()



// ::TODO change to tile index::
// ::TODO change to u8 second argument::
constant Clear_Map_Horizontal___ImmU16_ImmU16 = YieldInstruction__4ByteArg

// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: Y - currentScriptIndex
// RETURN: Jump to `ResumeInstruction_Sleep_Y` or `ResumeInstruction_GotoNextOpcode_Y`
a8()
i16()
code()
function Clear_Map_Horizontal___ImmU16_ImmU16___Resume {

    rep     #$30
a16()
    lda.w   scripts.resumeArg1,y
    tax

    clc
    adc.w   MetaTiles.map.bytesPerColumn
    cmp.w   MetaTiles.map.dataSize
    bcs     Done
    sta.w   scripts.resumeArg1,y

    txy
    lda.w   #0
    jsr     MetaTiles.SetMapCell


    ldy.w   currentScriptIndex

    lda.w   scripts.resumeArg2,y
    dec
    sta.w   scripts.resumeArg2,y
    beq     Done
    bmi     Done

    jmp     ResumeInstruction_Sleep_Y


Done:
    sep     #$20
a8()
    jmp     ResumeInstruction_GotoNextOpcode_Y

}


// INPUT: Y - program counter
// OUTPUT: programCounter - program counter after arguments
// RETURN: Jump to GotoNextOpcode_LoadPC
a8()
i16()
code()
function Crew_Rescued {
    sty.b   programCounter

    lda.b   crewRescuedFlag
    bne     EndIf
        php

        rep     #$30
    a16()
        lda.w   #Project.EntityIds.crew_rescued
        ldy.w   #0
        jsr     Entity.Spawn

        plp
EndIf:
    jmp     GotoNextOpcode_LoadPC
}

}
}

