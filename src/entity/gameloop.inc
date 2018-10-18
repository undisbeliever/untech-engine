// entity/gameloop.inc
// ===================
//
// Entity Game-Loop.
//
// The GameLoop will:
//   * Process all active entities' Process routine
//   * Entity Collision detection
//   * Animate all active entities
//   * Activate entities that enter the active window
//   * Activate entities in the `Entity.tryToActivateAgain` list
//   * Delete all entities in the `Entity.toDelete` list
//
//
// The design of the entity loop and the `@EndEntityProcess` routines
// were inspired by cosmicchipsocket's Superfeather SNES Game Engine[1].
//
// [1]: https://gitlab.com/cosmicchipsocket/superfeather
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(NO_FUNCTION_POINTER)
RegisterBreakId(INVALID_PROCESS_ENTITY_RETURN)
RegisterBreakId(STACK_SMASHED)


namespace Entity {


// Previous entity being processed
// Used in TryToActivateAgain, ProcessActiveWindow and the entity loop.
// (word address)
allocateTmpWord(_previousEntity)


// Change the list of the current entity.
//
// ASSUMES: The system is in the middle of an entity loop.
// ASSUMES: The given list if not the currently looping list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// PARAM: the entity list to move the entity to
//
// INPUT: DP = The current entity being processed
//        _previousEntity = The address of the previous entity.
//                          If there is no previous entity, then
//                          `_previousEntity` is the address of the
//                          current entity list.
// OUTPUT: A = The address of the next entity of the original entity
//         z set if the current entity has no next entity
//               (ie, there is no more entities in the current list)
//
// NOTE: You MUST NOT set the `_previousEntity` variable after invoking
// this macro.
macro __ChangeEntityList(list) {
    assert16a()
    assert16i()

    // This allows me to skip the _previousEntity test.
    // (As long as _previousEntity is the entity list address when there is
    // this entity is the first one in the list)
    assert(BaseEntity.next == 0)

    // _previousEntity->next = dp->next
    // dp->next = *newList
    // *newList = this
    //
    // return _previousEntity->next

    ldx.w   _previousEntity

    lda.b   BaseEntity.next
    sta.w   BaseEntity.next,x
    tax

    lda.w   Entity.lists.{list}
    sta.b   BaseEntity.next

    tdc
    sta.w   Entity.lists.{list}

    txa
}


// Change the list of the current entity.
//
// ASSUMES: The system is in the middle of an entity loop.
// ASSUMES: The given list if not the currently looping list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: DP = The current entity being processed
//        Y = list Index (listId * 2, MUST NOT BE THE CURRENT LIST INDEX)
//        _previousEntity = The address of the previous entity.
//                          If there is no previous entity, then
//                          `_previousEntity` is the address of the
//                          current entity list.
// OUTPUT: A = The address of the next entity of the original entity
//         z set if the current entity has no next entity
//               (ie, there is no more entities in the current list)
//
// NOTE: You MUST NOT set the `_previousEntity` variable after invoking
// this macro.
macro __ChangeEntityList_Y() {
    assert16a()
    assert16i()

    assert(BaseEntity.next == 0)

    // _previousEntity->next = dp->next
    // dp->next = lists[listId]
    // lists[listId] = this
    //
    // return _previousEntity->next

    ldx.w   _previousEntity

    lda.b   BaseEntity.next
    sta.w   BaseEntity.next,x
    tax

    lda.w   lists.FIRST,y
    sta.b   BaseEntity.next

    tdc
    sta.w   lists.FIRST,y

    txa
}


// Processes the tryToActivateAgain entity list
//
// For each entity in the list it will try to reactivate the entity and
// if it succeeds move it into its designated entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
inline _ProcessTryToActivateAgainList() {
    assert16a()
    assert16i()

    function ProcessTryToActivateAgainList {
        lda.w   lists.tryToActivateAgain
        beq     EndLoop

        // this allows _previousEntity variable to work on first item
        assert(BaseEntity.next == 0)

        ldy.w   #lists.tryToActivateAgain
        sty.w   _previousEntity

        Loop:
            tcd

            jsr     MetaSprite.Activate
            bcs     ActivatedSuccessfully

                // Entity is still not active
                // goto next entity

                tdc
                sta.w   _previousEntity
                lda.b   BaseEntity.next

                bne     Loop
                bra     EndLoop


            InvalidEntityId:
                // This should not happen
                break(INVALID_ENTITY_LIST_ID)


            ActivatedSuccessfully:
                // Entity is now Active
                //  move entity to current list

                lda.b   BaseEntity.listId
                and.w   #0xff
                cmp.w   #lists.N_LISTS
                bcs     InvalidEntityId

                asl
                tay

                __ChangeEntityList_Y()
                // A = next entity address
                // z set if there are no more entities in the current list
                // MUST NOT CHANGE _previousEntity as the entity is no longer in the tryToActivateAgain list

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
inline _ProcessActiveWindow() {
    assert16a()
    assert16i()

    function ProcessActiveWindow {
        lda.w   Camera.xPos
        and.w   #TEST_ACTIVE_WINDOW_MASK
        tax

        lda.w   Camera.yPos
        and.w   #TEST_ACTIVE_WINDOW_MASK

        cpx.w   prevActiveWindowX
        bne     XChanged

        cmp.w   prevActiveWindowY
        beq     SkipTests

        XChanged:
            stx.w   prevActiveWindowX
            sta.w   prevActiveWindowY


            lda.w   lists.deactivated
            beq     EndLoop

            ldy.w   #lists.deactivated
            sty.w   _previousEntity

            Loop:
                tcd

                BranchEntityOutsideActiveWindow(Continue)

                    // entity is now inside active window
                    // try to activate it
                    jsr     MetaSprite.Activate
                    bcs     ActivatedSuccessfully

                        // Could not activate the entity.
                        // Try again in the next frame.
                        ldy.w   #lists._TRY_TO_ACTIVATE_AGAIN_INDEX
                        bra     _ChangeEntityList_Y


                    InvalidEntityId:
                        // This should not happen
                        break(INVALID_ENTITY_LIST_ID)


                    ActivatedSuccessfully:
                        // Entity is now Active
                        //  move entity to current list

                        lda.b   BaseEntity.listId
                        and.w   #0xff
                        cmp.w   #lists.N_LISTS
                        bcs     InvalidEntityId

                        asl
                        tay

                    _ChangeEntityList_Y:
                        __ChangeEntityList_Y()
                        // A = next entity address
                        // z set if there are no more entities in the current list
                        // MUST NOT CHANGE _previousEntity as the entity is no longer in the deactivated list

                        bne     Loop
                        bra     EndLoop


            Continue:
                // goto next entity

                tdc
                sta.w   _previousEntity
                lda.b   BaseEntity.next

                bne     Loop

    EndLoop:
    SkipTests:
    }
}


// Process an entity list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// PARAM: list: the name of the entity list to process
//
// MODIFIES: DP
macro _ProcessEntityList(list) {
    assert16a()
    assert16i()

    lda.w   Entity.lists.{list}
    beq     _{list}_Empty
        ldy.w   #Entity.lists.{list}
        jsr     __ProcessEntityList

_{list}_Empty:
}

// Process an entity list.
//
// ASSUMES: The list is not empty
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: A = address of the first entity (NOT NULL)
//        Y = address of the entity list (NOT NULL)
//
// MODIFIES: DP
function __ProcessEntityList {

    sty.w   _previousEntity

    tcd
    ldx.b   BaseEntity.functionTable
    beq     NoFunctionPointer
    jsr     (BaseEntityFunctionTable.Process,x)
AfterJSR:

    // Using JSR above so the engine can panic when an entity's Process
    // routine exits using an RTS instruction.

    // Get Process routine that failed, make it easier to find the invalid entity.
    ldx.b   BaseEntity.functionTable
    lda.l   BaseEntityFunctionTable.Process,x

    // X = entity function table
    // A = entity process routine

    break(INVALID_PROCESS_ENTITY_RETURN)

NoFunctionPointer:
    break(NO_FUNCTION_POINTER)
}

// Goto and process to the next entity.
//
// NOTE: If the entity list has changed by using `__ChangeEntityList` or
// `__ChangeEntityList_Y` then you MUST IMMEDIATELY ENTER this routine
// via the `GotoNextEntity.EntityListChanged` label.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: invokes RTS or BRK
inline __GotoNextEntity() {
    assert16a()
    assert16i()

    tdc
    sta.w   _previousEntity

    lda.b   BaseEntity.next

// A = next entity
// z = next entity exists
EntityListChanged:
    beq     NoMoreEntities

        tcd
        ldx.b   BaseEntity.functionTable
        beq     NoFunctionPointer
        jmp     (BaseEntityFunctionTable.Process,x)


NoMoreEntities:
    // Verify the stack is unchanged.
    // This should never happen, but I decided to test for it anyway.

    plx
    cpx.w   #__ProcessEntityList.AfterJSR - 1
    bne     StackSmashed

    rts


// Should replace with in DeleteAndGotoNextEntity release build
NoFunctionPointer:
    break(NO_FUNCTION_POINTER)

StackSmashed:
    break(STACK_SMASHED)
}


// PUBLIC API
// ==========

// Process the entity gameloop
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: Y - address of list
a16()
i16()
code()
function ProcessGameLoop {
    jsr     _UpdateActiveWindows

    _ProcessTryToActivateAgainList()

    _ProcessActiveWindow()

    lda.w   #0
    tcd

    MetaSprite.EntityHitbox.Begin()
        MetaSprite.EntityHitbox.TestCollisions(playerProjectiles, enemies, bosses)
        MetaSprite.EntityHitbox.TestCollisions(players, enemyProjectiles, enemies, bosses)
    MetaSprite.EntityHitbox.End()


    _ProcessEntityList(playerProjectiles)
    _ProcessEntityList(enemyProjectiles)

    _ProcessEntityList(platforms)
    _ProcessEntityList(enemies)
    _ProcessEntityList(bosses)
    _ProcessEntityList(npcs)

    _ProcessEntityList(players)

    _ProcessEntityList(particles)


    lda.w   #0
    tcd

    rts
}



// Delete the current entity and then process the next entity.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: GotoNextEntity
//
// @EndEntityProcess
a16()
i16()
code()
function DeleteAndGotoNextEntity {

    jsr     __EntityDestructor

    __ChangeEntityList(free)
    // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER a __ChangeEntityList() call
    bra     GotoNextEntity.EntityListChanged
}


// Change the entity's list and then process the next entity.
//
// REQUITES: DB = 0x7e
// INPUT: dp = current entity
//         A = entity List Id
//
// RETURN: GotoNextEntity
//
// @EndEntityProcess
au()
iu()
code()
function ChangeEntityListIdAndGotoNextEntity {
    sep     #$30
a8()
i8()
    cmp.b   BaseEntity.listId
    beq     ListIdUnchanged

    cmp.b   #lists.N_LISTS
    bcs     InvalidEntityId

    sta.b   BaseEntity.listId

    assert(lists.N_LISTS < 128)
    asl
    tay

    rep     #$30
a16()
i16()
    __ChangeEntityList_Y()
    // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER a __ChangeEntityList_Y() call

    bra     GotoNextEntity.EntityListChanged

a8()
i8()
ListIdUnchanged:
    rep     #$30
a16()
i16()
    bra     GotoNextEntity

InvalidEntityId:
    break(INVALID_ENTITY_LIST_ID)
}


// Deactivate the current entity and then process the next entity.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: GotoNextEntity
//
// @EndEntityProcess
a16()
i16()
code()
function DeactivateAndGotoNextEntity {

    jsr     MetaSprite.Deactivate

    __ChangeEntityList(deactivated)
    // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER a __ChangeEntityList() call

    bra     GotoNextEntity.EntityListChanged
}



// Test if the current entity is outside the inactive window.
// If it is delete it, otherwise call GotoNextEntity.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: DeleteAndGotoNextEntity or DrawAndGotoNextEntity
//
// @EndEntityProcess
a16()
i16()
code()
function DeleteIfOutsideAndGotoNextEntity {
    BranchEntityOutsideInactiveWindow(DeleteAndGotoNextEntity)

    bra     GotoNextEntity
}



// Test if the current entity is outside the inactive window.
// If it is deactivate it, otherwise call GotoNextEntity.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: DeactivateAndGotoNextEntity or DrawAndGotoNextEntity
//
// @EndEntityProcess
a16()
i16()
code()
function DeactivateIfOutsideAndGotoNextEntity {
    BranchEntityOutsideInactiveWindow(DeactivateAndGotoNextEntity)

// GotoNextEntity
FallThrough:
}



// Goto and process the next entity in the list.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// INTERNAL NOTE: If the entity list has changed by using
// `__ChangeEntityList` or `__ChangeEntityList_Y` then you MUST
// IMMEDIATELY ENTER this routine via the `EntityListChanged` label.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: next entity's Process routine or exits the entity loop
//
// @EndEntityProcess
a16()
i16()
code()
function GotoNextEntity {
    assert(pc() == DeactivateIfOutsideAndGotoNextEntity.FallThrough)

    // Keep old behaviour (for now)
    jsr     MetaSprite.Animation.Process

    __GotoNextEntity()
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

