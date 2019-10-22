// metatiles/collisions.inc
// ========================
//
// MetaTile-entity collision code.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// ::KUDOS Rodrigo Monteiro, for blogging about tile collisions ::
// http://higherorderfun.com/blog/2012/05/20/the-guide-to-implementing-2d-platformers/

// ::KUDOS danjb, for blogging about tile platformer slope collisions ::
// http://danjb.com/game_dev/tilebased_platformer_slopes


namespace MetaTiles {

// Bitfield containing the collisions that occurred in the
// last `MoveEntityAndProcessTileCollisions` call.
//
// See BaseEntity.tileCollisionState.
namespace TileCollisionState {
    constant DOWN_COLLISION  = 0x80
    constant UP_COLLISION    = 0x40
    constant RIGHT_COLLISION = 0x20
    constant LEFT_COLLISION  = 0x10
}


code()
namespace MoveEntityAndProcessTileCollisions {
// DP = entity
a16()
i16()
NoHitbox:
    // X position already updated

    // add entity Y velocity to Y position
    clc
    lda.b   BaseEntity.yVecl
    adc.b   BaseEntity.yPos.sx
    sta.b   BaseEntity.yPos.sx
    lda.b   BaseEntity.yVecl + 2
    adc.b   BaseEntity.yPos.px
    sta.b   BaseEntity.yPos.px

    // no tile collision
    ldy.w   #0

    rts
}


// Applies velocity to the entity's position and checks for tile collisions.
//
// If a collison occurs then the entity is moved so its tile hitbox is outside
// the collision and the approperate bits in `BaseEntity.tileCollisionState`
// will be set.
//
// You SHOULD NOT modify the value of `BaseEntity.tileCollisionState` unless
// the entity has been moved (in which case it should be reset to 0).
//
// NOTE: you MUST ONLY call this routine inside the entity's Process routine
//
// NOTE: Collision testing is only preformed in the direction of movement.
//       For instance, if the entity is moving right then a left collision test
//       is not preformed.
//
// NOTE: `MoveEntityAndProcessTileCollisions` will only preform a y-axis test
//       on the first 32 pixels to the left and right of the entity's xPos.
//
//       The x-axis test will be preformed using the full width of the tile hitbox.
//
// INPUT: dp = entity
// OUTPUT: Y = The tilemap index directly above/below the entity (depending on yVecl),
//             Set to 0 if there is no tile hitbox.
//        BaseEntity.tileCollisionState = The collisions that occurred in this frame.
//
// This code assumes:
//   * This routine is called once per frame.
//   * The maximum x/y velocity is 8px/frame.
//   * The entity is inside the map boundary.
//   * An END_SLOPE tile or a SOLID wall is adjacent to the long side of a *_SLOPE tile.
//   * An EMPTY tile or a SOLID wall is adjacent to the short side of a *_SLOPE tile.
//   * A SOLID, UP_PLATFORM or END_SLOPE tile is below the DOWN_*_SLOPE tiles.
//   * A SOLID, DOWN_PLATFORM or END_SLOPE tile is above the UP_*_SLOPE tiles.
//   * An EMPTY tile is above a DOWN_*_SLOPE tiles.
//   * An EMPTY tile is below a UP_*_SLOPE tiles.
//   * An EMPTY tile is *not* to the left or right of an END_SLOPE time.
//   * An UP_PLATFORM tile is only one tile tall and not touching the floor.
//   * An DOWN_PLATFORM tile is only one tile tall and not touching the ceiling.
//   * A short/tall slope pair is never broken, unless there is a SOLID wall in the way.
//   * It is not possible to walk into the long side of a slope tile.
//   * The top-left tile of the map is SOLID.
//
// If none of these assumptions are met then the entity can either:
//   * Overshoot slope tiles when `abs(xVecl)` > 1px/frame.
//   * Fall through a slope if the x origin tile is EMPTY.
//
//
// NOTE: a *SOLID wall* means the tile adjacent and adjacent-above the slope is solid.
//
//       For example, the following tile arrangements are valid.
//             ◼     ◼
//            ◣◼◢   ◢◼◣
//           ◼◼◼◼◼◼◼◼◼◼◼
//
//       Whereas the following are forbidden:
//
//            ◣◼◢   ◢◼◣
//           ◼◼◼◼◼◼◼◼◼◼◼
//
a16()
i16()
code()
function MoveEntityAndProcessTileCollisions {

assert(METATILE_SIZE_PX == 16)

// Confirm velocity is a 32 bit value
assert(BaseEntity.yVecl - BaseEntity.xVecl == 4)

assert(MetaSprite.Format.Frame.TileHitbox.left + 1 == MetaSprite.Format.Frame.TileHitbox.right)
assert(MetaSprite.Format.Frame.TileHitbox.yOffset + 1 == MetaSprite.Format.Frame.TileHitbox.height)

// Third byte of _leftRightPair and _yOffsetHeightPair is 0
constant _leftRightPair = MetaSprite.EntityHitbox._xPosWidthPair
constant _yOffsetHeightPair = MetaSprite.EntityHitbox._yPosHeightPair

allocateTmpWord(_top)
allocateTmpWord(_oldTileCollisionState)

allocateTmpWord(_counter)
allocateTmpWord(_xTmp)

allocateTmpWord(_bottom)
allocateTmpWord(_yDelta)
allocateTmpWord(_bottomOffset)
allocateTmpWord(_edgeOnPlatform)

constant _HeightTableIndex_FIRST_UP_SLOPE   = HeightTableIndex.UP_RIGHT_SLOPE
constant _HeightTableIndex_LAST_UP_SLOPE    = HeightTableIndex.UP_LEFT_SHORT_SLOPE
constant _HeightTableIndex_FIRST_DOWN_SLOPE = HeightTableIndex.DOWN_RIGHT_SLOPE
constant _HeightTableIndex_LAST_DOWN_SLOPE  = HeightTableIndex.DOWN_LEFT_SHORT_SLOPE


    // clear tileCollisionState
    sep     #$20
a8()
    lda.b   BaseEntity.tileCollisionState
    sta.w   _oldTileCollisionState
    stz.b   BaseEntity.tileCollisionState


//  X AXIS
// --------
//
// X-Axis movement can only collide with a SOLID tile, all other tile collision types are ignored.


    // add entity X velocity to X position
    rep     #$31
a16()
    // carry clear
    lda.b   BaseEntity.xVecl
    adc.b   BaseEntity.xPos.sx
    sta.b   BaseEntity.xPos.sx
    lda.b   BaseEntity.xVecl + 2
    adc.b   BaseEntity.xPos.px
    sta.b   BaseEntity.xPos.px


    // Calculate tile index for the top of the tile hitbox (if any)

    ldx.b   BaseEntity.MetaSprite.currentFrame
    // currentFrame is never null
    lda.l   MetaSprite.Format.Frame.TileHitbox.yOffset,x
    bmi     NoHitbox
    sta.w   _yOffsetHeightPair

    // _top = MS.TileHitbox[x].yOffset + entity.yPos.px
    // RDMPY = (_top - map.TOP) / 16 * map.width
    ora.w   #0xff00                 // yOffset is always a negative value
    clc
    adc.b   BaseEntity.yPos.px
    sta.w   _top
    sec
    sbc.w   #map.TOP
    lsr
    lsr
    lsr
    lsr
    sep     #$20
a8()
    xba
    lda.w   map.width

    rep     #$31        // also clear carry
a16()
    sta.l   WRMPYA      // a = map.width, b = top / 16


    // MUST NOT CHANGE CARRY HERE
    lda.l   MetaSprite.Format.Frame.TileHitbox.left,x
    sta.w   _leftRightPair

    // MUST NOT CHANGE CARRY HERE
    // MUST NOT CHANGE A HERE

    // Check if moving left/right
    ldy.b   BaseEntity.xVecl + 2
    bmi     MovingLeft


a16()
i16()
MovingRight:
    namespace MovingRight {
        // Calculate tile index for top-right of tile hitbox

        // _xTmp = entity.xPos.px + _leftRightPair.right
        // Y = (_xTmp - map.LEFT) / 16 + RDMPY
        lda.b   BaseEntity.xPos.px
        // carry clear
        adc.w   _leftRightPair + 1          // Third byte of _leftRightPair is 0
        sta.w   _xTmp
        sec
        sbc.w   #map.LEFT
        lsr
        lsr
        lsr
        lsr
        clc
        adc.l   RDMPY
        tay

        // set Ah to 0 for `tax` below
        lda.w   #0


        // Goto Collision if tile index is a SOLID tile
        sep     #$20
    a8()
        // if collisionMap[map.data[y]] == HeightTableIndex.SOLID goto Collision
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        cmp.b   #HeightTableIndex.SOLID
        beq     Collision


        // Test the remaining tiles (Goto Collision if right edge touches a SOLID tile)

        // _counter = (_top & 0xf + _yOffsetHeightPair.height - 1) / 16
        lda.w   _top
        and.b   #0xf
        clc
        adc.w   _yOffsetHeightPair + 1      // Third byte of _yOffsetHeightPair is 0
        dec         // ::TODO - required, maybe make height `value - 1`::
        lsr
        lsr
        lsr
        lsr
        beq     NoCollision

        sta.w   _counter

        rep     #$30    // Transfer 16 bits in `tya`
    i16()
    a16()
        tya
    au()
    iu()
        Loop:
            // A = previous tile index
            rep     #$31
        i16()
        a16()
            // carry clear
            adc.w   map.bytesPerRow
            tay

            ldx.w   map.data,y

            sep     #$30
        a8()
        i8()
            ldy.w   collisionMap,x
            cpy.b   #HeightTableIndex.SOLID
            beq     Collision

            dec.w   _counter
            bne     Loop


    a8()
    iu()
    NoCollision:
        rep     #$30
    a16()
    i16()
        jmp     EndXaxis_A16


    a8()
    iu()
    Collision:
        // The right edge of the tile hitbox touched a SOLID tile

        // set collision flag
        lda.b   #TileCollisionState.RIGHT_COLLISION
        tsb.b   BaseEntity.tileCollisionState

        // entity.xPos.px = (_xTmp & 0xfff0) + _leftRightPair.right
        rep     #$30
    a16()
    i16()
        lda.w   _xTmp
        and.w   #0xfff0
        sec
        sbc.w   _leftRightPair + 1          // 3rd byte of pair is 0
        sta.b   BaseEntity.xPos.px

        // Reset xVecl
        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2

        jmp     EndXaxis_A16
    }


a16()
i16()
MovingLeft:
    namespace MovingLeft {
        // Calculate tile index for top-left of tile hitbox

        // _xTmp = entity.xPos.px - _leftRightPair.left
        // Y = (_xTmp - map.LEFT) / 16 + RDMPY

        // A = _leftRightPair
        and.w   #0x00ff
        eor.w   #0xffff
        sec
        adc.b   BaseEntity.xPos.px
        sta.w   _xTmp
        sec
        sbc.w   #map.LEFT
        lsr
        lsr
        lsr
        lsr
        clc
        adc.l   RDMPY
        tay

        // set Ah to 0 for `tax` below
        lda.w   #0


        // Goto Collision if tile index is a SOLID tile
        sep     #$20
    a8()
        // if collisionMap[map.data[y]] == HeightTableIndex.SOLID goto Collision
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        cmp.b   #HeightTableIndex.SOLID
        beq     Collision


        // Test the remaining tiles (Goto Collision if left edge touches a SOLID tile)

        // _counter = (_top & 0xf + _yOffsetHeightPair.height - 1) / 16
        lda.w   _top
        and.b   #0xf
        clc
        adc.w   _yOffsetHeightPair + 1      // Third byte of _yOffsetHeightPair is 0
        dec         // ::TODO - required, maybe make height `value - 1`::
        lsr
        lsr
        lsr
        lsr
        beq     NoCollision

        sta.w   _counter

        rep     #$30    // Transfer 16 bits in `tya`
    i16()
    a16()
        tya

        au()
        iu()
        Loop:
            // A = previous tile index
            rep     #$31
        i16()
        a16()
            // carry clear
            adc.w   map.bytesPerRow
            tay

            ldx.w   map.data,y

            sep     #$30
        a8()
        i8()
            ldy.w   collisionMap,x
            cpy.b   #HeightTableIndex.SOLID
            beq     Collision

            dec.w   _counter
            bne     Loop

    a8()
    iu()
    NoCollision:
        rep     #$30
    a16()
    i16()
        bra     EndXaxis_A16


    a8()
    iu()
    Collision:
        // The right edge of the tile hitbox touched a SOLID tile

        // set collision flag
        lda.b   #TileCollisionState.LEFT_COLLISION
        tsb.b   BaseEntity.tileCollisionState

        // entity.xPos.px += (~_xTmp & 0xf) + 1;
        lda.w   _xTmp
        eor.b   #0x0f

        rep     #$30
    a16()
    i16()
        and.w   #0x000f
        sec
        adc.b   BaseEntity.xPos.px
        sta.b   BaseEntity.xPos.px

        // Reset xVecl
        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2
    }
a16()
i16()
EndXaxis_A16:



//  Y AXIS
// --------

// The collision tests preformed on the y-axis depend on the state of
// the `BaseEntity.tileCollisionState` and the direction of movement.
//
// NOMENCLATURE: center tile means the tile above or below the entity's xPos.

    ldy.b   BaseEntity.yVecl + 2
    bpl     ++
        // Moving Upwards
        assert(TileCollisionState.UP_COLLISION == 0x40)
        bit.w   _oldTileCollisionState - 1
        bvs     +
            jmp     MovingUp
        +
        jmp     MovingUp_Standing
    +

    // Moving downwards
    assert(TileCollisionState.DOWN_COLLISION == 0x80)
    bit.w   _oldTileCollisionState - 1
    bpl     +
        jmp     MovingDown_Standing
    +


a16()
i16()
MovingDown:
    // Y = hiword(BaseEntity.yVecl)
    namespace MovingDown {
        // Entity is moving down and was not standing on the floor in the
        // previous `MoveEntityAndProcessTileCollisions` call.
        //
        // The code first tests the tile on the hitbox's bottom edge at the
        // entity's xPos (refered to as the bottom-center tile).
        //
        // If the bottom-center tile is empty we then test the bottom
        // left/right tiles for a SOLID or UP_PLATFORM collision.  Only two
        // tiles to the left and two to the right are tested.  If the tile
        // hitbox's left/right values are > 32 the remaining tiles will be
        // ignored.
        //
        // If the tile being tested is an UP_PLATFORM then collision only
        // occurs if the entity was above (or on top of) the tile before the
        // `MoveEntityAndProcessTileCollisions` call.
        //
        // If there is a collision then the entity will be moved one pixel
        // above the tile's collision area.
        //
        // Only the bottom-center tile is tested for slope collisions, which is
        // not an issue as the engine requires all down slope tiles to have a
        // SOLID or UP_PLATFORM tile underneath them.
        //
        // The END_SLOPE tile is only solid in the bottom-center tile.
        // There is no need to write code to test for END_SLOPE
        // `BottomHeightTable[END_SLOPE | (xPos & 0xf)]` is always 0.


        // verify there is no need to test for END_SLOPE in this code.
        assert(HeightTableIndex.END_SLOPE & 0xf0 == HeightTableIndex.SOLID)


        // add entity Y velocity to Y position, saving the change in Ypos to _yDelta
        clc
        lda.b   BaseEntity.yVecl
        adc.b   BaseEntity.yPos.sx
        sta.b   BaseEntity.yPos.sx

        // Y = hiword(BaseEntity.yVecl)
        tya
        adc.w   #0
        sta.w   _yDelta

        clc
        adc.b   BaseEntity.yPos.px
        sta.b   BaseEntity.yPos.px


        // Calculate tile index for bottom-center of tile hitbox

        // _bottom = _yOffsetHeightPair.yOffset + _yOffsetHeightPair.height + entity.yPos.px
        // Y = (_bottom - map.TOP) / 16 * map.width + (entity.xPos.px - map.LEFT) / 16
        lda.w   _yOffsetHeightPair
        ora.w   #0xff00                 // yOffset is always negative value
        clc
        adc.w   _yOffsetHeightPair + 1  // Third byte of _yOffsetHeightPair is 0
        clc
        adc.b   BaseEntity.yPos.px
        sta.w   _bottom
        sec
        sbc.w   #map.TOP
        lsr
        lsr
        lsr
        lsr
        sep     #$20
    a8()
        sta.l   WRMPYB  // WRMPYA set in X axis test

        rep     #$31
    a16()

        lda.b   BaseEntity.xPos.px
        // carry clear
        sbc.w   #map.LEFT - 1
        lsr
        lsr
        lsr
        lsr
        clc
        adc.l   RDMPY
        tay
        // Y = entity bottom-center map tile

        // set Ah to 0 for the `tax`s below
        lda.w   #0


        // Test to see if the bottom-center of the tile hitbox is inside the tile collision area.
        sep     #$20
    a8()
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        assert(HeightTableIndex.EMPTY == 0)
        beq     TestLeftRightTiles
            cmp.b   #HeightTableIndex.UP_PLATFORM
            beq     CenterTileIsUpPlatform

                //  x = (entity.xPos.px & 0xf) | collisionMap[x]
                //  if (_bottom & 0xf) >= TopHeightTable[x]:
                //      goto Collision
                //  else:
                //      return
                lda.b   BaseEntity.xPos.px
                and.b   #0x0f
                ora.w   collisionMap,x
                tax

                lda.w   _bottom
                and.b   #0xf
                cmp.l   TopHeightTable,x
                bcs     Collision
                    // Entity has not yet landed on the slope tile, no need to test left/right tiles.

                    // Y = entity bottom-center map tile
                    rep     #$30
                a16()
                    rts

    // ::SPAGHETTI Have to put Collision here to prevent a "branch out of bounds" assembly error::

    a8()
    i16()
    Collision:
        // Entity is inside the collision area of a tile.
        //
        // Ah = 0
        // X = TopHeightTable index
        // Y = entity bottom-center map tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingDown_Standing.CollisionBelow`

        // set collision flag
        lda.b   #TileCollisionState.DOWN_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Calculate bottom offset.
        //
        // I have decided it is faster to calculate the Tile Hitbox bottom
        // offset on a down collision (with an 8 bit A) than calculating the
        // Tile Hitbox height on every X-axis test (with a 16 bit A).

        // _bottomOffset = _yOffsetHeightPair.height + _yPosHeightPair.yOffset
        lda.w   _yOffsetHeightPair + 1
        clc
        adc.w   _yOffsetHeightPair      // yOffset is always negative value

        rep     #$30
    a16()
        // Ah = 0
        sta.w   _bottomOffset


        // Move entity to one pixel above the tile's TopHeightTable value
        //
        // BaseEntity.yPos.px = ((_bottom & 0xfff0) | TopHeightTable[x]) - _bottomOffset - 1
        lda.w   _bottom
        and.w   #0xfff0

        sep     #$20
    a8()
        ora.l   TopHeightTable,x

        rep     #$21
    a16()
        // carry clear (-1 to place entity above solid tiles)
        sbc.w   _bottomOffset
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx


        // Reset yVecl
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        rts


    a8()
    i16()
    // Ah = 0
    CenterTileIsUpPlatform:
        // if entity was above the UP_PLATFORM in the previous frame:
        //      goto SolidCollision
        // else:
        //      goto TestLeftRightTiles

        lda.w   _bottom
        and.b   #0xf
        beq     SolidCollision
        cmp.w   _yDelta
        bcc     SolidCollision


    a8()
    i16()
    TestLeftRightTiles:
        // Entity bottom-center tile is empty.

    inline TestLeftRightTiles(platform, SolidCollisionLabel, NoCollisionLabel) {
        // Test for solid/platform tiles to the left / right of xPos.
        //
        // the top/bottom left/right can only collide with a SOLID or {platform} tiles.
        //
        // INPUT: y = center tile
        // INPUT: Ah = 0
        // BRANCH to `SolidCollisionLabel` if the test hits is a SOLID tile
        // BRANCH to `NoCollisionLabel` if the test does not find a SOLID or {platform} tile.
        // NO BRANCH if the test touches a {platform} tile and does not hit a SOLID tile
        assert8a()
        assert16i()

        // _edgeOnPlatform = 0
        // for each left/right tile:
        //      a = collisionMap[map.data[y + offset]]
        //      if a == PlatformIndex.SOLID:
        //          goto {SolidCollisionLabel}
        //      if a == {platform}:
        //          _edgeOnPlatform = {platform}
        //
        // if _edgeOnPlatform == 0:
        //      goto {NoCollisionLabel}

        // _edgeOnPlatform will be set to a non-zero value if the tile is a UP_PLATFORM
        assert(HeightTableIndex.{platform} != 0)
        stz.w   _edgeOnPlatform

        lda.b   BaseEntity.xPos.px
        and.b   #0x0f
        sec
        sbc.w   _leftRightPair
        bcs     NoTilesOnLeft
            // A is negative
            cmp.b   #-METATILE_SIZE_PX
            bcs     OnlyOneTileOnLeft
                // Two tiles on left
                lda.w   map.data - 2,y
                tax
                lda.w   collisionMap,x
                cmp.b   #HeightTableIndex.SOLID
                beq     {SolidCollisionLabel}
                cmp.b   #HeightTableIndex.{platform}
                bne     +
                    sta.w   _edgeOnPlatform
                +

        OnlyOneTileOnLeft:
            lda.w   map.data - 1,y
            tax
            lda.w   collisionMap,x
            cmp.b   #HeightTableIndex.SOLID
            beq     {SolidCollisionLabel}
            cmp.b   #HeightTableIndex.{platform}
            bne     +
                sta.w   _edgeOnPlatform
            +
            sec

        // carry set
    NoTilesOnLeft:
        lda.b   BaseEntity.xPos.px
        and.b   #0xf
        // carry set (+1)
        adc.w   _leftRightPair + 1
        cmp.b   #16 + 2
        bcc     NoTilesOnRight
            cmp.b   #32 + 2
            bcc     OnlyOneTileOnRight
                // two tiles on right
                lda.w   map.data + 2,y
                tax
                lda.w   collisionMap,x
                cmp.b   #HeightTableIndex.SOLID
                beq     {SolidCollisionLabel}
                cmp.b   #HeightTableIndex.{platform}
                bne     +
                    sta.w   _edgeOnPlatform
                +

        OnlyOneTileOnRight:
            lda.w   map.data + 1,y
            tax
            lda.w   collisionMap,x
            cmp.b   #HeightTableIndex.SOLID
            beq     {SolidCollisionLabel}
            cmp.b   #HeightTableIndex.{platform}
            beq     OnPlatformTile

    NoTilesOnRight:
        assert(HeightTableIndex.{platform} != 0)
        lda.w   _edgeOnPlatform
        beq     {NoCollisionLabel}

    OnPlatformTile:
    }
        TestLeftRightTiles(UP_PLATFORM, SolidCollision, NoCollision)

            // bottom left/right is touching a DOWN_PLATFORM tile

            // goto SolidCollision if entity was above the UP_PLATFORM in the previous frame
            lda.w   _bottom
            and.b   #0xf
            beq     SolidCollision
            cmp.w   _yDelta
            bcc     SolidCollision

    a8()
    i16()
    NoCollision:
        // Y = entity bottom-center map tile
        rep     #$30
    a16()
        rts


    a8()
    i16()
    SolidCollision:
        // Entity collided with a SOLID or PLATFORM tile.
        //
        // Ah = 0
        // Y = entity bottom-center map tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingDown_Standing.SolidCollisionBelow`

        // set collision flag
        lda.b   #TileCollisionState.DOWN_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel above the current tile
        //
        // _bottomOffset = _yOffsetHeightPair.height + _yPosHeightPair.yOffset
        // BaseEntity.yPos.px = (_bottom & 0xfff0) - _bottomOffset - 1
        lda.w   _yOffsetHeightPair + 1
        clc
        adc.w   _yOffsetHeightPair      // yOffset is always negative value

        rep     #$31
    a16()
        // Ah = 0
        sta.w   _bottomOffset

        lda.w   _bottom
        and.w   #0xfff0
        // carry clear (-1 to place entity above solid tiles)
        sbc.w   _bottomOffset
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx


        // Reset yVecl
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        rts
    }


a16()
i16()
MovingDown_Standing:
    namespace MovingDown_Standing {
        // Entity is moving down and was standing on the floor of a
        // slope/platform/solid on the previous
        // `MoveEntityAndProcessTileCollisions` call.
        //
        // The Y velocity is not processed when standing.  The entity's Y
        // position will be set so it is always 1 pixel above the floor height.
        //
        // When ascending the slope the hitbox bottom is already on the correct
        // tile so the collision test passes.
        //
        // The routine will then test up to five tiles below the hitbox's
        // bottom.  We test the tile below the hitbox for two reasons:
        //    1. The previous call to `MoveEntityAndProcessTileCollisions`
        //    moved the entity so it is one pixel above the floor.
        //    2. It allows the entity to gracefully walk/run down slopes.
        //
        // The bottom left/right tile collision tests will only be test if the
        // entity collides with a SOLID or UP_PLATFORM tile.  This is not an
        // issue as there all down slope tiles have a SOLID or UP_PLATFORM tile
        // underneath them.
        //
        // The bottom left/right corner tests will only check two tiles to the
        // left and two tiles to the right of the X position.  If the tile
        // hitbox's left/right values are > 32 the remaining tiles will be
        // ignored.
        //
        // As an optimisation this code treats UP_PLATFORM tiles as solid.
        // Unfortunately this design choice will cause the entity to zip
        // upwards if the entity walks into an UP_PLATFORM.
        //
        // There is no need to write code to test for END_SLOPE, as
        // `BottomHeightTable[END_SLOPE | (xPos & 0xf)]` is always 0.


        // Reset yVecl
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2


        // Calculate tile index for bottom-center of tile hitbox

        // _bottom = _yOffsetHeightPair.yOffset + _yOffsetHeightPair.height + entity.yPos.px
        // Y = (_bottom - map.TOP) / 16 * map.width + (entity.xPos.px - map.LEFT) / 16
        lda.w   _yOffsetHeightPair
        ora.w   #0xff00                 // yOffset is always negative value
        clc
        adc.w   _yOffsetHeightPair + 1  // Third byte of _yOffsetHeightPair is 0
        clc
        adc.b   BaseEntity.yPos.px
        sta.w   _bottom
        sec
        sbc.w   #map.TOP
        lsr
        lsr
        lsr
        lsr
        sep     #$21    // also set carry
    a8()
        sta.l   WRMPYB  // WRMPYA set in X axis test

        rep     #$30
    a16()
        lda.b   BaseEntity.xPos.px
        // carry set
        sbc.w   #map.LEFT
        lsr
        lsr
        lsr
        lsr
        clc
        adc.l   RDMPY
        tay
        // Y = entity bottom-center map tile

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        sep     #$20
    a8()

        // Check to see if we are about to enter a new slope.
        //
        // This test must jump to `MovingDown.Collision` if the entity
        // is on-top of a UP_PLATFORM or a downwards slope to prevent
        // the entity from overshooting a slope when xVecl is too large.

        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        assert(HeightTableIndex.EMPTY == 0)
        beq     +
        cmp.b   #HeightTableIndex.DOWN_PLATFORM
        beq     +
            lda.b   BaseEntity.xPos.px
            and.b   #0x0f
            ora.w   collisionMap,x
            tax
            jmp     MovingDown.Collision
        +


        // Test tile below the current one

        // Y -= map.bytesPerRow
        rep     #$31
    a16()
        tya
        // carry clear
        adc.w   map.bytesPerRow
        tay

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        // Goto CollisionBelow if tile below the entity is not EMPTY or a DOWN_PLATFORM
        sep     #$20
    a8()
        lda.w   map.data,y
        tax

        lda.w   collisionMap,x
        assert(HeightTableIndex.EMPTY == 0)
        beq     +
        cmp.b   #HeightTableIndex.DOWN_PLATFORM
        beq     +
            // not EMPTY or DOWN_PLATFORM
            lda.b   BaseEntity.xPos.px
            and.b   #0xf
            ora.w   collisionMap,x
            tax
            bra     CollisionBelow
        +


    inline TestLeftRightTiles(platform, SolidCollisionLabel) {
        // Test for collisions to the left / right of xPos.
        //
        // the top/bottom left/right can only collide with a SOLID or {platform} tiles.
        //
        // NOTE: we are not reusing `MovingDown.TestLeftRightTiles` as
        //       we treat platforms as solids when walking up/down.
        //
        // INPUT: Y = center tile
        // INPUT: Ah = 0
        // BRANCH to `SolidCollisionLabel` if the test hits is a SOLID or {platform} tile
        // NO BRANCH if test does not find a SOLID or {platform} tile
        assert8a()
        assert16i()

        // for each left/right tile:
        //      a = collisionMap[map.data[y + offset]]
        //      if a == SOLID or a == {platform}:
        //          goto {SolidCollisionLabel}
        //
        // if _edgeOnPlatform == 0:
        //      goto {NoCollisionLabel}

        lda.b   BaseEntity.xPos.px
        and.b   #0x0f
        sec
        sbc.w   _leftRightPair
        bcs     NoTilesOnLeft
            // A is negative
            cmp.b   #-METATILE_SIZE_PX
            bcs     OnlyOneTileOnLeft
                // two tiles on left
                lda.w   map.data - 2,y
                tax
                lda.w   collisionMap,x
                cmp.b   #HeightTableIndex.SOLID
                beq     {SolidCollisionLabel}
                cmp.b   #HeightTableIndex.{platform}
                beq     {SolidCollisionLabel}

        OnlyOneTileOnLeft:
            lda.w   map.data - 1,y
            tax
            lda.w   collisionMap,x
            cmp.b   #HeightTableIndex.SOLID
            beq     {SolidCollisionLabel}
            cmp.b   #HeightTableIndex.{platform}
            beq     {SolidCollisionLabel}

            sec

        // carry set
    NoTilesOnLeft:
        lda.b   BaseEntity.xPos.px
        and.b   #0xf
        // carry set (+1)
        adc.w   _leftRightPair + 1
        cmp.b   #16 + 2
        bcc     NoTilesOnRight
            cmp.b   #32 + 2
            bcc     OnlyOneTileOnRight
                // two tiles on right
                lda.w   map.data + 2,y
                tax
                lda.w   collisionMap,x
                cmp.b   #HeightTableIndex.SOLID
                beq     {SolidCollisionLabel}
                cmp.b   #HeightTableIndex.{platform}
                beq     {SolidCollisionLabel}

        OnlyOneTileOnRight:
            lda.w   map.data + 1,y
            tax
            lda.w   collisionMap,x
            cmp.b   #HeightTableIndex.SOLID
            beq     {SolidCollisionLabel}
            cmp.b   #HeightTableIndex.{platform}
            beq     {SolidCollisionLabel}
    NoTilesOnRight:
    }
        TestLeftRightTiles(UP_PLATFORM, SolidCollisionBelow)

        rep     #$30
    a16()
        // Y = tile index of tile below the bottom-center of hitbox
        rts


    a8()
    i16()
    CollisionBelow:
        // Tile below the entity is a down-collision.
        //
        // Ah = 0
        // X = TopHeightTable index
        // Y = tile index of tile below the bottom-center of hitbox
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingDown.Collision`

        // set collision flag
        lda.b   #TileCollisionState.DOWN_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel above the tile's TopHeightTable value
        //
        // _bottomOffset = _yOffsetHeightPair.height + _yPosHeightPair.yOffset
        // BaseEntity.yPos.px = (((_bottom + METATILE_SIZE_PX) & 0xfff0) | TopHeightTable[x]) - _bottomOffset - 1
        lda.w   _yOffsetHeightPair + 1
        clc
        adc.w   _yOffsetHeightPair

        rep     #$31
    a16()
        // Ah 0
        sta.w   _bottomOffset


        lda.w   _bottom
        // carry clear
        adc.w   #METATILE_SIZE_PX   // Move entity one tile below _bottom
        and.w   #0xfff0

        sep     #$20
    a8()
        ora.l   TopHeightTable,x

        rep     #$21
    a16()
        // carry clear (-1 to place entity above solid tiles)
        sbc.w   _bottomOffset
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        // yVecl is already reset
        rts


    a8()
    i16()
    SolidCollisionBelow:
        // Entity collided with a SOLID or UP_PLATFORM tile.
        //
        // Ah = 0
        // Y = entity bottom-center map tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingDown.SolidCollision`

        // set collision flag
        lda.b   #TileCollisionState.DOWN_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel above the tile below the _bottom
        //
        // _bottomOffset = _yOffsetHeightPair.height + _yPosHeightPair.yOffset
        // BaseEntity.yPos.px = (_bottom & 0xfff0) + METATILE_SIZE_PX - _bottomOffset - 1
        lda.w   _yOffsetHeightPair + 1
        clc
        adc.w   _yOffsetHeightPair

        rep     #$31
    a16()
        // Ah = 0
        sta.w   _bottomOffset

        lda.w   _bottom
        and.w   #0xfff0
        // carry clear
        adc.w   #METATILE_SIZE_PX   // Move entity to tile below _bottom
        clc     // (-1 to place entity above solid tiles)
        sbc.w   _bottomOffset
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        // yVecl is already reset
        rts
    }


a16()
i16()
MovingUp:
    // Y = hiword(BaseEntity.yVecl)
    namespace MovingUp {
        // Entity moving up and was not standing on a ceiling in the previous
        // `MoveEntityAndProcessTileCollisions` call


        // add entity Y velocity to Y position, saving the change in Ypos to _yDelta
        clc
        lda.b   BaseEntity.yVecl
        adc.b   BaseEntity.yPos.sx
        sta.b   BaseEntity.yPos.sx

        // Y = hiword(BaseEntity.yVecl)
        tya
        adc.w   #0
        sta.w   _yDelta

        clc
        adc.b   BaseEntity.yPos.px
        sta.b   BaseEntity.yPos.px


        // Calculate tile index for top-center of tile hitbox

        // _top = _yPosHeightPair.yPos + entity.yPos.px
        // Y = (_top - map.TOP) / 16 * map.width + (entity.xPos.px - map.LEFT) / 16
        lda.w   _yOffsetHeightPair
        ora.w   #0xff00                 // yOffset is always negative value
        clc
        adc.b   BaseEntity.yPos.px
        sta.w   _top
        sec
        sbc.w   #map.TOP
        lsr
        lsr
        lsr
        lsr
        sep     #$20
    a8()
        sta.l   WRMPYB  // WRMPYA set in X axis test

        rep     #$31
    a16()

        lda.b   BaseEntity.xPos.px
        // carry clear
        sbc.w   #map.LEFT - 1
        lsr
        lsr
        lsr
        lsr
        clc
        adc.l   RDMPY
        tay
        // Y = entity top-center map tile

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        sep     #$20
    a8()


        // Test to see if the top-center of the tile hitbox is inside the tile collision area.

        lda.w   map.data,y
        tax
        lda.w   collisionMap,x

        // goto CenterTileIsSolid if tile is SOLID or END_SLOPE
        assert(HeightTableIndex.END_SLOPE & 0xf0 == HeightTableIndex.SOLID)
        and.b   #0xf0
        cmp.b   #HeightTableIndex.SOLID
        beq     CenterTileIsSolid

        // goto TestLeftRightTiles if tile is EMPTY or UP_PLATFORM
        assert(HeightTableIndex.EMPTY == 0x00)
        assert(HeightTableIndex.SOLID == 0x10)
        assert(HeightTableIndex.UP_PLATFORM == 0x20)
        cmp.b   #HeightTableIndex.UP_PLATFORM + 0x10
        bcc     TestLeftRightTiles

        // Treat DOWN slopes as platforms,
        // prevents small entities from falling through the floor when jumping on a down slope.
        assert(_HeightTableIndex_LAST_DOWN_SLOPE < _HeightTableIndex_FIRST_UP_SLOPE)
        cmp.b   #_HeightTableIndex_LAST_DOWN_SLOPE + 0x10
        bcc     CenterTileIsDownPlatform
            // Center tile is an up slope

            //  x = (entity.xPos.px & 0xf) | collisionMap[x]
            //  if (_top & 0xf) <= BottomHeightTable[x]:
            //      goto Collision
            //  else:
            //      return
            lda.b   BaseEntity.xPos.px
            and.b   #0x0f
            ora.w   collisionMap,x
            tax

            lda.w   _top
            and.b   #0xf
            cmp.l   BottomHeightTable,x
            beq     Collision
            bcc     Collision
                // Y = entity bottom-center map tile
                rep     #$30
            a16()
                rts

    CenterTileIsSolid:
        jmp     SolidCollision

    // ::SPAGHETTI Have to put Collision here to prevent a "branch out of bounds" assembly error::

    a8()
    i16()
    Collision:
        // Entity is inside the collision area of a tile.
        //
        // X = BottomHeightTable index
        // Y = entity top-center map tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp_Standing.CollisionAbove`

        // set collision flag
        lda.b   #TileCollisionState.UP_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel below the tile's BottomHeightTable value
        //
        // BaseEntity.yPos.px = (_top & 0xfff0) | BottomHeightTable[x]) - _yPosHeightPair.yOffset + 1

        // sign extend yOffset to 16 bits (yOffset is always negative)
        lda.b   #0xff
        sta.w   _yOffsetHeightPair + 1

        rep     #$30
    a16()

        lda.w   _top
        and.w   #0xfff0

        sep     #$20
    a8()
        ora.l   BottomHeightTable,x

        rep     #$20
    a16()
        inc     // +1 to place entity below solid tiles
        sec
        sbc.w   _yOffsetHeightPair
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx


        // Reset yVecl
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        rts


    a8()
    i16()
    // Ah = 0
    CenterTileIsDownPlatform:
        // if entity was below the DOWN_PLATFORM in the previous frame:
        //      goto SolidCollision
        // else:
        //      goto TestLeftRightTiles

        lda.w   _top
        and.b   #0x0f
        sec
        sbc.w   _yDelta                 // yDelta is negative
        cmp.b   #METATILE_SIZE_PX - 1
        bcs     SolidCollision


    a8()
    i16()
    TestLeftRightTiles:
        MovingDown.TestLeftRightTiles(DOWN_PLATFORM, SolidCollision, NoCollision)

            // top left/right is touching a DOWN_PLATFORM tile

            // goto SolidCollision if entity was below the DOWN_PLATFORM in the previous frame
            lda.w   _top
            and.b   #0x0f
            sec
            sbc.w   _yDelta
            cmp.b   #METATILE_SIZE_PX - 1
            bcs     SolidCollision

    a8()
    i16()
    NoCollision:
        // Y = entity bottom-center map tile
        rep     #$30
    a16()
        rts


    a8()
    i16()
    SolidCollision:
        // Entity collided with a SOLID or PLATFORM tile.
        //
        // Y = entity center top tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp_Standing.SolidCollisionAbove`

        // set collision flag
        lda.b   #TileCollisionState.UP_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel below the tile's BottomHeightTable value
        //
        // BaseEntity.yPos.px = (_top & 0xfff0) | BottomHeightTable[x]) - _yPosHeightPair.yOffset + 1

        // sign extend _yOffset to 16 bits
        lda.b   #0xff
        sta.w   _yOffsetHeightPair + 1

        rep     #$31
    a16()
        lda.w   _top
        and.w   #0xfff0
        // carry clear
        adc.w   #METATILE_SIZE_PX - 1 + 1      // +1 to place entity below solid tiles
        sec
        sbc.w   _yOffsetHeightPair
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx


        // Reset yVecl
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        rts
    }


a16()
i16()
MovingUp_Standing:
    namespace MovingUp_Standing {
        // Entity is moving up and was standing on the ceiling of a
        // slope/platform/solid on the previous
        // `MoveEntityAndProcessTileCollisions` call.


        // Reset yVecl
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2


        // Calculate tile index for bottom-center of tile hitbox
        // _top and RDMPY already set in X-axis test

        // Y = (entity.xPos.px - map.LEFT) / 16 + RDMPY
        clc
        lda.b   BaseEntity.xPos.px
        sec
        sbc.w   #map.LEFT
        lsr
        lsr
        lsr
        lsr
        clc
        adc.l   RDMPY
        tay
        // Y = entity top-center map tile

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        sep     #$20
    a8()

        // Check to see if we are about to enter a new slope.
        //
        // This test must jump to `MovingUp.Collision` if the entity
        // is below of a DOWN_PLATFORM or a upwards slope to prevent the
        // entity from overshooting a slope when xVecl is too large.

        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        assert(HeightTableIndex.EMPTY == 0)
        beq     +
        cmp.b   #HeightTableIndex.UP_PLATFORM
        beq     +
            // not EMPTY or UP_PLATFORM
            lda.b   BaseEntity.xPos.px
            and.b   #0x0f
            ora.w   collisionMap,x
            tax
            jmp     MovingUp.Collision
        +


        // Test tile below the current one

        // Y += map.bytesPerRow
        rep     #$30
    a16()
        tya
        sec
        sbc.w   map.bytesPerRow
        tay

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        // goto CollisionAbove if tile below the entity is not EMPTY or an UP_PLATFORM
        sep     #$20
    a8()
        lda.w   map.data,y
        tax

        lda.w   collisionMap,x
        assert(HeightTableIndex.EMPTY == 0)
        beq     +
        cmp.b   #HeightTableIndex.UP_PLATFORM
        beq     +
            // not EMPTY or UP_PLATFORM
            lda.b   BaseEntity.xPos.px
            and.b   #0xf
            ora.w   collisionMap,x
            tax
            bra     CollisionAbove
        +

        // goto SolidCollisionAbove if left/right tiles are SOLID or DOWN_PLATFORM
        MovingDown_Standing.TestLeftRightTiles(DOWN_PLATFORM, SolidCollisionAbove)

        rep     #$30
    a16()
        // Y = tile index of tile above the top-center of hitbox
        rts


    a8()
    i16()
    CollisionAbove:
        // Tile above the entity is up-collidable.
        //
        // X = BottomHeightTable index
        // Y = tile index of tile above the top-center of hitbox
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp.Collision`

        // set collision flag
        lda.b   #TileCollisionState.UP_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel below the tile's BottomHeightTable value
        //
        // BaseEntity.yPos.px = ((_top & 0xfff0) | BottomHeightTable[x])
        //                       - _yPosHeightPair.yOffset - METATILE_SIZE_PX + 1

        // sign extend _yOffset to 16 bits
        lda.b   #0xff
        sta.w   _yOffsetHeightPair + 1

        rep     #$31
    a16()
        lda.w   _top
        and.w   #0xfff0

        sep     #$20
    a8()
        ora.l   BottomHeightTable,x

        rep     #$31
    a16()
        // carry clear
        sbc.w   #METATILE_SIZE_PX - 1 - 1 // -1 for carry and -1 to place entity below solid tiles
        sec
        sbc.w   _yOffsetHeightPair
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        // yVecl is already reset
        rts


    a8()
    i16()
    SolidCollisionAbove:
        // Entity collided with a SOLID or DOWN_PLATFORM tile.
        //
        // Ah = 0
        // Y = tile index of tile above the top-center of hitbox
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp.SolidCollision`

        // set collision flag
        lda.b   #TileCollisionState.UP_COLLISION
        tsb.b   BaseEntity.tileCollisionState


        // Move entity to one pixel below the current tile
        //
        // BaseEntity.yPos.px = (_top & 0xfff0) - _yPosHeightPair.yOffset + 1

        // sign extend _yOffset to 16 bits
        lda.b   #0xff
        sta.w   _yOffsetHeightPair + 1

        rep     #$30
    a16()
        lda.w   _top
        and.w   #0xfff0
        sec
        sbc.w   _yOffsetHeightPair
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        // yVecl already reset
        rts
    }
}



// Top Y position of the tile collision for each X position
// NOTE: If you change these values, you must also change them in untech-editor
//
// Values must be between 0-15 or >= 32
rodata(rom0)
TopHeightTable:
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
    db  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
    db 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8
    db  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0
    db  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7
    db  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
assert(pc() - TopHeightTable == 16 * 16)

// Bottom Y position of the tile collision for each X position
// NOTE: If you change these values, you must also change them in untech-editor
//
// Values must be between 0-15
rodata(rom0)
BottomHeightTable:
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
    db 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
    db  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7
    db  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15
    db 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8
    db  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0
assert(pc() - BottomHeightTable == 16 * 16)

}

// vim: ft=bass-65816 ts=4 sw=4 et:

