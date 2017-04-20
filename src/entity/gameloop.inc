
scope Entity {

// ENTITY ITERATION MACROS
// =======================

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
        Continue:
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

        ChangeEntityList_SkipTest:
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


// PROCESS ENTITY LISTS
// ====================

// Processes a list of entities with no collisions,
// entities that move outside the inactive window will be
// deactivated.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: Y - address of list (MUST NOT be the deactivated list)
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
            sta.b   BaseEntity._ft_currentList

            ldy.w   #lists.deactivated
            bra     ChangeEntityList_SkipTest

        StillActive:
            jsr     MetaSprite.Animation.Process
    _NEXT()

    rts
}

// Processes a list of entities with no collisions,
// entities that move outside the inactive window will be
// deleted.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: Y - address of list (MUST NOT be the toDelete list)
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
            bra     ChangeEntityList_SkipTest

        StillActive:
            jsr     MetaSprite.Animation.Process
    _NEXT()

    rts
}


// Process the enemy/boss list.
//
// For each entity in the list:
//   - Executes entity's Process routine
//   - Deactivates the entity if moved outside the active window.
//   - Tests for collisions between entity and player projectiles.
//
// PARAM: list - the name of the list to execute
// PARAM: doDeactivateTest - if true then the entity
//        is deactivated if outside the inactive window.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
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
                    sta.b   BaseEntity._ft_currentList

                    ldy.w   #lists.deactivated
                    bra     ChangeEntityList_SkipTest

                StillActive:
            }

            MetaSprite.EntityHitbox.ProcessList(playerProjectiles)

        SkipCollision:
            jsr     MetaSprite.Animation.Process
        _NEXT()
    }
}



// Process the player entity list.
//
// For each entity in the list:
//   - Executes entity's Process routine
//   - Tests for collisions between the entity and:
//      * bosses
//      * enemies
//      * enemy projectiles
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

                MetaSprite.EntityHitbox.ProcessList(bosses)
                MetaSprite.EntityHitbox.ProcessList(enemies)
                MetaSprite.EntityHitbox.ProcessList(enemyProjectiles)

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



// Processes the tryToActivateAgain entity list
//
// For each entity in the list it will try to reactivate the entity and
// if it succeeds move it into its designated entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessTryToActivateAgainList() {
    assert16a()
    assert16i()

    scope TryToActivateAgain: {
        lda.w   lists.tryToActivateAgain
        beq     EndLoop

        // this allows previousEntity variable to work on first item
        assert(BaseEntity.next == 0)

        ldy.w   #lists.tryToActivateAgain
        sty.w   previousEntity

        Loop:
            tcd

            jsr     MetaSprite.Activate
            bcc     Continue

            // MetaSprite.Activate was successful.
            // move to the designated list.

            // list = BaseEntity._ft_currentList
            // previousEntity->next = dp->next
            // dp->next = *list
            // *list = this
            // next = previousEntity->next

            ldx.w   previousEntity

            lda.b   BaseEntity.next
            sta.w   BaseEntity.next,x

            ldy.b   BaseEntity._ft_currentList
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
            // could not activate, goto next entity
            tdc
            sta.w   previousEntity
            lda.b   BaseEntity.next

            bne     Loop

    EndLoop:
    }
}



// Processes the deactivated list if the camera has moved enough.
//
// For each entity in the list it will check to see if the entity is
// inside the active window, and if so activate it and moved the entity
// into the appropriate list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessActiveWindow() {
    assert16a()
    assert16i()

    scope ProcessActiveWindow: {
        lda.w   Camera.xPos
        and.w   #TEST_ACTIVE_WINDOW_MASK
        cmp.w   prevActiveWindowX
        bne     XChanged
        tax

        lda.w   Camera.yPos
        and.w   #TEST_ACTIVE_WINDOW_MASK
        cmp.w   prevActiveWindowY
        bne     YChanged

        bra     SkipTests

            XChanged:
                tax
                lda.w   Camera.yPos
                and.w   #~3

            YChanged:
                stx.w   prevActiveWindowX
                sta.w   prevActiveWindowY


            _LOOP(lists.deactivated)
                Camera.BranchEntityOutsideActiveWindow(Continue)

                    // entity is now inside active window
                    // try to activate if possible
                    jsr     MetaSprite.Activate
                    bcc     CouldNotActivate

                    // Entity is Active
                    //  get list to move entity to
                    //  restore functionPtr
                    //  move entity to correct list

                    ldy.b   BaseEntity._ft_currentList

                    ldx.b   BaseEntity.romPtr
                    lda.l   BaseEntityData.functionTable,x
                    sta.b   BaseEntity.functionTable

                    bra     ChangeEntityList_SkipTest

            CouldNotActivate:
                    // try again in next frame

                    ldy.w   #lists.tryToActivateAgain
                    bra     ChangeEntityList_SkipTest
            _NEXT()
    SkipTests:
    }
}

// PUBLIC API
// ==========

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
    Camera._UpdateWindows()

    _ProcessTryToActivateAgainList()

    _ProcessActiveWindow()


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


    _DeleteAllInList(toDelete)

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

