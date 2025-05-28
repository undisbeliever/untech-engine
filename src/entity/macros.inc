// entity/macros.inc
// =================
//
// Public API for the Entities namespace.
//
// Project Entity functions (inside the Entities namespace) SHOULD ONLY
// be using these macros to invoke an untech-engine routine.
//
// SPDX-FileCopyrightText: © 2019 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2019 Marcus Rowe <undisbeliever@gmail.com>
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

// Retrieves the frameId for the given frame in the current ExportOrder
define GetFrameId(fName) = {ExportOrder}.Frames.{fName}

// Retrieves the animationId for the given animation in the current ExportOrder
define GetAnimationId(aName) = {ExportOrder}.Animations.{aName}

// Retrieves the entityId
define GetEntityId(eName) = Project.EntityIds.{eName}

// Retrieves the projectileId
define GetProjectileId(pName) = Project.ProjectileIds.{pName}


// Moves the entity (using the entity's movementState and momentum).
//
// NOTE: The entity's standing flag is ignored by this routine.
//
// NOTE: This routine will apply gravity to the entity's `yMomentum` if the
//       entity's `NO_GRAVITY_FLAG` movementState flag is clear.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: dp = entity
macro MoveEntity() {
    assert16a()
    assert16i()

    jsr     Entity.Movement.MoveEntity
}

// Moves the entity (using the entity's movementState and momentum values) and
// preforms collision checks between the MetaTiles and the entity's tile
// hitbox.
//
// See MetaTiles.MoveEntityAndProcessTileCollisions for more details.
//
// ASSUMES: The entity is inside the map boundary.
//
// NOTE: This macro MUST ONLY be invoked in the entity's Process routine.
//
// NOTE: If the entity's MetaSprite frame has no tile hitbox then the entity
//       will not be moved.
//
// NOTE: This routine will apply gravity to the entity's `yMomentum` if the
//       entity's `NO_GRAVITY_FLAG` movementState flag is clear.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: dp = entity
// OUTPUT: carry set if collision occurred in Y-Axis
//         Y = The tile index of the tile that collided in the Y-Axis (if carry set).
macro MoveEntityAndProcessTileCollisions() {
    assert16a()
    assert16i()

    jsl     MetaTiles.MoveEntityAndProcessTileCollisions__far
}



// Move entity, process tile collisions and call any interactive tile functions involving the entity.
//
// This function will call the `InteractiveTiles.Entity*` interactive tile functions.
//
// This function SHOULD NOT be called by a player entity.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
macro MoveEntityWithTileCollisionsAndInteractiveTiles() {
    assert16a()
    assert16i()

    jsr     MetaTiles.MoveEntityWithTileCollisionsAndInteractiveTiles
}



// Move player, process tile collisions and call any interactive tile functions involving the player.
//
// This function will call the `InteractiveTiles.Player*` interactive tile functions.
//
// This function MUST NOT be called by a non-player entity.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
macro MovePlayerWithTileCollisionsAndInteractiveTiles() {
    assert16a()
    assert16i()

    jsr     MetaTiles.MovePlayerWithTileCollisionsAndInteractiveTiles
}



// Sets the Entity's MetaSprite frame
//
// NOTE: See `Entities.SetFrame_A()` for more details about how this macro
//       works.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7e
//
// PARAM: frameId - the frame to select within the current ExportOrder
// INPUT: dp - entity
// OUTPUT: carry set if successful
macro SetFrame(frameId) {
    assert16a()
    assert16i()

    lda.w   #{ExportOrder}.Frames.{frameId}
    jsr     MetaSprite.SetFrame
}


// SEE: MetaSprite.SetFrame
macro SetFrame_A() {
    assert16a()
    assert16i()

    jsr     MetaSprite.SetFrame
}



// Set the entity's MetaSprite palette
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// INPUT: A - PaletteId
// OUTPUT: carry set if successful
macro SetPalette_A() {
    assert8a()
    assert16i()

    jsr     MetaSprite.Palette.SetPalette
}

// Set the entity's MetaSprite palette
//
// REQUIRES: 8 bit A, 16 bit Index, DB = 0x7e
// PARAM: pal - palette number
// OUTPUT: carry set if successful
macro SetPalette(evaluate pal) {
    assert8a()
    assert16i()

    lda.b   #{pal}
    jsr     MetaSprite.Palette.SetPalette
}



// Set the entity's animation
//
// NOTE: Will not reset the animation if the animation is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// PARAM: animationId - the animation with the current ExportOrder
// INPUT: dp - entity
macro SetAnimation(animationId) {
    assert16a()
    assert16i()

    lda.w   #{ExportOrder}.Animations.{animationId}
    jsr     MetaSprite.Animation.SetAnimationId
}

// Set the entity's animation to the AnimationId in the A register.
//
// NOTE: Will not reset the animation if the animation is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp - entity
//        A - animationId
macro SetAnimation_A() {
    assert16a()
    assert16i()

    jsr     MetaSprite.Animation.SetAnimationId
}

// Disable the entity's animation
//
// REQUIRES: 16 bit A
//
// INPUT: dp - entity
macro DisableAnimation() {
    assert16a()

    jsr     MetaSprite.Animation.DisableAnimation
}



// Sets the Entity's nextAnimation to a given animation
//
// REQUIRES: 8 bit A or 8 bit Index
//
// PARAM: animation - animation Name
macro SetNextAnimation(animation) {
    if __CPU__.aSize == 8 {
        lda.b   #{ExportOrder}.Animations.{animation}
        sta.b   BaseEntity.MetaSprite.Animation.nextAnimation
    } else if __CPU__.iSize == 8 {
        ldy.b   #{ExportOrder}.Animations.{animation}
        sty.b   BaseEntity.MetaSprite.Animation.nextAnimation
    } else {
        error "Expected 8 bit A or 8 bit Index"
    }
}

// Sets the Entity's nextAnimation
//
// REQUIRES: 8 bit A
//
// INPUT: A = Animation Id
macro SetNextAnimation_A() {
    assert8a()

    sta.b   BaseEntity.MetaSprite.Animation.nextAnimation
}

// Sets the Entity's nextAnimation
//
// REQUIRES: 8 bit Index
//
// INPUT: Y = Animation Id
macro SetNextAnimation_Y() {
    assert8i()

    sty.b   BaseEntity.MetaSprite.Animation.nextAnimation
}

// Sets the Entity's nextAnimation
//
// REQUIRES: 8 bit Index
//
// INPUT: X = Animation Id
macro SetNextAnimation_X() {
    assert8i()

    stx.b   BaseEntity.MetaSprite.Animation.nextAnimation
}



// Sets the Entity's projectileId
//
// REQUIRES: 8 bit A or 8 bit Index
//
// PARAM: projectile - projectile Name
macro SetProjectileId(projectile) {
    if __CPU__.aSize == 8 {
        lda.b   #Project.ProjectileIds.{projectile}
        sta.b   BaseEntity.projectileId
    } else if __CPU__.iSize == 8 {
        ldy.b   #Project.ProjectileIds.{projectile}
        sty.b   BaseEntity.projectileId
    } else {
        error "Expected 8 bit A or 8 bit Index"
    }
}

// Sets the Entity's projectileId
//
// REQUIRES: 8 bit A
//
// INPUT: A = projectileId
macro SetProjectileId_A() {
    assert8a()

    sta.b   BaseEntity.projectileId
}

// Sets the Entity's projectileId
//
// REQUIRES: 8 bit Index
//
// INPUT: Y = projectileId
macro SetProjectileId_Y() {
    assert8i()

    sty.b   BaseEntity.projectileId
}

// Sets the Entity's projectileId
//
// REQUIRES: 8 bit Index
//
// INPUT: X = projectileId
macro SetProjectileId_X() {
    assert8i()

    stx.b   BaseEntity.projectileId
}



// Spawn new entity.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// PARAM: entityId - the name of the entity to spawn
// INPUT: Y - entity parameter (passed to entity init function)
//        SpawnEntity.xPos - entity x Position (uint16)
//        SpawnEntity.yPos - entity y Position (uint16)
//
// OUTPUT: Y - address of new entity
//         C - set if entity created successfully
macro SpawnEntity(entityId) {
    assert16a()
    assert16i()

    lda.w   #Project.EntityIds.{entityId}
    jsr     Entity.Spawn
}
constant SpawnEntity.xPos = Entity.Spawn.xPos
constant SpawnEntity.yPos = Entity.Spawn.yPos



// Spawn a new entity at the MetaSprite Collision centre.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// PARAM: entityId - the name of the entity to spawn
// INPUT: Y - entity parameter (passed to entity init function)
//
// OUTPUT: carry set if entity created successfully
//         Y - address of new entity
macro SpawnEntityAtMetaSpriteCollisionCentre(entityId) {
    assert16a()
    assert16i()

    lda.w   #Project.EntityIds.{entityId}
    jsr     Entity.SpawnAtMetaSpriteCollisionCentre
}



// SEE Entity.SpawnManualPlayerProjectiles
macro SpawnManualPlayerProjectiles() {
    assert16a()
    assert16i()

    jsr     Entity.SpawnManualPlayerProjectiles
}

// SEE Entity.SpawnManualEnemyProjectiles
macro SpawnManualEnemyProjectiles() {
    assert16a()
    assert16i()

    jsr     Entity.SpawnManualEnemyProjectiles
}



// SEE: Entity.CountEntitiesInList
inline CountEntitiesInList(listName) {
    Entity.CountEntitiesInList({listName})
}

// Branch to a label if a given entity list is empty
//
// REQUIRES: 16 bit Index, DB = 0x7e
//
// PARAM: listName - the entity list name
// BRANCH: to `label` if entity list is empty
macro BranchIfEntityListEmpty(listName, label) {
    assert16i()

    ldy.w   {Entity.listIndex({listName})}
    beq     {label}
}

// Branch to a label if a given entity list is not empty
//
// REQUIRES: 16 bit Index, DB = 0x7e
//
// PARAM: listName - the entity list name
// BRANCH: to `label` if entity list is not empty
macro BranchIfEntityListNotEmpty(listName, label) {
    assert16i()

    ldy.w   {Entity.listIndex({listName})}
    bne     {label}
}



// SEE: Entity.GetEntityWithIdOrBranchIfEmpty_Y
inline GetEntityWithIdOrBranchIfEmpty_Y(label) {
    Entity.GetEntityWithIdOrBranchIfEmpty_Y({label})
}

// SEE: Entity.GetEntityWithIdOrBranchIfEmpty_X
inline GetEntityWithIdOrBranchIfEmpty_X(label) {
    Entity.GetEntityWithIdOrBranchIfEmpty_X({label})
}

// SEE: Entity.GetEntityWithIdOrBranchIfEmpty_A8Y
inline GetEntityWithIdOrBranchIfEmpty_A8Y(label) {
    Entity.GetEntityWithIdOrBranchIfEmpty_A8Y({label})
}

// SEE: Entity.GetEntityWithIdOrBranchIfEmpty_A8X
inline GetEntityWithIdOrBranchIfEmpty_A8X(label) {
    Entity.GetEntityWithIdOrBranchIfEmpty_A8X({label})
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

