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

    // Ensure bgIndex can be used for PPU offset registers
    assert(BG1HOFS + Resources.BG_INDEX_INCREMENT == BG2HOFS)

    assertShadowRam(Resources.AnimatedTileset.bgIndex)
    assertShadowRam(Resources.AnimatedTileset.displayHoffset)
    assertShadowRam(Resources.AnimatedTileset.displayVoffset)

    ldx.w   Resources.AnimatedTileset.bgIndex
    bmi     SkipUpdate
        ldy.w   Resources.AnimatedTileset.displayHoffset
        sty.b   BG1HOFS,x
        ldy.w   Resources.AnimatedTileset.displayHoffset + 1
        sty.b   BG1HOFS,x

        ldy.w   Resources.AnimatedTileset.displayVoffset
        sty.b   BG1VOFS,x
        ldy.w   Resources.AnimatedTileset.displayVoffset + 1
        sty.b   BG1VOFS,x
    SkipUpdate:

    assert(Resources.SceneSettingsFunctionTable.tableSize < 256)  // Confirm 8 bit indexing is allowed on function table
    ldx.w   Resources.Scene.functionTableIndex
    jsr     (Resources.Format.SceneSettingsFunctionTable.VBlank_dp2100,x)
}

}

