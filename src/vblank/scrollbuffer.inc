// vblank/scrollbuffer.inc
// =======================
//
// VBlank buffer for the scroll registers.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace ScrollBuffer {
namespace bg1 {
    allocate(hOffset, shadow, 2)
    allocate(vOffset, shadow, 2)
}
namespace bg2 {
    allocate(hOffset, shadow, 2)
    allocate(vOffset, shadow, 2)
}


// VBlank code, updates scroll registers
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank() {
    assert16a()
    assert8i()

    ldx.w   ScrollBuffer.bg1.hOffset
    stx.w   BG1HOFS
    ldx.w   ScrollBuffer.bg1.hOffset + 1
    stx.w   BG1HOFS

    ldx.w   ScrollBuffer.bg1.vOffset
    stx.w   BG1VOFS
    ldx.w   ScrollBuffer.bg1.vOffset + 1
    stx.w   BG1VOFS

    ldx.w   ScrollBuffer.bg2.hOffset
    stx.w   BG2HOFS
    ldx.w   ScrollBuffer.bg2.hOffset + 1
    stx.w   BG2HOFS

    ldx.w   ScrollBuffer.bg2.vOffset
    stx.w   BG2VOFS
    ldx.w   ScrollBuffer.bg2.vOffset + 1
    stx.w   BG2VOFS
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

