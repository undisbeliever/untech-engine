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

namespace UpGravity {

constant EntityCollision = NullFunction
constant PlayerLeftRightCollision = NullFunction
constant PlayerOriginCollision = NullFunction


// Called when a tile-interacting player hovers over the tile.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: Y = map data index
a16()
i16()
code()
function PlayerAirCollision {
    sep     #$20
a8()
    lda.b   BaseEntity.movementState
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    bne     +
        and.b   #~Entity.MovementState.STANDING_FLAG
        ora.b   #Entity.MovementState.UP_GRAVITY_FLAG
        sta.b   BaseEntity.movementState

        lda.b   #SFX.playerUpGravity
        jsr     Audio.QueueSoundEffect__DpUnknown
    +

    rep     #$30
a16()

    rts
}


// Called when a tile-interacting entity hovers over the tile.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: Y = map data index
a16()
i16()
code()
function EntityAirCollision {
    sep     #$20
a8()
    lda.b   BaseEntity.movementState
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    bne     +
        and.b   #~Entity.MovementState.STANDING_FLAG
        ora.b   #Entity.MovementState.UP_GRAVITY_FLAG
        sta.b   BaseEntity.movementState

        lda.b   #SFX.enemyUpGravity
        jsr     Audio.QueueSoundEffect__DpUnknown
    +

    rep     #$30
a16()

    rts
}

}


namespace DownGravity {

constant EntityCollision = NullFunction
constant PlayerLeftRightCollision = NullFunction
constant PlayerOriginCollision = NullFunction


// Called when a tile-interacting player hovers over the tile.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: Y = map data index
a16()
i16()
code()
function PlayerAirCollision {
    sep     #$20
a8()
    lda.b   BaseEntity.movementState
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    beq     +
        and.b   #~(Entity.MovementState.UP_GRAVITY_FLAG | Entity.MovementState.STANDING_FLAG)
        sta.b   BaseEntity.movementState

        lda.b   #SFX.playerDownGravity
        jsr     Audio.QueueSoundEffect__DpUnknown
    +

    rep     #$30
a16()

    rts
}


// Called when a tile-interacting entity hovers over the tile.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: Y = map data index
a16()
i16()
code()
function EntityAirCollision {
    sep     #$20
a8()
    lda.b   BaseEntity.movementState
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    beq     +
        and.b   #~(Entity.MovementState.UP_GRAVITY_FLAG | Entity.MovementState.STANDING_FLAG)
        sta.b   BaseEntity.movementState

        lda.b   #SFX.enemyDownGravity
        jsr     Audio.QueueSoundEffect__DpUnknown
    +

    rep     #$30
a16()

    rts
}

}

}
