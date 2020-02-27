// resources/_vblank.inc
// =====================
//
// VBlank routine for resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

// Resources VBlank code
//
//  * Updates animated tileset display offset
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $2100
//
// MODIFIES: VMAIN = VMAIN.incrementMode.high
macro VBlank_dp2100() {
    assert16a()
    assert8i()

    assertShadowRam(Resources.AnimatedTileset.displayHoffset)
    assertShadowRam(Resources.AnimatedTileset.displayVoffset)

    ldy.w   Resources.AnimatedTileset.displayHoffset
    sty.b   BG1HOFS
    ldy.w   Resources.AnimatedTileset.displayHoffset + 1
    sty.b   BG1HOFS

    ldy.w   Resources.AnimatedTileset.displayVoffset
    sty.b   BG1VOFS
    ldy.w   Resources.AnimatedTileset.displayVoffset + 1
    sty.b   BG1VOFS
}

}

