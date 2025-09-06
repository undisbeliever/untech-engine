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


namespace Mode {
    createEnum(0, 2)
        enum(GAME)
        enum(TITLE_SCREEN)
        enum(LEVEL_NAME)
    endEnum()
}

constant N_MODES = Mode.__ENUM__.count

code()
ModeFunctionTable:
    dw  GameLoop
    dw  TitleScreen.Run
    dw  LevelNameScreen.Run
assert(pc() - ModeFunctionTable == N_MODES * 2)



// CAUTION: Changes Stack Pointer
// IN: A = new mode
au()
iu()
// DB unknown
code()
function SwitchMode {
    // 16 bit A, 16 bit I, decimal mode disabled
    rep #$38
a16()
i16()
    // Reset stack
    ldx.w   #STACK_BOTTOM
    txs

    // Reset DB (without setting A)
    pea     0x7e7e
    plb
    plb

    // Reset direct page (without changing A)
    pea     0
    pld

    // A = new mode
    and.w   #0xfe
    cmp.w   #N_MODES * 2
    bcc     +
        lda.w   #Mode.TITLE_SCREEN
    +
    sta.w   currentMode
    tax
    jsr     (ModeFunctionTable,x)

    bra     SwitchMode
}


