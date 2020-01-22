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

code()
namespace MoveEntityAndProcessTileCollisions {

// Check if standing and set appropriate `movementState` flags.
// Invoked when in a falling state and bottom of hitbox collided with a tile.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// KEEP: X and Y
inline _CheckIfStanding_Bottom() {
    assert8a()
    assert16i()

    // MUST NOT MODIFY X or Y

    // BUGFIX: I have to set direction of movement when setting Standing flag
    //         to prevent a Mario 64 style ceiling clip.

    // standing if no gravity or down gravity
    lda.b   BaseEntity.movementState
    bit.b   #Entity.MovementState.NO_GRAVITY_FLAG
    bne     NoGravity{#}
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG
    bne     NotStanding{#}
    NoGravity{#}:
        // Set standing and moving down state
        // A = entity->movementState
        ora.b   #Entity.MovementState.STANDING_FLAG | Entity.MovementState.MOVING_DOWN_FLAG
        sta.b   BaseEntity.movementState

        bra     EndIf{#}


    NotStanding{#}:
        // Not Standing

        // Set Y direction to down
        // A = entity->movementState
        ora.b   #Entity.MovementState.MOVING_DOWN_FLAG
        sta.b   BaseEntity.movementState

        // reset yMomentum
        // ::TODO bonking momentum (make it room & region independent)::
        stz.b   BaseEntity.yMomentum.sx
        stz.b   BaseEntity.yMomentum.sx + 1
        stz.b   BaseEntity.yMomentum.px
EndIf{#}:
}


// Check if standing and set appropriate `movementState` flags.
// Invoked when in a falling state and top of hitbox collided with a tile.
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// KEEP: X and Y
inline _CheckIfStanding_Top() {
    assert8a()
    assert16i()

    // MUST NOT MODIFY X or Y

    // BUGFIX: I have to set direction of movement when setting Standing flag
    //         to prevent a Mario 64 style ceiling clip.

    // standing if no gravity or up gravity
    lda.b   BaseEntity.movementState
    bit.b   #Entity.MovementState.UP_GRAVITY_FLAG | Entity.MovementState.NO_GRAVITY_FLAG
    beq     NotStanding{#}
        // Set standing and moving up state
        // A = entity->movementState
        and.b   #~Entity.MovementState.MOVING_DOWN_FLAG
        ora.b   #Entity.MovementState.STANDING_FLAG
        sta.b   BaseEntity.movementState

        bra     EndIf{#}


    NotStanding{#}:
        // Not Standing

        // Set Y direction to UP
        // A = entity->movementState
        and.b   #~Entity.MovementState.MOVING_DOWN_FLAG
        sta.b   BaseEntity.movementState

        // Reset yMomentum
        // ::TODO bonking momentum (make it room & region independent)::
        stz.b   BaseEntity.yMomentum.sx
        stz.b   BaseEntity.yMomentum.sx + 1
        stz.b   BaseEntity.yMomentum.px
EndIf{#}:
}

namespace _CheckIfStanding {
    constant MovingDown = 1
    constant MovingUp = 2
}


// DP = entity
a16()
i16()
NoHitbox:
    rts
}


// Moves the entity (using the entity's movementState and momentum values) and
// preforms collision checks between the MetaTiles and the entity's tile
// hitbox.
//
// If a collision occurs then the entity is moved so its tile hitbox is outside
// the collision and the appropriate bits in `BaseEntity.movementState`
// will be set.
//
// If the entity is in a Standing state and there is no collision then
// the `Entity.MovementState.STANDING_FLAG` will be cleared.
//
// NOTE: You MUST ONLY call this routine inside the entity's Process routine
//
// NOTE: If the entity's MetaSprite frame has no tile hitbox then the entity
//       will not be moved.
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
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// OUTPUT: Y = The tilemap index directly above/below the entity (depending on Y direction of movement),
//             Set to 0 if there is no tile hitbox.
//
// This code assumes:
//   * This routine is called once per frame.
//   * The maximum x/y momentum is 8px/frame.
//   * The entity is inside the map boundary.
//   * An END_SLOPE tile or a SOLID wall is adjacent to the long side of a *_SLOPE tile.
//   * An EMPTY tile or a SOLID wall is adjacent to the short side of a *_SLOPE tile.
//   * A SOLID, UP_PLATFORM or END_SLOPE tile is below the DOWN_*_SLOPE tiles.
//   * A SOLID, DOWN_PLATFORM or END_SLOPE tile is above the UP_*_SLOPE tiles.
//   * An EMPTY tile is above a DOWN_*_SLOPE tiles.
//   * An EMPTY tile is below a UP_*_SLOPE tiles.
//   * An EMPTY tile is *not* to the left or right of an END_SLOPE tile.
//   * A short/tall slope pair is never broken, unless there is a SOLID wall in the way.
//   * It is not possible to walk into the long side of a slope tile.
//   * The top-left tile of the map is SOLID.
//
// If none of these assumptions are met then the entity can either:
//   * Overshoot slope tiles when `xMomentum` > 1px/frame.
//   * Fall through a slope if the x origin tile is EMPTY.
//   * Zip to the top or bottom of a tile when walking into an forbidden tile pattern.
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

// Confirm entity momentum is a 24 bit value
assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)

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

constant _TileCollisionType_FIRST_UP_SLOPE   = TileCollisionType.UP_RIGHT_SLOPE
constant _TileCollisionType_LAST_UP_SLOPE    = TileCollisionType.UP_LEFT_SHORT_SLOPE
constant _TileCollisionType_FIRST_DOWN_SLOPE = TileCollisionType.DOWN_RIGHT_SLOPE
constant _TileCollisionType_LAST_DOWN_SLOPE  = TileCollisionType.DOWN_LEFT_SHORT_SLOPE


//  X AXIS
// --------
//
// X-Axis movement can only collide with a SOLID tile, all other tile collision types are ignored.

a16()
i16()
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

    rep     #$30
a16()
    sta.l   WRMPYA      // a = map.width, b = top / 16


    lda.l   MetaSprite.Format.Frame.TileHitbox.left,x
    sta.w   _leftRightPair


    assert(Entity.MovementState.MOVING_RIGHT_FLAG == 0x40)
    bit.b   BaseEntity.movementState - 1
    bvs     MovingRight
        jmp     MovingLeft

a16()
i16()
MovingRight:
    namespace MovingRight {
        Entity.Movement._AddMomentumToPosition(x)

        // A = entity.xPos.px

        // _xTmp = entity.xPos.px + _leftRightPair.right
        // Y = (_xTmp - map.LEFT) / 16 + RDMPY
        clc
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

        // Y = tile index for top-right of hitbox

        // set Ah to 0 for `tax` below
        lda.w   #0


        // Goto Collision if tile index is a SOLID tile
        sep     #$20
    a8()
        // if collisionMap[map.data[y]] == TileCollisionType.SOLID goto Collision
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        assert(TileCollisionType.SOLID == 0)
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
            assert(TileCollisionType.SOLID == 0)
            beq     Collision

            dec.w   _counter
            bne     Loop


    a8()
    iu()
    NoCollision:
        // clear X collision flag
        lda.b   #Entity.MovementState.X_COLLISION_FLAG
        trb.b   BaseEntity.movementState

        rep     #$30
    a16()
    i16()
        jmp     EndXaxis_A16


    a8()
    iu()
    Collision:
        // The right edge of the tile hitbox touched a SOLID tile

        // set X collision flag
        lda.b   #Entity.MovementState.X_COLLISION_FLAG
        tsb.b   BaseEntity.movementState

        // Reset xMomentum
        stz.b   BaseEntity.xMomentum.px
        rep     #$30
    a16()
    i16()
        stz.b   BaseEntity.xMomentum.sx

        // entity.xPos.px = (_xTmp & 0xfff0) + _leftRightPair.right
        lda.w   _xTmp
        and.w   #0xfff0
        sec
        sbc.w   _leftRightPair + 1          // 3rd byte of pair is 0
        sta.b   BaseEntity.xPos.px

        jmp     EndXaxis_A16
    }


a16()
i16()
MovingLeft:
    namespace MovingLeft {
        Entity.Movement._SubtractMomentumFromPosition(x)

        // A = entity.xPos.px

        // _xTmp = entity.xPos.px - _leftRightPair.left
        // Y = (_xTmp - map.LEFT) / 16 + RDMPY

        lda.w   _leftRightPair
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

        // Y = tile index for top-left of hitbox

        // set Ah to 0 for `tax` below
        lda.w   #0


        // Goto Collision if tile index is a SOLID tile
        sep     #$20
    a8()
        // if collisionMap[map.data[y]] == TileCollisionType.SOLID goto Collision
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x
        assert(TileCollisionType.SOLID == 0)
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
            assert(TileCollisionType.SOLID == 0)
            beq     Collision

            dec.w   _counter
            bne     Loop

    a8()
    iu()
    NoCollision:
        // clear X collision flag
        lda.b   #Entity.MovementState.X_COLLISION_FLAG
        trb.b   BaseEntity.movementState

        rep     #$30
    a16()
    i16()
        bra     EndXaxis_A16


    a8()
    iu()
    Collision:
        // The left edge of the tile hitbox touched a SOLID tile

        // set X collision flag
        lda.b   #Entity.MovementState.X_COLLISION_FLAG
        tsb.b   BaseEntity.movementState

        stz.b   BaseEntity.xMomentum.px

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

        stz.b   BaseEntity.xMomentum.sx
    }
a16()
i16()
EndXaxis_A16:



//  Y AXIS
// --------

// The collision tests preformed on the y-axis depend on the value of
// `BaseEntity.movementState`.
//
// NOMENCLATURE: center tile means the tile above or below the entity's xPos.

    lda.b   BaseEntity.movementState
    and.w   #Entity.Movement.Y_AXIS_MOVEMENT_STATE_FUNCTION_TABLE_MASK
    tax
    jmp     (MovementState_FunctionTable,x)

constant MovementState_NoGravity_Falling_Down     = MovingDown
constant MovementState_NoGravity_Falling_Up       = MovingUp

constant MovementState_NoGravity_Standing_Down    = MovingDown_Standing
constant MovementState_UpGravity_Standing_Down    = MovingDown_Standing
constant MovementState_DownGravity_Standing_Down  = MovingDown_Standing
constant MovementState_NoGravity_Standing_Up      = MovingUp_Standing
constant MovementState_UpGravity_Standing_Up      = MovingUp_Standing
constant MovementState_DownGravity_Standing_Up    = MovingUp_Standing

MovementState_UpGravity_Falling_Up:
    Entity.Movement._AddGravityToYMomentum()
    jmp     MovingUp

MovementState_DownGravity_Falling_Up:
    Entity.Movement._SubtractGravityFromYMomentum_AndBranchIfDirectionChanges(MovingDown, Down)
    jmp     MovingUp

MovementState_UpGravity_Falling_Down:
    Entity.Movement._SubtractGravityFromYMomentum_AndBranchIfDirectionChanges(MovingUp, Up)
    jmp     MovingDown

MovementState_DownGravity_Falling_Down:
    Entity.Movement._AddGravityToYMomentum()
MovementState_DownGravity_Falling_Down__FallThrough:

a16()
i16()
MovingDown:
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
        // If there is no collision with the bottom tiles then the top-center
        // tile is tested for a SOLID or UP_SLOPE collision.  This test is to
        // prevent an entity that is jumping down-left (or right) at an angle
        // <45 degrees from falling through an UP_SLOPE.
        //
        // Only the top-center and bottom-center tiles are tested for slope
        // collisions, which is not an issue as the engine requires all down
        // slope tiles to have a SOLID or UP_PLATFORM tile underneath them.
        //
        // The END_SLOPE tile is only solid in the bottom-center tile.
        // There is no need to write code to test for END_SLOPE as
        // `TopHeightTable[END_SLOPE | (xPos & 0xf)]` is always 0.
        //
        // The falling code will only reset the entity's yMomentum if the
        // entity has a collision in the Y-Axis and it did not change into a
        // standing state.

        // verify there is no need to test for END_SLOPE in this code.
        assert(TileCollisionType.END_SLOPE & 0xf0 == TileCollisionType.SOLID)

    assert(pc() == MovementState_DownGravity_Falling_Down__FallThrough)

        // add entity Y momentum to Y position, saving the change in Ypos to _yDelta
        clc
        lda.b   BaseEntity.yMomentum.sx
        adc.b   BaseEntity.yPos.sx
        sta.b   BaseEntity.yPos.sx

        lda.b   BaseEntity.yMomentum.px
        and.w   #0xff
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

        // No collision if DOWN_PLATFORM, EMPTY or UP SLOPE
        assert(TileCollisionType.DOWN_PLATFORM > TileCollisionType.EMPTY)
        assert(_TileCollisionType_FIRST_UP_SLOPE > TileCollisionType.EMPTY)
        assert(_TileCollisionType_LAST_UP_SLOPE == 0xf0)
        cmp.b   #TileCollisionType.EMPTY
        bcs     TestLeftRightTiles

        // Test if UP_PLATFORM
        assert(TileCollisionType.UP_PLATFORM < TileCollisionType.EMPTY)
        cmp.b   #TileCollisionType.UP_PLATFORM
        beq     CenterTileIsUpPlatform

            // Tile is SOLID, END_SLOPE or DOWN SLOPE
            // Test if entity is below the collision area of the tile

            assert(TileCollisionType.SOLID < TileCollisionType.EMPTY)
            assert(TileCollisionType.END_SLOPE < TileCollisionType.EMPTY)
            assert(_TileCollisionType_LAST_DOWN_SLOPE < TileCollisionType.EMPTY)

            //  x = (entity.xPos.px & 0xf) | collisionMap[x]
            //  if (_bottom & 0xf) >= TopHeightTable[x]:
            //      goto Collision
            //  else:
            //      goto NoBottomCollision
            lda.b   BaseEntity.xPos.px
            and.b   #0x0f
            ora.w   collisionMap,x
            tax

            lda.w   _bottom
            and.b   #0xf
            cmp.l   TopHeightTable,x
            bcs     Collision
                jmp     NoBottomCollision

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
        // NOTE: If I modify this code, also modify code in `MovingDown_Standing.TileBelowCenterIsDownCollidable`

        _CheckIfStanding_Bottom()

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
        assert(TileCollisionType.{platform} != 0)
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
                assert(TileCollisionType.SOLID == 0)
                beq     {SolidCollisionLabel}
                cmp.b   #TileCollisionType.{platform}
                bne     +
                    sta.w   _edgeOnPlatform
                +

        OnlyOneTileOnLeft:
            lda.w   map.data - 1,y
            tax
            lda.w   collisionMap,x
            assert(TileCollisionType.SOLID == 0)
            beq     {SolidCollisionLabel}
            cmp.b   #TileCollisionType.{platform}
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
                assert(TileCollisionType.SOLID == 0)
                beq     {SolidCollisionLabel}
                cmp.b   #TileCollisionType.{platform}
                bne     +
                    sta.w   _edgeOnPlatform
                +

        OnlyOneTileOnRight:
            lda.w   map.data + 1,y
            tax
            lda.w   collisionMap,x
            assert(TileCollisionType.SOLID == 0)
            beq     {SolidCollisionLabel}
            cmp.b   #TileCollisionType.{platform}
            beq     OnPlatformTile

    NoTilesOnRight:
        assert(TileCollisionType.{platform} != 0)
        lda.w   _edgeOnPlatform
        beq     {NoCollisionLabel}

    OnPlatformTile:
    }
        TestLeftRightTiles(UP_PLATFORM, SolidCollision, NoBottomCollision)

            // bottom left/right is touching a UP_PLATFORM tile

            // if entity was above the UP_PLATFORM in the previous frame:
            //      goto SolidCollision
            // else:
            //      goto NoBottomCollision
            lda.w   _bottom
            and.b   #0xf
            beq     SolidCollision
            cmp.w   _yDelta
            bcs     NoBottomCollision

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

        _CheckIfStanding_Bottom()

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

        rts



    i16()
    NoBottomCollision:
        // No Collision on the bottom.
        // Test for a SOLID/END_SLOPE/UP_*_SLOPE collision with the top-center tile

        // Y = entity bottom-center map tile
        // MUST NOT MODIFY Y

        // Calculate tile index for top-center tile
        // _top = _yPosHeightPair.yPos + entity.yPos.px
        // X = (_top - map.TOP) / 16 * map.width + (entity.xPos.px - map.LEFT) / 16
        rep     #$31
    a16()
        lda.w   _yOffsetHeightPair
        ora.w   #0xff00                 // yOffset is always negative value
        // carry clear
        adc.b   BaseEntity.yPos.px
        sta.w   _top                    // required for `MovingUp.Collision`
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
        tax
        // X = entity top-center map tile

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        sep     #$20
    a8()
        lda.w   map.data,x
        tax
        lda.w   collisionMap,x

        // Test if top-center is a SOLID or END_SLOPE tile
        assert(TileCollisionType.SOLID == 0)
        assert(TileCollisionType.END_SLOPE & 0xf0 == 0)
        and.b   #0xf0
        bne     +
            jmp     MovingUp.SolidCollision
        +

        // Test if top-center tile is an UP SLOPE tile
        assert(_TileCollisionType_LAST_UP_SLOPE == 0xf0)
        cmp.b   #_TileCollisionType_FIRST_UP_SLOPE
        bcc     ++
            // top-center tile is an UP_*_SLOPE.
            // Test if _top is on or above the slope

            //  x = (entity.xPos.px & 0xf) | collisionMap[x]
            //  if (_top & 0xf) <= BottomHeightTable[x]:
            //      goto MovingUp.Collision
            lda.b   BaseEntity.xPos.px
            and.b   #0x0f
            ora.w   collisionMap,x
            tax

            lda.w   _top
            and.b   #0xf
            cmp.l   BottomHeightTable,x
            beq     +
            bcs     ++
            +
                jmp     MovingUp.Collision
        +

        rep     #$30
    a16()
        // Y = entity bottom-center map tile
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
        // The Y momentum is not processed when standing.  The entity's Y
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
        // issue as there all down slope tiles have a SOLID, END_SLOPE or
        // UP_PLATFORM tile underneath them.
        //
        // The bottom left/right corner tests will only check two tiles to the
        // left and two tiles to the right of the X position.  If the tile
        // hitbox's left/right values are > 32 the remaining tiles will be
        // ignored.
        //
        // There is no need to write code to test for END_SLOPE, as
        // `BottomHeightTable[END_SLOPE | (xPos & 0xf)]` is always 0.
        //
        // The Standing code will reset the yMomentum to 0 when there is no collision
        // and the entity's Standing flag is cleared.


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
        // touches an END_SLOPE, UP_PLATFORM or a downwards slope to prevent
        // the entity from overshooting a slope when X-Momentum is too large.

        lda.w   map.data,y
        tax
        lda.w   collisionMap,x

        // Collision if tile is SOLID, END_SLOPE, or DOWN SLOPE
        assert(TileCollisionType.SOLID < TileCollisionType.UP_PLATFORM)
        assert(TileCollisionType.END_SLOPE < TileCollisionType.UP_PLATFORM)
        assert(_TileCollisionType_LAST_DOWN_SLOPE < TileCollisionType.UP_PLATFORM)
        cmp.b   #TileCollisionType.UP_PLATFORM
        bcc     BottomCenterIsDownCollidable

        // tile is EMPTY, PLATFORM or UP SLOPE

        // Test tile below bottom-center

        // Y -= map.bytesPerRow
        rep     #$31
    a16()
        tya
        // carry clear
        adc.w   map.bytesPerRow
        tay

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        sep     #$20
    a8()
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x

        // Collision if tile is SOLID, END_SLOPE, UP_PLATFORM or DOWN SLOPE
        assert(TileCollisionType.SOLID < TileCollisionType.EMPTY)
        assert(TileCollisionType.END_SLOPE < TileCollisionType.EMPTY)
        assert(TileCollisionType.UP_PLATFORM < TileCollisionType.EMPTY)
        assert(_TileCollisionType_LAST_DOWN_SLOPE < TileCollisionType.EMPTY)
        cmp.b   #TileCollisionType.EMPTY
        bcc     TileBelowCenterIsDownCollidable

        // tile is EMPTY, DOWN_PLATFORM or UP SLOPE


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
                assert(TileCollisionType.SOLID == 0)
                beq     {SolidCollisionLabel}
                cmp.b   #TileCollisionType.{platform}
                beq     {SolidCollisionLabel}

        OnlyOneTileOnLeft:
            lda.w   map.data - 1,y
            tax
            lda.w   collisionMap,x
            assert(TileCollisionType.SOLID == 0)
            beq     {SolidCollisionLabel}
            cmp.b   #TileCollisionType.{platform}
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
                assert(TileCollisionType.SOLID == 0)
                beq     {SolidCollisionLabel}
                cmp.b   #TileCollisionType.{platform}
                beq     {SolidCollisionLabel}

        OnlyOneTileOnRight:
            lda.w   map.data + 1,y
            tax
            lda.w   collisionMap,x
            assert(TileCollisionType.SOLID == 0)
            beq     {SolidCollisionLabel}
            cmp.b   #TileCollisionType.{platform}
            beq     {SolidCollisionLabel}
    NoTilesOnRight:
    }
        TestLeftRightTiles(UP_PLATFORM, SolidCollisionBelow)

    NoCollision:
        // All tiles tested have no collision

        // Clear Standing flag
        lda.b   #Entity.MovementState.STANDING_FLAG
        trb.b   BaseEntity.movementState

        // Reset Y momentum
        stz.b   BaseEntity.yMomentum.px

        rep     #$30
    a16()
        stz.b   BaseEntity.yMomentum.sx

        // Y = tile index of tile below the bottom-center of hitbox
        rts


    a8()
    i16()
    BottomCenterIsDownCollidable:
        // bottom-center is down-collideable
        //
        // Ah = 0
        // X = collisionMap index
        // Y = tile index of bottom-center of hitbox
        lda.b   BaseEntity.xPos.px
        and.b   #0x0f
        ora.w   collisionMap,x
        tax
        jmp     MovingDown.Collision


    a8()
    i16()
    TileBelowCenterIsDownCollidable:
        // Tile below the entity is a down-collision.
        //
        // Ah = 0
        // X = collisionMap Index
        // Y = tile index of tile below the bottom-center of hitbox
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingDown.Collision`

        lda.b   BaseEntity.xPos.px
        and.b   #0xf
        ora.w   collisionMap,x
        tax
        // X = TopHeightTable index

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

        rts
    }


a16()
i16()
MovingUp:
    namespace MovingUp {
        // Entity moving up and was not standing on a ceiling in the previous
        // `MoveEntityAndProcessTileCollisions` call

        // Subtract 24 bit Y Momentum from 32 bit Position, saving the change in Ypos.px to _yDelta
        sec
        lda.b   BaseEntity.yPos.sx
        sbc.b   BaseEntity.yMomentum.sx
        sta.b   BaseEntity.yPos.sx

        lda.b   BaseEntity.yMomentum.px
        and.w   #0xff
        eor.w   #0xffff
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

        // Test if center tile is an UP SLOPE
        assert(_TileCollisionType_LAST_UP_SLOPE == 0xf0)
        cmp.b   #_TileCollisionType_FIRST_UP_SLOPE
        bcs     CenterTileIsUpSlope

        // Test if center tile is a DOWN_PLATFORM
        cmp.b   #TileCollisionType.DOWN_PLATFORM
        beq     CenterTileIsDownPlatform

        // Test if center tile is SOLID or END_SLOPE
        // Goto TestLeftRightTiles if center tile is UP_PLATFORM, EMPTY or DOWN_SLOPE
        assert(TileCollisionType.UP_PLATFORM > _TileCollisionType_LAST_DOWN_SLOPE)
        assert(TileCollisionType.EMPTY > _TileCollisionType_LAST_DOWN_SLOPE)
        assert(_TileCollisionType_FIRST_UP_SLOPE > _TileCollisionType_LAST_DOWN_SLOPE)

        assert(TileCollisionType.END_SLOPE & 0xf0 == TileCollisionType.SOLID)
        assert(TileCollisionType.SOLID == 0)
        and.b   #0xf0
        bne     TestLeftRightTiles
            jmp     SolidCollision


    CenterTileIsUpSlope:
        // Center tile is an up slope
        // Test if entity is above the collision area of the tile

        //  x = (entity.xPos.px & 0xf) | collisionMap[x]
        //  if (_top & 0xf) <= BottomHeightTable[x]:
        //      goto Collision
        //  else:
        //      goto NoTopCollision
        lda.b   BaseEntity.xPos.px
        and.b   #0x0f
        ora.w   collisionMap,x
        tax

        lda.w   _top
        and.b   #0xf
        cmp.l   BottomHeightTable,x
        beq     Collision
        bcc     Collision
            jmp     NoTopCollision


    a8()
    i16()
    Collision:
        // Entity is inside the collision area of a tile.
        //
        // X = BottomHeightTable index
        // Y = entity top-center map tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp_Standing.TileAboveCenterIsUpCollidable`

        _CheckIfStanding_Top()

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
    // Ah = 0
    TestLeftRightTiles:
        MovingDown.TestLeftRightTiles(DOWN_PLATFORM, SolidCollision, NoTopCollision)

            // top left/right is touching a DOWN_PLATFORM tile

            // if entity was below the DOWN_PLATFORM in the previous frame:
            //     goto SolidCollision
            // else:
            //     goto NoTopCollision

            lda.w   _top
            and.b   #0x0f
            sec
            sbc.w   _yDelta
            cmp.b   #METATILE_SIZE_PX - 1
            bcc     NoTopCollision

    a8()
    i16()
    SolidCollision:
        // Entity collided with a SOLID or PLATFORM tile.
        //
        // Y = entity center top tile
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp_Standing.SolidCollisionAbove`

        _CheckIfStanding_Top()

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

        rts


    i16()
    NoTopCollision:
        // No Collision on the top.
        // Test for SOLID/END_SLOPE/DOWN_*SLOPE collision with the bottom-center tile

        // Y = entity top-center map tile
        // MUST NOT MODIFY Y

        // Calculate tile index for bottom-center tile
        // _bottom = _yOffsetHeightPair.yOffset + _yOffsetHeightPair.height + entity.yPos.px
        // X = (_bottom - map.TOP) / 16 * map.width + (entity.xPos.px - map.LEFT) / 16

        rep     #$31
    a16()
        lda.w   _yOffsetHeightPair
        ora.w   #0xff00                 // yOffset is always negative value
        // carry clear
        adc.w   _yOffsetHeightPair + 1  // Third byte of _yOffsetHeightPair is 0
        clc
        adc.b   BaseEntity.yPos.px
        sta.w   _bottom                 // Required for `MovingDown.Collision`
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

        sep     #$20
    a8()
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x

        // test if tile is SOLID, END_SLOPE or DOWN_PLATFORM
        assert(TileCollisionType.SOLID < TileCollisionType.EMPTY)
        assert(TileCollisionType.END_SLOPE < TileCollisionType.EMPTY)
        assert(_TileCollisionType_LAST_DOWN_SLOPE < TileCollisionType.EMPTY)
        assert(_TileCollisionType_LAST_DOWN_SLOPE < TileCollisionType.UP_PLATFORM)
        assert(_TileCollisionType_LAST_DOWN_SLOPE < TileCollisionType.DOWN_PLATFORM)
        assert(_TileCollisionType_LAST_DOWN_SLOPE < _TileCollisionType_FIRST_UP_SLOPE)
        cmp.b   #_TileCollisionType_LAST_DOWN_SLOPE + 1
        bcs     +
            // bottom-center tile is a SOLID, END_SLOPE or DOWN_*_SLOPE.
            // Test if _bottom is on or below the slope.

            //  x = (entity.xPos.px & 0xf) | collisionMap[x]
            //  if (_bottom & 0xf) >= TopHeightTable[x]:
            //      goto MovingDown.Collision
            lda.b   BaseEntity.xPos.px
            and.b   #0x0f
            ora.w   collisionMap,x
            tax

            lda.w   _bottom
            and.b   #0xf
            cmp.l   TopHeightTable,x
            bcc     +
                jmp     MovingDown.Collision
        +

        rep     #$30
    a16()
        // Y = entity top-center map tile
        rts
    }


a16()
i16()
MovingUp_Standing:
    namespace MovingUp_Standing {
        // Entity is moving up and was standing on the ceiling of a
        // slope/platform/solid on the previous
        // `MoveEntityAndProcessTileCollisions` call.


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

        lda.w   map.data,y
        tax
        lda.w   collisionMap,x

        // Collision if SOLID or END_SLOPE
        assert(TileCollisionType.END_SLOPE & 0xf0 == TileCollisionType.SOLID)
        assert(TileCollisionType.SOLID == 0)
        and.b   #0xf0
        beq     TopCenterIsUpCollidable

        // Collision if UP_SLOPE
        assert(_TileCollisionType_LAST_UP_SLOPE == 0xf0)
        cmp.b   #_TileCollisionType_FIRST_UP_SLOPE
        bcs     TopCenterIsUpCollidable

        // tile is EMPTY, PLATFORM or DOWN SLOPE

        // Test tile above top-center

        // Y += map.bytesPerRow
        rep     #$30
    a16()
        tya
        sec
        sbc.w   map.bytesPerRow
        tay

        // set Ah to 0 for the `tax`s below
        lda.w   #0

        sep     #$20
    a8()
        lda.w   map.data,y
        tax
        lda.w   collisionMap,x

        // Collision if SOLID or END_SLOPE
        assert(TileCollisionType.END_SLOPE & 0xf0 == TileCollisionType.SOLID)
        assert(TileCollisionType.SOLID == 0)
        and.b   #0xf0
        beq     TileAboveCenterIsUpCollidable

        // Collision if DOWN_PLATFORM or UP_SLOPE
        assert(_TileCollisionType_FIRST_UP_SLOPE == TileCollisionType.DOWN_PLATFORM + 0x10)
        cmp.b   #TileCollisionType.DOWN_PLATFORM
        bcs     TileAboveCenterIsUpCollidable

        // tile is EMPTY, UP_PLATFORM or DOWN SLOPE

        // goto SolidCollisionAbove if left/right tiles are SOLID or DOWN_PLATFORM
        MovingDown_Standing.TestLeftRightTiles(DOWN_PLATFORM, SolidCollisionAbove)

    NoCollision:
        // All tiles tested have no collision

        // Clear Standing flag
        lda.b   #Entity.MovementState.STANDING_FLAG
        trb.b   BaseEntity.movementState

        // Reset Y momentum
        stz.b   BaseEntity.yMomentum.px

        rep     #$30
    a16()
        stz.b   BaseEntity.yMomentum.sx

        // Y = tile index of tile below the bottom-center of hitbox
        rts


    a8()
    i16()
    TopCenterIsUpCollidable:
        // top-center tile is up-collidable.
        //
        // Ah = 0
        // X = collisionMap index
        // Y = tile index of top-center of hitbox
        lda.b   BaseEntity.xPos.px
        and.b   #0x0f
        ora.w   collisionMap,x
        tax
        jmp     MovingUp.Collision


    a8()
    i16()
    TileAboveCenterIsUpCollidable:
        // Tile above the entity is up-collidable.
        //
        // Ah = 0
        // X = collisionMap Index
        // Y = tile index of tile above the top-center of hitbox
        // MUST NOT MODIFY Y
        //
        // NOTE: If I modify this code, also modify code in `MovingUp.Collision`

        lda.b   BaseEntity.xPos.px
        and.b   #0xf
        ora.w   collisionMap,x
        tax
        // X = BottomHeightTable index

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

        rts
    }


// ::SPAGHETTI have to place this at bottom to prevent a `constant has unknown value` error::
MovementState_FunctionTable:
    Entity.Movement._GenerateYAxisMovementStateFunctionTable(MovementState, 1)
assert(pc() - MovementState_FunctionTable == Entity.Movement.N_Y_AXIS_MOVEMENT_STATE_FUNCTIONS * 2)
}



// Top Y position of the tile collision for each X position
// NOTE: If you change these values, you must also change them in untech-editor
//
// Values must be between 0-15 or >= 32
rodata(rom0)
TopHeightTable:
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1,  0
    db  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15
    db 15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 10, 10,  9,  9,  8,  8
    db  7,  7,  6,  6,  5,  5,  4,  4,  3,  3,  2,  2,  1,  1,  0,  0
    db  0,  0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  7,  7
    db  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
    db 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32
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
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    db  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
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

