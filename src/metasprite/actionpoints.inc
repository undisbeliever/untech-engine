// metasprite/actionpoints.inc
// ===========================
//
// Example MetaSprite ActionPoints for use in the untech-engine.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

