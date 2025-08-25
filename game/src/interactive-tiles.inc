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

namespace InteractiveTiles {

constant NullFunction = InteractiveTiles.NoTileInteraction.NullFunction

constant LADDER_INTERACTIVE_TILES_INDEX = 6


namespace PlayerActivateScriptTrigger {

constant EntityCollision = NullFunction
constant EntityAirCollision = NullFunction
constant PlayerLeftRightCollision = NullFunction
constant PlayerAirCollision = NullFunction


// Activate a room script if the player is standing on the tile
// and the up button is pressed.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: Y = map data index
a16()
i16()
code()
function PlayerOriginCollision {
    lda.w   Controller.Joy1.pressed
    bit.w   #JOY.up
    beq     +
        jmp     InteractiveTiles.PlayerScriptTrigger.PlayerOriginCollision
    +
    rts
}

}


namespace Ladder {

constant EntityCollision = NullFunction
constant EntityAirCollision = NullFunction

// Must be blank if the tile is not solid
constant PlayerLeftRightCollision = NullFunction


constant PlayerOriginCollision = Entities.Players.Player.PlayerLadderTileCollision
constant PlayerAirCollision = Entities.Players.Player.PlayerLadderTileCollision

}

}
