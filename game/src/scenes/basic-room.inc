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

namespace Scenes {
namespace BasicRoom {

a16()
i8()
code()
function SetupPpu_dp2100 {
    rts
}


a16()
i8()
code()
function Process {
    rts
}


a16()
i8()
// DB = 0x80
// DP = $2100
farCode()
function VBlank_dp2100 {
    Healthbar.VBlank_dp2100()

    jmp     VBlank_dp2100_AnimatedTileset_Bg1
}


}
}

