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


define MEMORY_MAP = LOROM
define ROM_SIZE = 2
define ROM_SPEED = slow // ::TODO change in debug/release build::
define REGION = US // NTSC
define ROM_NAME = "RESCUE"
define VERSION = 0

include "../../engine/common.inc"
include "memmap.inc"


include "../gen/audio-data.inc"
include "../gen/project.inc"

include "../gen/movement-table.inc"
include "../gen/tables/sine-table.inc"


// Setup console font
include "../resources/text/text.inc"
namespace Text {
    namespace Font {
rodata()
        insert FixedTiles, "../gen/font-fixed.1bpp"
        constant FixedTiles.bitDepth = 1
    }
}


include "../../engine/untech.inc"

include "_variables.inc"


include "input.inc"
include "healthbar.inc"
include "gameloop.inc"
include "bytecodes.inc"
include "misc.inc"

include "entities/_common.inc"
include "entities/_movement.inc"
include "entities/_player.inc"
include "entities/player-projectile.inc"
include "entities/walk-and-turn.inc"
include "entities/walk-avoid-ledges.inc"

include "interactive-tiles.inc"

include "scenes/basic-room.inc"




au()
iu()
code()
function Main {

    // Set Data Bank to Work RAM
    sep     #$20
a8()
    lda.b   #0x7e
    pha
    plb
// DB = 0x7e

    rep     #$30
a16()
i16()
    jsr     GameState.LoadInitialGameState


    // ::TODO show a screen when loading audio::
    sep     #$20
a8()
    jsl     Audio.Init__far


    // ::TODO start and options screens::


    // Load first room and run the Game Loop
    jmp     GameLoop
}


finalizeMemory()

