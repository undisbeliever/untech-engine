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
namespace Projectiles {
namespace UpDiagonalProjectile {

buildFunctionTable(BaseEntityFunctionTable, UpDiagonalProjectile)


define State = Entities.Projectiles.LeftRightProjectile.State
define ES = Entities.Projectiles.LeftRightProjectile.EntityStruct



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//       `MetaSprite.ActionPoint.positionPair` = MS frame position that spawned the entity
// OUTPUT: C set = always activate entity
a16()
i16()
code()
function Init {
    assert({State}.MOVE == 0)
    stz.b   {ES}.state

    ldx.b   {ES}.romPtr

    lda.l   {RomStruct}.xMomentum,x
    sta.b   {ES}.xMomentum.sx

    lda.l   {RomStruct}.yMomentum,x
    sta.b   {ES}.yMomentum.sx

    sep     #$30
a8()
i8()

    lda.l   {RomStruct}.xMomentum + 2,x
    sta.b   {ES}.xMomentum.px

    lda.l   {RomStruct}.yMomentum + 2,x
    sta.b   {ES}.yMomentum.px


    QueueSoundEffect(enemyShootProjectile)


    assert(INT_MS8_OFFSET == 0x80)
    lda.w   MetaSprite.ActionPoint.positionPair.xPos
    bmi     Right
        ldx.b   #Entity.MovementState.NO_GRAVITY_FLAG
        lda.b   #{GetAnimationId(left)}

        bra     EndIf
    Right:
        // Action point is on the right, entity is moving right
        ldx.b   #Entity.MovementState.NO_GRAVITY_FLAG | Entity.MovementState.MOVING_RIGHT_FLAG | Entity.MovementState.FACING_RIGHT_FLAG
        lda.b   #{GetAnimationId(right)}
    EndIf:

    stx.b   {ES}.movementState

    rep     #$30
a16()
i16()
    jmp     MetaSprite.Animation.SetAnimationId
}


constant Destructor = LeftRightProjectile.Destructor
constant Deactivated = LeftRightProjectile.Deactivated
constant HitboxCollision = LeftRightProjectile.HitboxCollision
constant HurtboxCollision = LeftRightProjectile.HurtboxCollision
constant ShieldCollision = LeftRightProjectile.ShieldCollision
constant Process = LeftRightProjectile.Process

}
}
}
