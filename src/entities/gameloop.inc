
scope Entities {

// Previous entity being processed
// (word address)
allocateTmpWord(previousEntity)

macro _LOOP(list) {
    assert16a()
    assert16i()

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
            tdc
            sta.w   previousEntity
            lda.b   BaseEntity.next

            bne     Loop
            bra     EndLoop

        ChangeEntityList:
            // Change the list the entity is in THEN return
            // Y = list to change to (NOT NULL)

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

        // ::TODO offscreen test::
        bra     Else

        Deactivate:
            ldy.w   currentList
            sta.b   BaseEntity.functionTable

            ldy.w   #lists.deactivated
            bra     ChangeEntityList

        Else:
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

        // ::TODO offscreen test::
        bra     Else

        Delete:
            ldy.w   #lists.toDelete
            bra     ChangeEntityList

        Else:
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

            if {doDeactivateTest} == 1 {
                // ::TODO offscreen test::
                bra EndIf

                Deactivate:
                    ldy.w   #lists.{list}
                    sta.b   BaseEntity.functionTable

                    ldy.w   #lists.deactivated
                    bra     ChangeEntityList
                EndIf:
            }

            // ::TODO do collisions with playerProjectiles

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

            // ::TODO collisions with enemies
            // ::TODO collisions with bosses
            // ::TODO collisions with enemyProjectiles

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
            jsr     MetaSprite.Animation.Process
        _NEXT()
    }
}

// Process to Delete list
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

