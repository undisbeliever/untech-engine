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


RegisterBreakId(INVALID_ENTITY_ID)
RegisterBreakId(INVALID_ENTITY_LIST_ID)
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



// Spawn a new entity at the MetaSprite Collision Centre.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A - Entity Data Id
//        Y - entity parameter (passed to entity init function)
//
// OUTPUT: Y - address of new entity
//         C - set if entity created successfully
a16()
i16()
code()
function SpawnAtMetaSpriteCollisionCentre {
    ldx.w   MetaSprite.Collision.xPos
    stx.w   Entity.Spawn.xPos

    ldx.w   MetaSprite.Collision.yPos
    stx.w   Entity.Spawn.yPos

FallThrough:
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

    assert(pc() == SpawnAtMetaSpriteCollisionCentre.FallThrough)

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
    // DP = entity
        lda.b   BaseEntity.next
        sta.w   specialLists.free
        bne     +
            // There are no more free entities left, clear lastFreeEntity
            stz.w   lastFreeEntity
        +

        // Get Function Table
        // X = Entity Data Id * 2
        lda.l   Entity.Data.FunctionTables,x
        sta.b   BaseEntity.functionTable


        // Get Entity ROM Data

        lda.l   Entity.Data.EntityRomDataList,x
        tax
        stx.b   BaseEntity.romPtr


        // Setup BaseEntity

        lda.w   xPos
        sta.b   BaseEntity.xPos.px
        stz.b   BaseEntity.xPos.sx

        lda.w   yPos
        sta.b   BaseEntity.yPos.px
        stz.b   BaseEntity.yPos.sx

        // Confirm entity momentum is a 24 bit value
        assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)
        stz.b   BaseEntity.xMomentum.sx
        stz.b   BaseEntity.xMomentum.sx + 2
        stz.b   BaseEntity.xMomentum.sx + 4

        // Clear movementState and gravity
        assert(BaseEntity.gravity == BaseEntity.movementState + 1)
        stz.b   BaseEntity.movementState


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
        Entity._InstanceIdTable._AssignNewInstanceId()
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



// Spawns the player entity
//
// NOTE: This function overrides the previous entity.
//
// NOTE: This function will always activate the entity.
//
// MUST ONLY BE INVOKED BY `Room.Entrances.SpawnPlayer`
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E, DP = 0
//
// ASSUMES: MetaSprite Palette/VRAM slots empty
// ASSUMES: Dma Queue empty
//
// INPUT: A = movementState of the player
//        Room.playerDataId = the Data id of the player
//        Entity.Player.xPos.px, Entity.Player.yPos.px = player position
macro _SpawnPlayer() {
    assert16a()
    assert16i()

    // Y = Room.Format.RoomEntity address

    sep     #$20
a8()
    sta.w   Entity.playerEntity + BaseEntity.movementState

    lda.w   GameState.playerDataId
    cmp.b   #Entity.Data.PlayerRomDataListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    stz.w   Entity.playerEntity + BaseEntity.gravity

    rep     #$31
a16()
    and.w   #0xff
    // carry clear
    adc.w   #Entity.Data.PlayerRomDataListOffset
    asl
    tax


    lda.w   #Entity.playerEntity
    tcd
// DP = playerEntity

    // player entity has no next entity
    stz.b   BaseEntity.next

    // Get Function Table
    // X = entity Data Id * 2
    lda.l   Entity.Data.FunctionTables,x
    sta.b   BaseEntity.functionTable

    // Get Entity ROM Data
    lda.l   Entity.Data.EntityRomDataList,x
    tax
    stx.b   BaseEntity.romPtr

    // Reset player subpixel position (position set by Room.Entrances._SpawnPlayer)
    stz.b   BaseEntity.xPos.sx
    stz.b   BaseEntity.yPos.sx

    // Reset momentum
    assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)
    stz.b   BaseEntity.xMomentum.sx
    stz.b   BaseEntity.xMomentum.sx + 2
    stz.b   BaseEntity.xMomentum.sx + 4


    // Player entities have no instanceId
    assert(BaseEntity.instanceId + 1 == BaseEntity.projectileId) // check if safe to preform a 16 bit store here)
    stz.b   BaseEntity.instanceId

    // Set projectileId and clear listId
    assert(BaseEntity.projectileId + 1 == BaseEntity.listId)
    assert(BaseEntityRomStruct.initialProjectileId + 1 == BaseEntityRomStruct.initialListId)
    lda.l   BaseEntityRomStruct.initialProjectileId,x
    ora.w   #0xff00
    sta.b   BaseEntity.projectileId

    // Initiate the MetaSprite
    lda.l   BaseEntityRomStruct.defaultPalette,x
    tay
    lda.l   BaseEntityRomStruct.frameSetId,x

    MetaSprite._InitEntity()


    // Call Player Init function
    ldx.b   BaseEntity.functionTable
    lda.w   #0
    jsr     (BasePlayerFunctionTable.Init,x)


    // Activate player
    //
    // This should always succeed as the first thing the room subsystem does is load the player entity
    jsr     MetaSprite.Activate


    lda.w   #0
    tcd
// DP = 0
}



// Spawn a new room entity from an room entity group
//
// MUST ONLY BE INVOKED BY `Room.EntityGroups.Spawn_Entity_Group__BgTask.ProcessBgTask`
//
// The newly created entity will be given an instanceId of `_currentEntityInstanceId` (if it is free).
//
// The newly created entity will be placed in either the:
//
//   * `activateNextFrame` special list if the Entity Init function
//      returns carry set **or** the entity is inside the active window.
//
//   * `deactivated` special list if the Entity Init function returns
//      carry clear and the entity is outside the active window.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E, DP = 0
//
// INPUT: Y = Room.Format.RoomEntity address
// INPUT: _currentEntityInstanceId = instanceId of the new entity (byte)
//
// BRANCH: EntityPoolEmpty if no more entities can be spawned
macro _SpawnRoomEntity() {
    assert16a()
    assert16i()

    // Y = Room.Format.RoomEntity address

    lda.w   Room.Format.RoomEntity.entityDataId,y
    and.w   #0xff
    cmp.w   #Entity.Data.EntityRomDataListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    asl
    tax

    lda.w   Entity.specialLists.free
    beq     EntityPoolEmpty


    // Remove Entity from free list

    tcd
// DP = entity
    lda.b   BaseEntity.next
    sta.w   Entity.specialLists.free
    bne     +
        // There are no more free entities left, clear lastFreeEntity
        stz.w   Entity.lastFreeEntity
    +

    // Get Function Table
    // X = entity Data Id * 2
    lda.l   Entity.Data.FunctionTables,x
    sta.b   BaseEntity.functionTable


    // Get Entity ROM Data

    lda.l   Entity.Data.EntityRomDataList,x
    tax
    stx.b   BaseEntity.romPtr


    // Set BaseEntity position

    // Confirm position is 3 bytes in size
    assert(Room.Format.RoomEntity.position + 3 == Room.Format.RoomEntity.parameter)

    lda.w   Room.Format.RoomEntity.position,y
    and.w   #Room.Format.RoomEntity.POSITION_MASK
    clc
    adc.w   #Room.Format.RoomEntity.POSITION_X_OFFSET
    sta.b   BaseEntity.xPos.px
    stz.b   BaseEntity.xPos.sx

    assert(Room.Format.RoomEntity.POSITION_SHIFT == 8 + 4)
    lda.w   Room.Format.RoomEntity.position + 1,y
    lsr
    lsr
    lsr
    lsr
    and.w   #Room.Format.RoomEntity.POSITION_MASK
    clc
    adc.w   #Room.Format.RoomEntity.POSITION_Y_OFFSET
    sta.b   BaseEntity.yPos.px
    stz.b   BaseEntity.yPos.sx


    // Confirm parameter is byte
    assert(Room.Format.RoomEntity.size - Room.Format.RoomEntity.parameter == 1)
    lda.w   Room.Format.RoomEntity.parameter,y
    and.w   #0xff
    // Store parameter for later
    pha

    // Can now modify Y

    // Confirm entity momentum is a 24 bit value
    assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)
    stz.b   BaseEntity.xMomentum.sx
    stz.b   BaseEntity.xMomentum.sx + 2
    stz.b   BaseEntity.xMomentum.sx + 4

    // Clear movementState and gravity
    assert(BaseEntity.gravity == BaseEntity.movementState + 1)
    stz.b   BaseEntity.movementState


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


    // Assign and increment _currentEntityInstanceId
    sep     #$10
i8()
    ldx.w   _currentEntityInstanceId

    // X = new entity instanceId
    Entity._InstanceIdTable._SetInstanceId()

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

        Entity.BranchEntityOutsideActiveWindow(IsInactive)

        SkipActiveWindowTest:
            // We cannot move the entity into its entity list while
            // the entity-loop is running.
            // Instead we move it to the `activateNextFrame` special list,
            // where it will be activated and moved to the correct
            // entity list at the start of the next frame.

            lda.w   Entity.specialLists.activateNextFrame
            sta.b   BaseEntity.next
            tdc
            sta.w   Entity.specialLists.activateNextFrame

            bra     EndIf


        InvalidEntityListId:
            break(INVALID_ENTITY_LIST_ID)


        IsInactive:
            // Insert the entity into the deactivated list

            lda.w   Entity.specialLists.deactivated
            sta.b   BaseEntity.next
            tdc
            sta.w   Entity.specialLists.deactivated

        EndIf:

    // Restore DP back to 0
    lda.w   #0
    tcd
// DP = 0
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
//         Y - Entity Data Id
//         A - action point positionPair (int_ms8 pair)
function SpawnEntityAtActionPoint {

    cpy.w   #Entity.Data.EntityRomDataListCount
    bcc     +
        break(INVALID_ENTITY_ID)
    +

    sta.w   MetaSprite.ActionPoint.positionPair

    tya

    // get listId from BaseEntityRomStruct
    ldy.w   #0xffff

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

    // Projectiles are stored directly after entity data
    // Carry clear
    adc.w   #Entity.Data.EntityRomDataListCount

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
//         A - entity Rom Id (bound checked)
//         Y - enemy projectile list id (if negative then BaseEntityRomStruct.initialListId is used instead)
//        MetaSprite.ActionPoint.positionPair = action point position
a16()
i16()
code()
function __SpawnAtActionPoint {
    assert(__SpawnProjectileAtActionPoint.FallThrough == pc())

    constant EntityPoolEmpty = __SpawnProjectileAtActionPoint.SkipProjectile

    asl
    tax

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


        // Get Function Table
        // X = Entity Data Id * 2
        lda.l   Entity.Data.FunctionTables,x
        sta.b   BaseEntity.functionTable


        // Get Entity ROM Data

        lda.l   Entity.Data.EntityRomDataList,x
        tax
        stx.b   BaseEntity.romPtr


        // HACK
        // ::TODO remove when adding projectiles subsystem ::
        // ::: replace with `Set projectileId and listId` from `SpawnEntity`::

        // Y = listId
        tya
        bpl     +
            // If Y was negative, use listId from BaseEntityRomStruct
            lda.l   BaseEntityRomStruct.initialListId,x
        +
        sta.b   BaseEntity.listId
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

        // Confirm entity momentum is a 24 bit value
        assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)
        stz.b   BaseEntity.xMomentum.sx
        stz.b   BaseEntity.xMomentum.sx + 2
        stz.b   BaseEntity.xMomentum.sx + 4

        // Clear movementState and gravity
        assert(BaseEntity.gravity == BaseEntity.movementState + 1)
        stz.b   BaseEntity.movementState


        // Initiate entity data
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

