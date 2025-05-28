// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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

namespace MetaSpriteTest {

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
a16()
i16()
code()
function _Init_MetaSpriteModule {
    jsr     Dma.Init
    jsr     MetaSprite.Init

    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
a16()
i16()
code()
function _InitEntity {
    MetaSprite._InitEntity()
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
a16()
i16()
code()
function _InitEntityAndActivate {
    jsr     _InitEntity
    jmp     MetaSprite.Activate
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
a16()
i16()
code()
function _InitEntity0Palette {
    ldy.w   #0
    jmp     _InitEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
a16()
i16()
code()
function _InitEntity0PaletteAndActivate {
    ldy.w   #0
    jmp     _InitEntityAndActivate
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
// INPUT:  X = frame Id
// OUTPUT: C set on success
a16()
i16()
code()
function _InitEntityFrameActivate {
    phx
        jsr     _InitEntity
    pla

    jsr     MetaSprite.SetFrame
    bcc     Fail

    jmp     MetaSprite.Activate

Fail:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

