// entity/spawn.inc
// ================
//
// Entity Spawning Routines.
//
// NOTE: Newly spawned entities are not immediately activated or added to their
// entity list when spawned.  Instead the newly spawned entity will be
// activated and moved to its entity list at the start of the next frame.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_ENTITY_ID)
RegisterBreakId(INVALID_ENTITY_LIST_ID)
RegisterBreakId(INVALID_PROJECTILE_ID)
Warnings.Register(ENTITY_POOL_EMPTY, 31, 0, 31)

namespace Entity {

code()
namespace Spawn {
a16()
i16()
EntityPoolEmpty:
    Warnings.Warn(ENTITY_POOL_EMPTY)

    clc
    rts
}

// Spawn a new entity.
//
// The newly created entity will be placed in either the:
//
//   * `activateNextFrame` special list if the Entity Init function
//      returns carry set **or** the entity is inside the active window.
//
//   * `deactivated` special list if the Entity Init function returns
//      carry clear and the entity is outside the active window.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: A - Entity Data Id
//        Y - entity parameter (passed to entity init function)
//        Entity.Spawn.xPos - entity x Position (uint16)
//        Entity.Spawn.yPos - entity y Position (uint16)
//
// OUTPUT: Y - address of new entity
//         C - set if entity created successfully
a16()
i16()
code()
function Spawn {

allocateTmpWord(xPos)
allocateTmpWord(yPos)

    cmp.w   #Entity.Data.EntityRomDataListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    asl
    tax

    lda.w   specialLists.free
    beq     EntityPoolEmpty

    phd
        phy

        // Remove Entity from free list

        tcd
        lda.b   BaseEntity.next
        sta.w   specialLists.free
        bne     +
            // There are no more free entities left, clear lastFreeEntity
            stz.w   lastFreeEntity
        +

        // Get Entity ROM Data

        lda.l   Entity.Data.EntityRomDataList,x
        tax
        stx.b   BaseEntity.romPtr


        // Setup BaseEntity

        lda.l   BaseEntityRomStruct.functionTable,x
        sta.b   BaseEntity.functionTable

        lda.w   xPos
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos.sx

        lda.w   yPos
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2


        // Set projectileId and listId
        assert(BaseEntity.projectileId + 1 == BaseEntity.listId)
        assert(BaseEntityRomStruct.initialProjectileId + 1 == BaseEntityRomStruct.initialListId)
        lda.l   BaseEntityRomStruct.initialProjectileId,x
        sta.b   BaseEntity.projectileId


        // Initiate the MetaSprite
        lda.l   BaseEntityRomStruct.defaultPalette,x
        tay
        lda.l   BaseEntityRomStruct.frameSetId,x

        MetaSprite._InitEntity()


        // Assign an new instanceId to the entity
        sep     #$10
    i8()
        Entity._InstanceIdTable.AssignInstanceId()
        rep     #$30
    i16()


        // Call Entity Init function
        ldx.b   BaseEntity.functionTable
        pla
        jsr     (BaseEntityFunctionTable.Init,x)

        // entity is active if, either
        //   A. The Init function returns carry set, or
        //   B. entity is inside active window

        bcs     SkipActiveWindowTest

            BranchEntityOutsideActiveWindow(IsInactive)

            SkipActiveWindowTest:
                // We cannot move the entity into its entity list while
                // the entity-loop is running.
                // Instead we move it to the `activateNextFrame` special list,
                // where it will be activated and moved to the correct
                // entity list at the start of the next frame.

                lda.w   specialLists.activateNextFrame
                sta.b   BaseEntity.next
                tdc
                sta.w   specialLists.activateNextFrame

                bra     EndIf


            InvalidEntityListId:
                break(INVALID_ENTITY_LIST_ID)


            IsInactive:
                // Insert the entity into the deactivated list

                lda.w   specialLists.deactivated
                sta.b   BaseEntity.next
                tdc
                sta.w   specialLists.deactivated

            EndIf:

        // A = DP
        tay
    pld

    sec
    rts
}



// Spawn a new entity at the MetaSprite Collision Centre.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A - Entity Data Id
//        Y - entity parameter (passed to entity init function)
//
// OUTPUT: Y - address of new entity
//         C - set if entity created successfully
function SpawnAtMetaSpriteCollisionCentre {
    ldx.w   MetaSprite.Collision.xPos
    stx.w   Entity.Spawn.xPos

    ldx.w   MetaSprite.Collision.yPos
    stx.w   Entity.Spawn.yPos

    jmp     Entity.Spawn
}



// Spawn a projectile at all {ap} action points in the current MetaSprite
// frame.
//
// When a projectile is spawned the action point position is stored in the
// variable `MetaSprite.ActionPoint.positionPair` (as a int_ms8 pair)
// and can be accessed by the projectile's `Init` routine.
//
// This routine will always place the newly created projectile in the
// `activateNextFrame` special list, regardless of the output of the
// projectile's Init routine.
//
// NOTE: This routine will invalidate `MetaSprite.ActionPoint.positionPair`.
//       DO NOT access `positionPair` after calling this function.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: DP - entity
macro __SpawnManualProjectiles(ap, listId) {
    assert16a()
    assert16i()

    ldx.b   BaseEntity.MetaSprite.currentFrame
    lda.l   MetaSprite.Format.Frame.actionPoints,x
    beq     Return

    tax

    lda.l   MetaSprite.Format.ActionPoints.type,x
    and.w   #0xff
    beq     Return

    Loop:
        cmp.w   #Project.ActionPoints.{ap}
        bne     Continue

            phx
                lda.l   MetaSprite.Format.ActionPoints.positionPair,x

                ldy.w   #EntityLists.{{listId}}
                jsr     __SpawnProjectileAtActionPoint
            plx

    Continue:
        inx
        inx
        inx
        lda.l   MetaSprite.Format.ActionPoints.type,x
        and.w   #0xff
        bne     Loop

Return:
}



if {defined Project.ActionPoints.ManualPlayerProjectile} {
// Spawn a player projectile at all `ManualPlayerProjectile` Action Points in
// the current MetaSprite Frame.
//
// When a projectile is spawned the action point position is stored in the
// variable `MetaSprite.ActionPoint.positionPair` (as a int_ms8 pair)
// and can be accessed by the projectile's `Init` routine.
//
// This routine will always place the newly created projectile in the
// `activateNextFrame` special list, regardless of the output of the
// projectile's Init routine.
//
// NOTE: This routine will invalidate `MetaSprite.ActionPoint.positionPair`.
//       DO NOT access `positionPair` after calling this function.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: DP - entity
a16()
i16()
code()
function SpawnManualPlayerProjectiles {
    __SpawnManualProjectiles(ManualPlayerProjectile, PLAYER_PROJECTILE_ENTITY_LIST)

    rts
}
}



if {defined Project.ActionPoints.ManualEnemyProjectile} {
// Spawn an enemy projectile at all `ManualEnemyProjectile` Action Points in
// the current MetaSprite Frame.
//
// When a projectile is spawned the action point position is stored in the
// variable `MetaSprite.ActionPoint.positionPair` (as a int_ms8 pair)
// and can be accessed by the projectile's `Init` routine.
//
// This routine will always place the newly created projectile in the
// `activateNextFrame` special list, regardless of the output of the
// projectile's Init routine.
//
// NOTE: This routine will invalidate `MetaSprite.ActionPoint.positionPair`.
//       DO NOT access `positionPair` after calling this function.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: DP - entity
a16()
i16()
code()
function SpawnManualEnemyProjectiles {
    __SpawnManualProjectiles(ManualEnemyProjectile, ENEMY_PROJECTILE_ENTITY_LIST)

    rts
}
}



// Spawn an entity at a given action point.
//
// This routine will always place the newly created entity in the
// `activateNextFrame` special list, regardless of the output of the
// projectile's Init routine.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: DP - entity
//         Y - Entity RomData Id
//         A - action point positionPair (int_ms8 pair)
function SpawnEntityAtActionPoint {

    cpy.w   #Entity.Data.EntityRomDataListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    sta.w   MetaSprite.ActionPoint.positionPair

    tya
    asl
    tax
    lda.l   Entity.Data.EntityRomDataList,x
    tax

    lda.l   BaseEntityRomStruct.initialListId,x
    tay

    bra     __SpawnAtActionPoint
}



code()
namespace __SpawnProjectileAtActionPoint {
SkipProjectile:
    rts
}



// Spawn a player projectile at an action point
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP - entity
//         A - action point positionPair (int_ms8 pair)
a16()
i16()
code()
function _SpawnPlayerProjectileAtActionPoint {
    ldy.w   #EntityLists.{PLAYER_PROJECTILE_ENTITY_LIST}
    bra     __SpawnProjectileAtActionPoint
}


// Spawn a enemy projectile at an action point
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP - entity
//         A - action point positionPair (int_ms8 pair)
a16()
i16()
code()
function _SpawnEnemyProjectileAtActionPoint {
    ldy.w   #EntityLists.{ENEMY_PROJECTILE_ENTITY_LIST}

FallThrough:
}


// Spawn an projectile at an action point
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP - entity
//         A - action point positionPair (int_ms8 pair)
//         Y - entity list id
a16()
i16()
code()
function __SpawnProjectileAtActionPoint {
    assert(_SpawnEnemyProjectileAtActionPoint.FallThrough == pc())

    sta.w   MetaSprite.ActionPoint.positionPair

    // Do not spawn projectile if projectileId is invalid
    lda.b   BaseEntity.projectileId
    and.w   #0xff
    cmp.w   #Entity.Data.ProjectileRomDataListCount
    bcs     SkipProjectile

    asl
    tax
    lda.l   Entity.Data.ProjectileRomDataList,x
    tax

FallThrough:
}


// Spawn an entity or projectile at the action point.
//
// When a projectile is spawned the action point position is stored in the
// variable `MetaSprite.ActionPoint.positionPair` (as a int_ms8 pair)
// and can be accessed by the projectile's `Init` routine.
//
// This routine will always place the newly created entity in the
// `activateNextFrame` special list, regardless of the output of the
// projectile's Init routine.
//
// This routine will override the `BaseEntityRomStruct.initialListId`
// with the value of the Y register.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP - entity
//         A - action point positionPair (int_ms8 pair)
//         Y - enemy projectile list id
//         X - EntityRomData address
//        MetaSprite.ActionPoint.positionPair = action point position
a16()
i16()
code()
function __SpawnAtActionPoint {
    assert(__SpawnProjectileAtActionPoint.FallThrough == pc())

    constant EntityPoolEmpty = __SpawnProjectileAtActionPoint.SkipProjectile

    // Get new entity address
    lda.w   Entity.specialLists.free
    beq     EntityPoolEmpty

    phd
        // remove entity from free list
        tcd
        lda.b   BaseEntity.next
        sta.w   Entity.specialLists.free
        bne     +
            // There are no more free entities left, clear lastFreeEntity
            stz.w   lastFreeEntity
        +

        // save entity listId
        // Y = listId
        sty.b   BaseEntity.listId
        // check if was safe to do a 16 bit write to listId
        assert(BaseEntity.listId + 1 == BaseEntity.MetaSprite.frameSet)


        // Calculate entity spawn location

        // load original entity address
        lda     1,s
        tay
        // A = original entity as routine argument

        clc
        lda.w   MetaSprite.ActionPoint.positionPair
        and.w   #0xff
        // carry clear, will set carry on overflow (thus the -1)
        adc.w   #-INT_MS8_OFFSET - 1
        adc.w   BaseEntity.xPos.px,y
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos.sx

        clc
        lda.w   MetaSprite.ActionPoint.positionPair + 1
        // no masking required, third byte of positionPair is always zero
        // carry clear, will set carry on overflow (thus the -1)
        adc.w   #-INT_MS8_OFFSET - 1
        adc.w   BaseEntity.yPos.px,y
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2


        // Initiate entity data
        // X = EntityRomData address
        stx.b   BaseEntity.romPtr

        lda.l   BaseEntityRomStruct.functionTable,x
        sta.b   BaseEntity.functionTable

        assert(BaseEntityRomStruct.defaultPalette + 1 == BaseEntityRomStruct.initialProjectileId)
        lda.l   BaseEntityRomStruct.defaultPalette,x
        tay
        // Y = defaultPalette
        // hiword A = initialProjectileId

        // Clear instanceId and set projectileId
        assert(BaseEntity.instanceId + 1 == BaseEntity.projectileId)
        and.w   #0xff00
        sta.b   BaseEntity.instanceId


        // Initiate the MetaSprite
        lda.l   BaseEntityRomStruct.frameSetId,x
        // Y = defaultPalette
        MetaSprite._InitEntity()


        // Call Entity Init function
        ldx.b   BaseEntity.functionTable
        lda     1,s
        // A = original entity as routine argument
        jsr     (BaseEntityFunctionTable.Init,x)

        // We cannot move the entity into its entity list while the
        // entity-loop is running.
        // Instead we move it to the `activateNextFrame` special list, where it
        // will be activated and moved to the correct entity list at the
        // start of the next frame.

        lda.w   specialLists.activateNextFrame
        sta.b   BaseEntity.next
        tdc
        sta.w   specialLists.activateNextFrame

    pld

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

