// resources/_vblank.inc
// =====================
//
// VBlank routine for resources subsystem.
//
// SPDX-FileCopyrightText: © 2020 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2020 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Resources {

// Resources VBlank code
//
//  * Updates animated tileset display offset
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $2100
macro VBlank_dp2100() {
    assert16a()
    assert8i()

    assert(Resources.Format.SceneSettingsFunctionTable.tableSize < 256)  // Confirm 8 bit indexing is allowed on function table
    assert(Resources.Format.SceneSettingsFunctionTable.VBlank_dp2100 >> 16 == pc() >> 16) // Confirm correct Program Bank

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
farCode()
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
farCode()
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
farCode()
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
