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
define REGION = US // NTSC
define ROM_NAME = "RESCUE"
define VERSION = 0

if {defined RELEASE_BUILD} {
    define ROM_SPEED = fast
    define CART_TYPE = romOnly
} else if {defined DEBUG_BUILD} {
    define ROM_SPEED = slow
    define CART_TYPE = romSram
    define CART_RAM_SIZE = 16

    define CONTROLLER_USE_JOY2
} else {
    error "Unknown build"
}

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
        insert FixedTiles, "../gen/font-fixed.2bpp"
        constant FixedTiles.bitDepth = 2
    }
}


include "../../engine/untech.inc"

include "_variables.inc"

if {defined DEBUG_BUILD} {
    include "debug.inc"
}


include "input.inc"
include "healthbar.inc"
include "bytecodes.inc"
include "misc.inc"
include "modes.inc"
include "gameloop.inc"

include "modes/level-name-screen.inc"
include "modes/title-screen.inc"


include "entities/_common.inc"
include "entities/_movement.inc"
include "entities/_player.inc"
include "entities/base-walk-enemy.inc"
include "entities/left-right-projectile.inc"
include "entities/up-diagonal-projectile.inc"
include "entities/player-projectile.inc"
include "entities/jump-around-spawn.inc"
include "entities/jump-towards-player.inc"
include "entities/move-left-right.inc"
include "entities/security-bot.inc"
include "entities/walk-and-turn.inc"
include "entities/walk-avoid-ledges.inc"

include "entities/walking-tank-boss.inc"
include "entities/boss-net.inc"

include "interactive-tiles.inc"

include "scenes/title-screen.inc"
include "scenes/text.inc"
include "scenes/basic-room.inc"
include "scenes/water-room.inc"



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


    // ::SHOULDDO show a screen when loading audio::
    sep     #$20
a8()
    jsl     Audio.Init__far

    rep     #$30
a16()


    if {defined DEBUG_BUILD} {
        DebugSave.LoadGameStateOrBranchIfNoSave(NoSave)
            // Game successfully loaded from Save-RAM

            // Skip title screen and jump straight to game loop
            lda.w   #Mode.GAME
            jmp     SwitchMode
        NoSave:
    }

    wdm #1

    lda.w   #Mode.TITLE_SCREEN
    jmp     SwitchMode
}


finalizeMemory()

