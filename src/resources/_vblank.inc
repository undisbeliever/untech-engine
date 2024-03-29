// resources/_vblank.inc
// =====================
//
// VBlank routine for resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

// Resources VBlank code
//
//  * Updates animated tileset display offset
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $2100
macro VBlank_dp2100() {
    assert16a()
    assert8i()

    assert(Resources.SceneSettingsFunctionTable.tableSize < 256)  // Confirm 8 bit indexing is allowed on function table
    ldx.w   Resources.Scene.functionTableIndex
    jsr     (Resources.Format.SceneSettingsFunctionTable.VBlank_dp2100,x)
}
}

namespace Scenes {

// Set Animated Tileset BG1 scroll registers
//
// This routine should only be called in a Scene's `VBlank_dp2100` routine.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $2100
a16()
i8()
code()
function VBlank_dp2100_AnimatedTileset_Bg1 {
    assertLowRam(Resources.AnimatedTileset.bgIndex)
    assertLowRam(Resources.AnimatedTileset.displayHoffset)
    assertLowRam(Resources.AnimatedTileset.displayVoffset)

    ldy.w   Resources.AnimatedTileset.displayHoffset
    sty.b   BG1HOFS
    ldy.w   Resources.AnimatedTileset.displayHoffset + 1
    sty.b   BG1HOFS

    ldy.w   Resources.AnimatedTileset.displayVoffset
    sty.b   BG1VOFS
    ldy.w   Resources.AnimatedTileset.displayVoffset + 1
    sty.b   BG1VOFS

    rts
}

// Set Animated Tileset BG2 scroll registers
//
// This routine should only be called in a Scene's `VBlank_dp2100` routine.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $2100
a16()
i8()
code()
function VBlank_dp2100_AnimatedTileset_Bg2 {
    assertLowRam(Resources.AnimatedTileset.bgIndex)
    assertLowRam(Resources.AnimatedTileset.displayHoffset)
    assertLowRam(Resources.AnimatedTileset.displayVoffset)

    ldy.w   Resources.AnimatedTileset.displayHoffset
    sty.b   BG2HOFS
    ldy.w   Resources.AnimatedTileset.displayHoffset + 1
    sty.b   BG2HOFS

    ldy.w   Resources.AnimatedTileset.displayVoffset
    sty.b   BG2VOFS
    ldy.w   Resources.AnimatedTileset.displayVoffset + 1
    sty.b   BG2VOFS

    rts
}



// Set Animated Tileset BG3 scroll registers
//
// This routine should only be called in a Scene's `VBlank_dp2100` routine.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $2100
a16()
i8()
code()
function VBlank_dp2100_AnimatedTileset_Bg3 {
    assertLowRam(Resources.AnimatedTileset.bgIndex)
    assertLowRam(Resources.AnimatedTileset.displayHoffset)
    assertLowRam(Resources.AnimatedTileset.displayVoffset)

    ldy.w   Resources.AnimatedTileset.displayHoffset
    sty.b   BG3HOFS
    ldy.w   Resources.AnimatedTileset.displayHoffset + 1
    sty.b   BG3HOFS

    ldy.w   Resources.AnimatedTileset.displayVoffset
    sty.b   BG3VOFS
    ldy.w   Resources.AnimatedTileset.displayVoffset + 1
    sty.b   BG3VOFS

    rts
}

}
