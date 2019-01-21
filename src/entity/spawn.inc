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

// Spawns a new entity and inserts it into its default list
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

    cmp.w   #Entity.Data.EntityDataListCount
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

        lda.l   Entity.Data.EntityDataList,x
        tax
        stx.b   BaseEntity.romPtr


        // Setup BaseEntity

        lda.l   BaseEntityData.functionTable,x
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

        lda.l   BaseEntityData.initialListId,x
        sta.b   BaseEntity.listId
        // check if was safe to do a 16 bit write to listId
        assert(BaseEntity.listId + 1 == BaseEntity.MetaSprite.frameSet)

        assert(BaseEntityData.initialListId + 1 == BaseEntityData.defaultPalette)
        xba
        tay
        lda.l   BaseEntityData.frameSetId,x

        MetaSprite._InitEntity()


        // Assign an new entityId to the entity
        sep     #$10
    i8()
        Entity._EntityIdTable.AssignEntityId()
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



// Spawns a given entity at the MetaSprite Collision Centre.
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



// Spawns a given entity on a given ActionPoint type
//
// This routine may spawn multiple entities.
//
// When entities are spawned the value `MetaSprite.ActionPoint.address`
// will be set to the address of the current Action Point.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// MUST NOT BE CALLED IN ENTITY CONSTRUCTOR
//
// INPUT: DP = entity
//        Y = Entity Data id
//        A = type to match
a16()
i16()
code()
function SpawnAtActionPoint {
allocateTmpWord(tmp_compare)
allocateTmpWord(tmp_entityToSpawn)

    cpy.w   #Entity.Data.EntityDataListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    sta.w   tmp_compare

    ldx.b   BaseEntity.MetaSprite.currentFrame
    lda.l   MetaSprite.Format.Frame.actionPoints,x
    beq     Return

    tax

    sty.w   tmp_entityToSpawn

    sep     #$20

a8()
    lda.l   MetaSprite.Format.ActionPoints.parameter,x
    beq     ReturnRep

    Loop:
        cmp.w   tmp_compare
        bne     SkipSpawn

            rep     #$31
        a16()
            lda.l   MetaSprite.Format.ActionPoints.xPos,x
            and.w   #0x00ff
            // carry clear, will set carry on overflow (thus the -1)
            adc.w   #-INT_MS8_OFFSET - 1
            adc.b   BaseEntity.xPos.px
            sta.w   Entity.Spawn.xPos

            clc
            lda.l   MetaSprite.Format.ActionPoints.yPos,x
            and.w   #0x00ff
            // carry clear, will set carry on overflow (thus the -1)
            adc.w   #-INT_MS8_OFFSET - 1
            adc.b   BaseEntity.yPos.px
            sta.w   Entity.Spawn.yPos

            stx.w   MetaSprite.ActionPoint.address
            tdc
            tay
            lda.w   tmp_entityToSpawn
            jsr     Entity.Spawn

            ldx.w   MetaSprite.ActionPoint.address

            sep     #$20
        a8()

    SkipSpawn:
        inx
        inx
        inx
        lda.l   MetaSprite.Format.ActionPoints.parameter,x
        bne     Loop

ReturnRep:
    rep     #$30
a16()
Return:
    rts
}



// Spawns one or more enemy projectiles from an ActionPoint type/location
//
// When entities are spawned the value `MetaSprite.ActionPoint.address`
// will be set to the address of the current Action Point.
//
// This routine will always activate the entity, regardless of the output of
// the entity's Init routine.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// MUST NOT BE CALLED IN ENTITY CONSTRUCTOR
//
// INPUT: DP = entity
//        Y = projectileId
//        A = type to match
a16()
i16()
code()
function SpawnEnemyProjectileFromActionPoint {
    ldx.w   #EntityLists.ENEMY_PROJECTILES
    bra     _SpawnProjectileFromActionPoint
}

// Spawns one or more player projectiles from an ActionPoint type/location
//
// When entities are spawned the value `MetaSprite.ActionPoint.address`
// will be set to the address of the current Action Point.
//
// This routine will always activate the entity, regardless of the output of
// the entity's Init routine.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// MUST NOT BE CALLED IN ENTITY CONSTRUCTOR
//
// INPUT: DP = entity
//        Y = projectile Id
//        A = type to match
a16()
i16()
code()
function SpawnPlayerProjectileFromActionPoint {
    ldx.w   #EntityLists.PLAYER_PROJECTILES
_fallthrough:
}


// Spawns a projectile at a given action point type
//
// INPUT: DP = entity
//        Y = projectile Id
//        A = type to match
//        X = list Id (ListId.PLAYER_PROJECTILE or ListId.ENEMY_PROJECTILE)
a16()
i16()
code()
function _SpawnProjectileFromActionPoint {
allocateTmpWord(tmp_listId)
allocateTmpWord(tmp_compare)
allocateTmpWord(tmp_projectileDataPtr)
allocateTmpWord(tmp_entityDoingSpawning)

    assert(SpawnPlayerProjectileFromActionPoint._fallthrough == pc())

    cpy.w   #Entity.Data.ProjectileListCount
    bcc     +
        break(INVALID_PROJECTILE_ID)
    +

    sta.w   tmp_compare
    stx.w   tmp_listId

    ldx.b   BaseEntity.MetaSprite.currentFrame
    lda.l   MetaSprite.Format.Frame.actionPoints,x
    beq     Return

    // A -> X, Y -> A
    tax
    tya
    txy

    asl
    tax
    lda.l   Entity.Data.ProjectileList,x
    sta.w   tmp_projectileDataPtr

    tyx

    tdc
    sta.w   tmp_entityDoingSpawning

    sep     #$20
a8()
    lda.l   MetaSprite.Format.ActionPoints.parameter,x
    beq     ReturnRepDp

    Loop:
        cmp.w   tmp_compare
        beq     SpawnProjectile

    Continue:
        inx
        inx
        inx
        lda.l   MetaSprite.Format.ActionPoints.parameter,x
        bne     Loop

ReturnRepDp:
    rep     #$30
a16()
ReturnDp:
    lda.w   tmp_entityDoingSpawning
    tcd

Return:
    rts

a16()
EntityPoolEmpty:
    Warnings.Warn(ENTITY_POOL_EMPTY)
    bra     ReturnDp


// Spawns a projectile then continues the loop
SpawnProjectile:
        rep     #$31
    a16()
        lda.w   Entity.specialLists.free
        beq     EntityPoolEmpty

        stx.w   MetaSprite.ActionPoint.address

        // remove entity from free list
        tcd
        lda.b   BaseEntity.next
        sta.w   Entity.specialLists.free
        bne     +
            // There are no more free entities left, clear lastFreeEntity
            stz.w   lastFreeEntity
        +

        // save entity listId
        lda.w   tmp_listId
        sta.b   BaseEntity.listId
        // check if was safe to do a 16 bit write to listId
        assert(BaseEntity.listId + 1 == BaseEntity.MetaSprite.frameSet)

        // calculate entity location
        ldy.w   tmp_entityDoingSpawning

        // carry clear (NO INSTRUCTION ABOVE MODIFIES CARRY)
        lda.l   MetaSprite.Format.ActionPoints.xPos,x
        and.w   #0x00ff
        // carry clear, will set carry on overflow (thus the -1)
        adc.w   #-INT_MS8_OFFSET - 1
        adc.w   BaseEntity.xPos.px,y
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos.sx

        clc
        lda.l   MetaSprite.Format.ActionPoints.yPos,x
        and.w   #0x00ff
        // carry clear, will set carry on overflow (thus the -1)
        adc.w   #-INT_MS8_OFFSET - 1
        adc.w   BaseEntity.yPos.px,y
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        // Get Entity ROM data
        ldx.w   tmp_projectileDataPtr
        stx.b   BaseEntity.romPtr

        lda.l   BaseEntityData.functionTable,x
        sta.b   BaseEntity.functionTable

        // Initiate the MetaSprite
        lda.l   BaseEntityData.defaultPalette,x
        tay
        lda.l   BaseEntityData.frameSetId,x
        MetaSprite._InitEntity()

        // Call Entity Init function
        ldx.b   BaseEntity.functionTable
        lda.w   tmp_entityDoingSpawning
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

        ldx.w   MetaSprite.ActionPoint.address

        sep     #$20
    a8()
        jmp     Continue


InvalidEntityListId:
    break(INVALID_ENTITY_LIST_ID)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

