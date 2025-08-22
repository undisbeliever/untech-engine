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


namespace Entities {
namespace WalkAndTurn {

namespace EntityStruct {
    childstruct(_BaseWalkEnemy.EntityStruct)
    endstruct()
}


buildFunctionTable(BaseEntityFunctionTable, WalkAndTurn)

constant Init = _BaseWalkEnemy.Init
constant Destructor = _BaseWalkEnemy.Destructor
constant Deactivated = _BaseWalkEnemy.Deactivated
constant HitboxCollision = _BaseWalkEnemy.HitboxCollision
constant HurtboxCollision = _BaseWalkEnemy.HurtboxCollision
constant ShieldCollision = _BaseWalkEnemy.ShieldCollision


_BaseWalkEnemy.BuildProcessFunction()



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WalkLeft {
    MoveEntityAndProcessTileCollisions()

    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
        jsr     XCollision
    +

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process__WalkRight {
    MoveEntityAndProcessTileCollisions()

    assert(Entity.MovementState.X_COLLISION_FLAG == 0x80)
    bit.b   EntityStruct.movementState - 1
    bpl     +
        jsr     XCollision
    +

    jmp     Entity.DeactivateIfOutsideOrDrawAndGotoNextEntity
}

}
}

