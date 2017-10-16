// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace MetaSpriteTest {

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
a16()
i16()
code()
function _Init_MetaSpriteModule {
    jsr     Dma.Init
    jsr     MetaSprite.Init

    stz.w   Dma.ObjPalette.dirty

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

