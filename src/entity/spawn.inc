
scope Entity {

// ::TODO separate projectile ROM data list::
// ::TODO create a projectile free list::
// ::DEBUG::
constant lists.projectileFree(Entity.lists.free)

scope Spawn {
ReturnFalse:
    clc
    rts
}

// Spawns a new entity and inserts it into its default list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: A - Entity Id
//        Y - entity parameter (passed to entity init function)
//        Create.xPos - entity x Position (word)
//        Create.yPos - entity y Position (word)
//
// OUTPUT: Y - address of new entity
//         C - set if entity created successfully
code()
a16()
i16()
scope Spawn: {

allocateTmpWord(xPos)
allocateTmpWord(yPos)

    // ::TODO limit entity Id
    asl
    tax

    lda.w   lists.free
    beq     ReturnFalse

    phd
        phy

        // Remove Entity from free list

        tcd
        lda.b   BaseEntity.next
        sta.w   lists.free


        // Get Entity ROM Data

        lda.l   Entity.Data.List,x
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
        stz.b   BaseEntity.xVecl + 4


        lda.l   BaseEntityData.defaultPalette,x
        tay
        lda.l   BaseEntityData.frameSetId,x

        MetaSprite._InitEntity()


        // Call Entity Init function
        ldx.b   BaseEntity.functionTable
        pla
        jsr     (BaseEntityFunctionTable.Init,x)


        // Insert entity into the appropriate list

        scope ActiveTest {
            // entity is active if:
            //   entity list only contains active entities
            //   OR entity is inside active window

            // get initial list index
            ldx.b   BaseEntity.romPtr
            lda.l   BaseEntityData.initialList,x
            and.w   #0xfe
            cmp.w   #lists.N_LISTS * 2
            bcs     MoveToDeleteList

            tax
            lda.l   EntityAlwaysActiveList,x
            bne     IsActive

            Camera.BranchEntityOutsideActiveWindow(IsInactive)

            IsActive:
                // entity is activate.
                // Insert into its active list

                phx
                    jsr     MetaSprite.Activate
                plx
                bcc     CouldNotActivate

                // x = list index
                lda.w   lists.FIRST,x
                sta.b   BaseEntity.next
                tdc
                sta.w   lists.FIRST,x

                bra     EndIf

            MoveToDeleteList:
                // invalid list id, move to delete list
                ldy.w   #lists.toDelete
                bra     InsertIntoInactiveList

            CouldNotActivate:
                ldy.w   #lists.tryToActivateAgain
                bra     InsertIntoInactiveList

            IsInactive:
                ldy.w   #lists.deactivated

            InsertIntoInactiveList:
                // Insert into deactivated list, remembering the list
                // address for when it is reactivated

                // x = list index
                // y = deactivated list address
                txa
                clc
                adc.w   #lists.FIRST
                sta.b   BaseEntity._ft_currentList

                lda.w   0,y
                sta.b   BaseEntity.next
                tdc
                sta.w   0,y

            EndIf:

            // A = DP
        }
        tay
    pld

    sec
    rts

rodata(rom0)
// one if the entity list is always active
// zero if entity list can be deactivated
scope EntityAlwaysActiveList: {
    dw  1       // players
    dw  1       // bosses
    dw  0       // enemies
    dw  0       // npcs
    dw  0       // platforms
    dw  1       // particles

    assert((pc() - EntityAlwaysActiveList) == lists.N_LISTS * 2)
}

code()
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
scope SpawnAtActionPoint: {
allocateTmpWord(tmp_compare)
allocateTmpWord(tmp_entityToSpawn)

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
//        Y = entityId
//        A = type to match
scope SpawnEnemyProjectileFromActionPoint: {
    ldx.w   #lists.enemyProjectiles
    bra     _SpawnFromActionPoint
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
//        Y = entityId
//        A = type to match
scope SpawnPlayerProjectileFromActionPoint: {
    ldx.w   #lists.playerProjectiles
_fallthrough:
}


// Spawns a projectile at a given action point type
//
// INPUT: DP = entity
//        Y = projectile Id
//        A = type to match
//        X = projectile list address
a16()
i16()
code()
scope _SpawnFromActionPoint: {
allocateTmpWord(tmp_listAddr)
allocateTmpWord(tmp_compare)
allocateTmpWord(tmp_projectileDataPtr)
allocateTmpWord(tmp_entityDoingSpawning)

    assert(SpawnPlayerProjectileFromActionPoint._fallthrough == pc())

    sta.w   tmp_compare
    stx.w   tmp_listAddr

    ldx.b   BaseEntity.MetaSprite.currentFrame
    beq     Return

    lda.l   MetaSprite.Format.Frame.actionPoints,x
    beq     Return

    // A -> X, Y -> A
    tax
    tya
    txy

    //::TODO limit projectile Id
    //::TODO separate projectile data list::
    asl
    tax
    lda.l   Entity.Data.List,x
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


// Spawns a projectile then continues the loop
SpawnProjectile:
        rep     #$31
    a16()
        lda.w   lists.projectileFree
        beq     ReturnDp

        stx.w   MetaSprite.ActionPoint.address

        // remove entity from projectileFree list
        tcd
        lda.b   BaseEntity.next
        sta.w   lists.projectileFree

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
        stz.b   BaseEntity.xVecl + 4

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

        // insert projectile into projectiles list
        ldx.w   tmp_listAddr
        lda.w   0,x
        sta.b   BaseEntity.next
        tdc
        sta.w   0,x

        ldx.w   MetaSprite.ActionPoint.address

        sep     #$20
    a8()
        jmp     Continue
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

