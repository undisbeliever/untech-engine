// metasprite/actionpoints.inc
// ===========================
//
// Example MetaSprite ActionPoints for use in the untech-engine.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


RegisterBreakId(INVALID_ACTION_POINT)

// As ActionPoints are project definable, we place the ActionPoints
// in the global namespace.

namespace ActionPoints {

// Invalid ActionPoint - BSOD
a16()
i16()
code()
function InvalidActionPoint {
    break(INVALID_ACTION_POINT)
}

// Blank ActionPoint - does nothing
a16()
i16()
code()
function Null {
    rts
}


// Placeholder for a manual player projectile.
//
// This routine does nothing on MetaSprite.FrameSet.
// To spawn a projectile at this action point location, simply call
// `Entity.SpawnManualPlayerProjectiles`.
//
// NOTE: Manual projectile Action Points are slow and should not be used on
//       regular entities.  You should only use this Action Point in the Player
//       entity where an immediate response to a button press (in the middle of
//       an animation) is required.
constant ManualPlayerProjectile = Null



// Placeholder for a manual enemy projectile.
//
// This routine does nothing on MetaSprite.FrameSet.
// To spawn a projectile at this action point location, simply call
// `Entity.SpawnManualEnemyProjectiles`.
//
// NOTE: Manual projectile Action Points are slow and should not be used on
//       regular entities.
//       This action point is only added for consistency with `ManualPlayerProjectile`
//       and should not be used.
constant ManualEnemyProjectile = Null



// Spawn a player projectile at the action point
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP - entity
//         A - action point positionPair (int_ms8 pair)
constant PlayerProjectile = Entity._SpawnPlayerProjectileAtActionPoint

// Spawn an enemy projectile at the action point
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP - entity
//         A - action point positionPair (int_ms8 pair)
constant EnemyProjectile = Entity._SpawnEnemyProjectileAtActionPoint


// Creates a spawn entity action point function
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
inline SpawnEntityFunction(name, entityName) {
    assert16a()
    assert16i()
    code()
    function {name} {
        ldy.w   #Project.EntityIds.{entityName}
        jmp     Entity.SpawnEntityAtActionPoint
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

