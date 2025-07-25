// metatiles/interactive-tiles.inc
// ===============================
//
// Interactive tiles
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


namespace MetaTiles {

assertPowerOfTwo(Tileset.interactiveTilesMap.N_FUNCTIONS)


// Calls the interactive tile function for a given tile index
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// PARAM: type = the interactive function type to call
// INPUT: dp = entity
// INPUT: Y = map data index
inline __jsr_InteractiveTileFunction(type) {
    assert16a()
    assert16i()

    lda.w   map.data,y
    and.w   #0xff
    tax
    lda.w   Tileset.interactiveTilesMap,x
    and.w   #Tileset.interactiveTilesMap.MASK
    tax

    jsr     (InteractiveTiles.{type}_FunctionTable,x)
}



// Jumps to the interactive tile function for a given tile index
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// PARAM: type = the interactive function type to jump to
// INPUT: dp = entity
// INPUT: Y = map data index
// RETURN: jumps to interactive tile function
inline __jmp_InteractiveTileFunction(type) {
    assert16a()
    assert16i()

    lda.w   map.data,y
    and.w   #0xff
    tax
    lda.w   Tileset.interactiveTilesMap,x
    and.w   #Tileset.interactiveTilesMap.MASK
    tax

    jmp     (InteractiveTiles.{type}_FunctionTable,x)
}



// Move entity, process tile collisions and call any interactive tile functions involving the entity.
//
// This function will call the `InteractiveTiles.Entity*` interactive tile functions.
//
// This function SHOULD NOT be called by a player entity.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// OUTPUT:
//      tileIndexTmp - the x component of the tile index at entity's x-position
//      yCollisionMapIndex - The tile index of the tile that collided in the Y-Axis (only valid if standing).
a16()
i16()
code()
function MoveEntityWithTileCollisionsAndInteractiveTiles {

// Set in `MoveEntityAndProcessTileCollisions__far`
constant tileIndexTmp      = MoveEntityAndProcessTileCollisions__far._tileIndexTmp
allocateTmpWord(yCollisionMapIndex)


    jsl     MoveEntityAndProcessTileCollisions__far
    bcc     NoYCollision
        // Store yCollisionMapIndex
        sty.w   yCollisionMapIndex

        // Collision occurred in the Y-Axis, call the `EntityCollision` tile function
        // Y = map data index of tile that collided with entity
        __jsr_InteractiveTileFunction(EntityCollision)


NoYCollision:
    // Call `EntityAirCollision` tile function on the tile at the entity's position.

    // There is no need to calculate the x component of the map data index.
    // `_tileIndexTmp` contains the x-component the map tile at the entity's xPos.

    lda.b   BaseEntity.yPos.px
    and.w   map.yPositionMask
    assert(METATILE_SIZE_PX == 1 << 4)
    lsr
    lsr
    lsr
    lsr
    ora.w   tileIndexTmp
    tay
    // Y = tile index at entity position

    __jmp_InteractiveTileFunction(EntityAirCollision)
}



// Move player, process tile collisions and call any interactive tile functions involving the player.
//
// This function will call the `InteractiveTiles.Player*` interactive tile functions.
//
// This function MUST NOT be called by a non-player entity.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
a16()
i16()
code()
function MovePlayerWithTileCollisionsAndInteractiveTiles {

// These variables are set in `MoveEntityAndProcessTileCollisions__far` and will be used by the function
constant _tileIndexTmp      = MoveEntityAndProcessTileCollisions__far._tileIndexTmp
constant _yOffsetHeightPair = MoveEntityAndProcessTileCollisions__far._yOffsetHeightPair
constant _leftRightPair     = MoveEntityAndProcessTileCollisions__far._leftRightPair

// Reuse temporary variables from MoveEntityAndProcessTileCollisions__far
constant _originTileIndex   = MoveEntityAndProcessTileCollisions__far._xTmp
constant _counter           = MoveEntityAndProcessTileCollisions__far._counter


    jsl     MoveEntityAndProcessTileCollisions__far
    bcs     +
        jmp     NoYCollision
    +

    // Collision occurred in the Y-Axis.
    // Call the `PlayerOriginCollision` and `PlayerLeftRightCollision` tile functions as necessary.

    // Y = map data index of tile that collided with entity
    // We don't know if the collided tile is at the player's xPosition, so we have to check.

    // Calculate the top-center or bottom-center map tile index
    tya
    and.w   map.dataIndexYmask
    ora.w   _tileIndexTmp
    sta.w   _originTileIndex

    cpy.w   _originTileIndex
    bne     NoOriginCollision
        // There was a collision in the bottom-centre or top-centre of the tile hitbox
        phy
            // Call the `PlayerOriginCollision` tile function
            __jsr_InteractiveTileFunction(PlayerOriginCollision)
        ply


NoOriginCollision:
    // Call `PlayerLeftRightCollision` tile function on the tiles to the left or right of `_originTileIndex`.

    // This code assumes only SOLID tiles have a non-blank `PlayerLeftRightCollision` function.
    //
    // This allows me an optimisation where the solidity of the left/right tiles can be ignored as
    // non-solid tiles have no `PlayerLeftRightCollision` behaviour.


    sep     #$20
a8()
    lda.b   BaseEntity.xPos.px
    and.b   #0x0f
    sec
    sbc.w   _leftRightPair
    rep     #$30
a16()
    bcs     NoTilesOnLeft
        // A is negative
        ora.w   #0xff00
        cmp.w   #-METATILE_SIZE_PX
        bcs     OnlyOneTileOnLeft

            // Two tiles on left
            lda.w   _originTileIndex
            sec
            sbc.w   map.bytesPerColumn
            sec
            sbc.w   map.bytesPerColumn
            tay

            __jsr_InteractiveTileFunction(PlayerLeftRightCollision)

    OnlyOneTileOnLeft:
        // Carry clear
        lda.w   _originTileIndex
        sec
        sbc.w   map.bytesPerColumn
        tay

        __jsr_InteractiveTileFunction(PlayerLeftRightCollision)

        sec

    // carry set
NoTilesOnLeft:
    lda.b   BaseEntity.xPos.px
    and.w   #0xf
    // carry set (+1)
    adc.w   _leftRightPair + 1
    cmp.w   #METATILE_SIZE_PX + 2
    bcc     NoTilesOnRight
        cmp.w   #METATILE_SIZE_PX * 2 + 2
        bcc     OnlyOneTileOnRight

            // two tiles on right
            lda.w   map.bytesPerColumn
            asl
            clc
            adc.w   _originTileIndex
            tay

            __jsr_InteractiveTileFunction(PlayerLeftRightCollision)

    OnlyOneTileOnRight:
        lda.w   _originTileIndex
        clc
        adc.w   map.bytesPerColumn
        tay

        __jsr_InteractiveTileFunction(PlayerLeftRightCollision)

    NoTilesOnRight:


NoYCollision:
    // Call `PlayerAirCollision` functions on all tiles between the top-center and bottom-center of tile hitbox.

    // NOTE: If I modify this code, also modify `MoveEntityAndProcessTileCollisions__far` `MovingLeft` and `MovingRight`
    // _top = MS.TileHitbox[x].yOffset + entity.yPos.px
    lda.w   _yOffsetHeightPair
    ora.w   #0xff00                 // yOffset is always a negative value
    clc
    adc.b   BaseEntity.yPos.px
    tax

    and.w   map.yPositionMask
    assert(METATILE_SIZE_PX == 1 << 4)
    lsr
    lsr
    lsr
    lsr
    ora.w   MoveEntityAndProcessTileCollisions__far._tileIndexTmp
    tay
    // Y = tile index at top-centre of player's tile hitbox

    // Determine number of tiles to test
    // _counter = (_top & 0xf + _yOffsetHeightPair.height - 1) / 16
    // X = top
    txa
    and.w   #0xf
    clc
    adc.w   _yOffsetHeightPair + 1      // Third byte of _yOffsetHeightPair is 0
    dec         // ::TODO - required, maybe make height `value - 1`::
    assert(METATILE_SIZE_PX == 1 << 4)
    lsr
    lsr
    lsr
    lsr
    beq     OnlyOneAirTile

    sta.w   _counter

    Loop:
        phy
            __jsr_InteractiveTileFunction(PlayerAirCollision)
        ply
        iny
        dec.w   _counter
        bne     Loop

OnlyOneAirTile:
    __jmp_InteractiveTileFunction(PlayerAirCollision)
}

}



// A blank (unused) interactive tile function
namespace InteractiveTiles {
namespace NoTileInteraction {

a16()
i16()
code()
EntityCollision:
EntityAirCollision:
PlayerOriginCollision:
PlayerLeftRightCollision:
PlayerAirCollision:
function NullFunction {
    rts
}

}
}


// vim: ft=bass-65816 ts=4 sw=4 et:

