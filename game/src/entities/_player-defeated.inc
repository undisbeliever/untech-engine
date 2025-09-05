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


namespace Entities {
namespace Players {

namespace Defeated {

allocate(frameCountdown, wram7e, 2)


constant FRAMES_BEFORE_LEVEL_SCREEN = 120


buildFunctionTable(BasePlayerFunctionTable, Defeated)

constant DrawPlayer = MetaSprite.Render.RenderEntity



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
// DP unknown
code()
inline SwitchToDefeatedPlayer() {
    assert8a()
    assert16i()

    ldx.w   #Entities.Players.Defeated.FunctionTable
    cpx.w   Entity.playerEntity + BaseEntity.functionTable
    beq     EndIf{#}
        stx.w   Entity.playerEntity + BaseEntity.functionTable

        ldx.w   #Entities.Players.Defeated.FRAMES_BEFORE_LEVEL_SCREEN - 1
        stx.w   Entities.Players.Defeated.frameCountdown

        // ::TODO switch to defeated song::
    EndIf{#}:
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function NullFunction {
    rts
}


constant Init = NullFunction
constant RoomAboutToChange = NullFunction
constant HitboxCollision = NullFunction
constant HurtboxCollision = NullFunction
constant ShieldCollision = NullFunction


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function Process {
    // Switch to level name screen after a delay
    dec.w   frameCountdown
    bpl     +
        lda.w   #Mode.LEVEL_NAME
        sta.w   currentMode
    +


    ldx.w   #MovementTableIndex.PlayerWalk
    jsr     DecelerateXmomentum


    MoveEntityAndProcessTileCollisions()

    // Player entities MUST NOT draw in the Process Routine
    jmp     MetaSprite.Animation.Process
}

}
}
}

