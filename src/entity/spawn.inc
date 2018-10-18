// entity/spawn.inc
// ================
//
// Entity Spawning Routines.
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
// INPUT: A - Entity Id
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

    cmp.w   #Entity.Data.EntityListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    asl
    tax

    lda.w   lists.free
    beq     EntityPoolEmpty

    phd
        phy

        // Remove Entity from free list

        tcd
        lda.b   BaseEntity.next
        sta.w   lists.free


        // Get Entity ROM Data

        lda.l   Entity.Data.EntityList,x
        tax
        stx.b   BaseEntity.romPtr


        // Setup BaseEntity

        lda.l   BaseEntityData.functionTable,x
        sta.b   BaseEntity.functionTable

        lda.w   xPos
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos

        lda.w   yPos
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos

        stz.b   BaseEntity.xVecl
        stz.b   BaseEntity.xVecl + 2
        stz.b   BaseEntity.yVecl
        stz.b   BaseEntity.yVecl + 2

        lda.l   BaseEntityData.initialListId,x
        sta.b   BaseEntity.listId

        lda.l   BaseEntityData.defaultPalette,x
        tay
        lda.l   BaseEntityData.frameSetId,x

        MetaSprite._InitEntity()


        // Call Entity Init function
        ldx.b   BaseEntity.functionTable
        pla
        jsr     (BaseEntityFunctionTable.Init,x)


        // Insert entity into the appropriate list

        namespace ActiveTest {
            // entity is active if:
            //   entity list only contains active entities
            //   OR entity is inside active window

            // get initial list index
            lda.b   BaseEntity.listId
            and.w   #0xff
            cmp.w   #lists.N_LISTS
            bcs     InvalidEntityListId

            tax
        a16()
            lda.l   EntityAlwaysActiveList - 1,x
            bmi     IsActive

            BranchEntityOutsideActiveWindow(IsInactive)

            IsActive:
                // entity is activate.
                // Activate and insert into the entity list

                jsr     MetaSprite.Activate
                bcc     CouldNotActivate

                lda.b   BaseEntity.listId
                and.w   #0xff
                asl
                tax

                bra     MoveToListX


            InvalidEntityListId:
                break(INVALID_ENTITY_LIST_ID)

            CouldNotActivate:
                ldx.w   #lists._TRY_TO_ACTIVATE_AGAIN_INDEX
                bra     MoveToListX

            IsInactive:
                ldx.w   #lists._DEACTIVATED_INDEX

            MoveToListX:
                // Insert the entity into the given list Id index

                lda.w   lists.FIRST,x
                sta.b   BaseEntity.next
                tdc
                sta.w   lists.FIRST,x

                bra     EndIf

            EndIf:

            // A = DP
        }
        tay
    pld

    sec
    rts

rodata(rom0)
// a negative number if the entity list is always active
// a positive number if entity list can be deactivated
EntityAlwaysActiveList: {
    db  0xff    // players
    db  0xff    // bosses
    db  0       // enemies
    db  0       // npcs
    db  0       // platforms
    db  0xff    // particles
    db  0xff    // playerProjectiles
    db  0xff    // enemyProjectiles

    assert((pc() - EntityAlwaysActiveList) == lists.N_LISTS)
}

code()
}



// Spawns a given entity at the MetaSprite Collision Centre.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A - Entity Id
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
//        Y = entityId
//        A = type to match
a16()
i16()
code()
function SpawnAtActionPoint {
allocateTmpWord(tmp_compare)
allocateTmpWord(tmp_entityToSpawn)

    cpy.w   #Entity.Data.EntityListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    sta.w   tmp_compare

    ldx.b   BaseEntity.MetaSprite.currentFrame
    beq     Return

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
// Will assume that the MetaSprite will always activate, will ignore
// out of VRAM/CGRAM errors and glitch out.
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
    ldx.w   #ListId.ENEMY_PROJECTILE
    bra     _SpawnProjectileFromActionPoint
}

// Spawns one or more player projectiles from an ActionPoint type/location
//
// When entities are spawned the value `MetaSprite.ActionPoint.address`
// will be set to the address of the current Action Point.
//
// Will assume that the MetaSprite will always activate, will ignore
// out of VRAM/CGRAM errors and glitch out.
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
    ldx.w   #ListId.PLAYER_PROJECTILE
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
    beq     Return

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
        lda.w   Entity.lists.free
        beq     EntityPoolEmpty

        stx.w   MetaSprite.ActionPoint.address

        // remove entity from free list
        tcd
        lda.b   BaseEntity.next
        sta.w   Entity.lists.free

        // save entity listId
        lda.w   tmp_listId
        sta.b   BaseEntity.listId

        // calculate entity location
        ldy.w   tmp_entityDoingSpawning

        // carry clear (NO INSTRUCTION ABOVE MODIFIES CARRY)
        lda.l   MetaSprite.Format.ActionPoints.xPos,x
        and.w   #0x00ff
        // carry clear, will set carry on overflow (thus the -1)
        adc.w   #-INT_MS8_OFFSET - 1
        adc.w   BaseEntity.xPos.px,y
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos

        clc
        lda.l   MetaSprite.Format.ActionPoints.yPos,x
        and.w   #0x00ff
        // carry clear, will set carry on overflow (thus the -1)
        adc.w   #-INT_MS8_OFFSET - 1
        adc.w   BaseEntity.yPos.px,y
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos

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

        // Assume projectiles always activate, even if it causes glitches.
        jsr     MetaSprite.Activate


        // Insert projectile into the entity list

        lda.b   BaseEntity.listId
        and.w   #0xff
        cmp.w   #Entity.lists.N_LISTS
        beq     InvalidEntityListId

        asl
        tax

        lda.w   lists.FIRST,x
        sta.b   BaseEntity.next
        tdc
        sta.w   lists.FIRST,x

        ldx.w   MetaSprite.ActionPoint.address

        sep     #$20
    a8()
        jmp     Continue


InvalidEntityListId:
    break(INVALID_ENTITY_LIST_ID)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

