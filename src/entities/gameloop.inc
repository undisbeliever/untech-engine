
scope Entities {

// Previous entity being processed
// (word address)
allocateTmpWord(previousEntity)

macro _LOOP(list) {
    assert16a()
    assert16i()

    global evaluate _LOOP_LIST({list})

    // this allows previousEntity variable to work on first item
    assert(BaseEntity.next == 0)

    lda.w   {list}
    beq     EndLoop

    ldy.w   #{list}
    sty.w   currentList
    sty.w   previousEntity

    stz.w   changeList

    Loop:
        tcd
}

macro _LOOP_Y() {
    assert16a()
    assert16i()

    global evaluate _LOOP_LIST(-1)

    // this allows previousEntity variable to work on first item
    assert(BaseEntity.next == 0)

    lda.w   0,y
    beq     EndLoop

    sty.w   currentList
    sty.w   previousEntity

    stz.w   changeList

    Loop:
        tcd
}

macro _NEXT() {
    assert16a()
    assert16i()

        ldy.w   changeList
        bne     ChangeEntityList
            // Continue loop as normal
        _GotoNext:
            tdc
            sta.w   previousEntity
            lda.b   BaseEntity.next

            bne     Loop
            bra     EndLoop

        ChangeEntityList:
            // Change the list the entity is in THEN return
            // Y = list to change to (NOT NULL)

            // don't reorder list if unnecessary
            if {_LOOP_LIST} == -1 {
                cpy.w   currentList
                beq     _GotoNext
            } else {
                cpy.w   #{_LOOP_LIST}
                beq     _GotoNext
            }

            // previousEntity->next = dp->next
            // dp->next = *changeList
            // *changeList = this
            //
            // changeList = NULL
            //
            // next = previousEntity->next

            ldx.w   previousEntity

            lda.b   BaseEntity.next
            sta.w   BaseEntity.next,x

            lda.w   0,y
            sta.b   BaseEntity.next

            tdc
            sta.w   0,y

            stz.w   changeList

            lda.w   BaseEntity.next,x
            bne     Loop
EndLoop:
}


// Render a list of entities
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: Y - address of list
// MODIFIES: DP
a16()
i16()
code()
scope ProcessEntityList_DeactivateOffscreen: {
    _LOOP_Y()
        ldx.b   BaseEntity.functionTable
        jsr     (BaseEntityFunctionTable.Process,x)

        // Skip Offscreen tests and animation if entity list changes.
        //
        // This is to prevent glitches/unnecessary processing.
        // Animation frame skips that occur if entity turns into a
        // particle should not be noticeable

        ldy.w   changeList
        bne     ChangeEntityList

        Camera.BranchEntityOutsideInactiveWindow(Deactivate)
        bra     StillActive

        Deactivate:
            // deactivate entity
            jsr     MetaSprite.Deactivate

            ldy.w   currentList
            sta.b   BaseEntity.functionTable

            ldy.w   #lists.deactivated
            bra     ChangeEntityList

        StillActive:
            jsr     MetaSprite.Animation.Process
    _NEXT()

    rts
}

// Render a list of entities
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: Y - address of list
// MODIFIES: DP
a16()
i16()
code()
scope ProcessEntityList_DeleteOffscreen: {
    _LOOP_Y()
        ldx.b   BaseEntity.functionTable
        jsr     (BaseEntityFunctionTable.Process,x)

        // Skip Offscreen tests and animation if entity list changes.
        //
        // This is to prevent glitches/unnecessary processing.
        // Animation frame skips that occur if entity turns into a
        // particle should not be noticeable

        ldy.w   changeList
        bne     ChangeEntityList

        Camera.BranchEntityOutsideInactiveWindow(Delete)
        bra     StillActive

        Delete:
            ldy.w   #lists.toDelete
            bra     ChangeEntityList

        StillActive:
            jsr     MetaSprite.Animation.Process
    _NEXT()

    rts
}

// Process an enemy list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessEnemyList(list, doDeactivateTest) {
    assert16a()
    assert16i()

    scope {list}: {
        _LOOP(lists.{list})
            ldx.b   BaseEntity.functionTable
            jsr     (BaseEntityFunctionTable.Process,x)

            // Skip collision/offscreen tests if entity list changes
            //
            // Still process the animation as the most common list
            // change is to the `npc` or `particle` list so the death
            // animation can be processed.

            ldy.w   changeList
            bne     SkipCollision

            if {doDeactivateTest} == 1 {
                Camera.BranchEntityOutsideInactiveWindow(Deactivate)
                bra     StillActive

                Deactivate:
                    jsr     MetaSprite.Deactivate

                    ldy.w   #lists.{list}
                    sta.b   BaseEntity.functionTable

                    ldy.w   #lists.deactivated
                    bra     ChangeEntityList

                StillActive:
            }

            // ::TODO do collisions with playerProjectiles

        SkipCollision:
            jsr     MetaSprite.Animation.Process
        _NEXT()
    }
}

// Process player list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessPlayerList() {
    assert16a()
    assert16i()

    scope players: {
        _LOOP(lists.players)
            ldx.b   BaseEntity.functionTable
            jsr     (BaseEntityFunctionTable.Process,x)

            // skip collisions/animations if entity list changes
            //
            // Still process the animation as the most common list
            // change is to the `particle` list so the death animation
            // can be processed.

            ldy.w   changeList
            bne     SkipCollisions

                // ::TODO collisions with enemies
                // ::TODO collisions with bosses
                // ::TODO collisions with enemyProjectiles

        SkipCollisions:
            jsr     MetaSprite.Animation.Process
        _NEXT()
    }
}


// Process the particle list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessParticleList() {
    assert16a()
    assert16i()

    scope particles: {
        _LOOP(lists.particles)
            ldx.b   BaseEntity.functionTable
            jsr     (BaseEntityFunctionTable.Process,x)

            // skip animation if entity list changes
            ldy.w   changeList
            bne     ChangeEntityList

            jsr     MetaSprite.Animation.Process
        _NEXT()
    }
}

// Process the to Delete list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessToDeleteList() {
    scope ToDelete: {
        lda.w   lists.toDelete
        beq     NoneToDelete

        Loop:
            tcd

            MetaSprite.EntityDestructor()

            ldx.b   BaseEntity.functionTable
            jsr     (BaseEntityFunctionTable.Destructor,x)

            lda.b   BaseEntity.next
            bne     Loop


        // Move the toDelete list into the free list

        // toDelete.last->next = lists.free
        // lists.free = lists.toDelete
        // lists.toDelete = NULL

        lda.w   lists.free
        sta.b   BaseEntity.next

        lda.w   lists.toDelete
        sta.w   lists.free

        stz.w   lists.toDelete

    NoneToDelete:
    }
}


// Try to reactivate an entity that could not be
// activated in the previous frame
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
a16()
i16()
code()
macro _ProcessTryToActivateAgainList() {
    assert16a()
    assert16i()

    scope TryToActivateAgain: {
        lda.w   lists.tryToActivateAgain
        beq     EndLoop

        sta.w   previousEntity

        Loop:
            tcd

            jsr     MetaSprite.Activate
            bcc     Continue

            // MetaSprite.Activate was successful, move to correct list

            // list = BaseEntity.functionTable
            // previousEntity->next = dp->next
            // dp->next = *list
            // *list = this
            // next = previousEntity->next

            ldx.w   previousEntity

            lda.b   BaseEntity.next
            sta.w   BaseEntity.next,x

            ldy.b   BaseEntity.functionTable
            lda.w   0,y
            sta.b   BaseEntity.next

            tdc
            sta.w   0,y

            ldy.w   BaseEntity.next,x

            // restore entity functionPtr
            ldx.b   BaseEntity.romPtr
            lda.l   BaseEntityData.functionTable,x
            sta.b   BaseEntity.functionTable

            tya
            bne     Loop
            bra     EndLoop

        Continue:
            // could not activate, just go to next entity
            tdc
            sta.w   previousEntity
            lda.b   BaseEntity.next

            bne     Loop

    EndLoop:
    }
}


// Process the entity gameloop
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: Y - address of list
// MODIFIES: DP
a16()
i16()
code()
scope ProcessGameLoop: {
    // ::DEBUG::
    // ::TODO move into Camera Game Loop::
    Camera._UpdateWindows()

    _ProcessTryToActivateAgainList()

    ldy.w   #lists.playerProjectiles
    jsr     ProcessEntityList_DeleteOffscreen

    ldy.w   #lists.enemyProjectiles
    jsr     ProcessEntityList_DeleteOffscreen

    ldy.w   #lists.platforms
    jsr     ProcessEntityList_DeactivateOffscreen

    _ProcessEnemyList(enemies, 1)
    _ProcessEnemyList(bosses, 0)

    _ProcessPlayerList()

    ldy.w   #lists.npcs
    jsr     ProcessEntityList_DeactivateOffscreen

    _ProcessParticleList()


    _ProcessToDeleteList()

    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

