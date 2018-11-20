// entity/gameloop.inc
// ===================
//
// Entity Game-Loop routines.
//
// The Game-Loop exists as two macros, `ProcessSpecialLists` and
// `ProcessEntityLists`, both of which need to be invoked by the
// subproject.
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
// Used in _ProcessActivateNextFrameList, _ProcessActiveWindow and __ProcessEntityLists
// (word address)
allocateTmpWord(_previousEntity)


// The current entity list being processed
// Used in __ProcessEntityLists
// (word address)
allocateTmpWord(_currentEntityList)


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

    ldx.w   Entity._previousEntity

    lda.b   BaseEntity.next
    sta.w   BaseEntity.next,x
    tax

    lda.w   Entity.lists.FIRST,y
    sta.b   BaseEntity.next

    tdc
    sta.w   Entity.lists.FIRST,y

    txa
}


// Processes the activateNextFrame entity list
//
// For each entity in the list it will try to reactivate the entity and
// if it succeeds move it into its designated entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
macro _ProcessActivateNextFrameList() {
    assert16a()
    assert16i()

    lda.w   Entity.lists.activateNextFrame
    beq     EndLoop

    // this allows _previousEntity variable to work on first item
    assert(BaseEntity.next == 0)

    ldy.w   #Entity.lists.activateNextFrame
    sty.w   Entity._previousEntity

    Loop:
        tcd

        jsr     MetaSprite.Activate
        bcs     ActivatedSuccessfully

            // Entity is still not active
            // goto next entity

            tdc
            sta.w   Entity._previousEntity
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
            cmp.w   #Entity.lists.N_LISTS
            bcs     InvalidEntityId

            asl
            tay

            Entity.__ChangeEntityList_Y()
            // A = next entity address
            // z set if there are no more entities in the current list
            // MUST NOT CHANGE _previousEntity as the entity is no longer in the activateNextFrame list

            bne     Loop

EndLoop:
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

    lda.w   Camera.xPos
    and.w   #Entity.TEST_ACTIVE_WINDOW_MASK
    tax

    lda.w   Camera.yPos
    and.w   #Entity.TEST_ACTIVE_WINDOW_MASK

    cpx.w   Entity.prevActiveWindowX
    bne     XChanged

    cmp.w   Entity.prevActiveWindowY
    beq     SkipTests

    XChanged:
        stx.w   Entity.prevActiveWindowX
        sta.w   Entity.prevActiveWindowY


        lda.w   Entity.lists.deactivated
        beq     EndLoop

        ldy.w   #Entity.lists.deactivated
        sty.w   Entity._previousEntity

        Loop:
            tcd

            Entity.BranchEntityOutsideActiveWindow(Continue)

                // entity is now inside active window
                // try to activate it
                jsr     MetaSprite.Activate
                bcs     ActivatedSuccessfully

                    // Could not activate the entity.
                    // Try again in the next frame.
                    ldy.w   #Entity.lists._ACTIVATE_NEXT_FRAME_INDEX
                    bra     _ChangeEntityList_Y


                InvalidEntityId:
                    // This should not happen
                    break(INVALID_ENTITY_LIST_ID)


                ActivatedSuccessfully:
                    // Entity is now Active
                    //  move entity to current list

                    lda.b   BaseEntity.listId
                    and.w   #0xff
                    cmp.w   #Entity.lists.N_LISTS
                    bcs     InvalidEntityId

                    asl
                    tay

                _ChangeEntityList_Y:
                    Entity.__ChangeEntityList_Y()
                    // A = next entity address
                    // z set if there are no more entities in the current list
                    // MUST NOT CHANGE _previousEntity as the entity is no longer in the deactivated list

                    bne     Loop
                    bra     EndLoop


        Continue:
            // goto next entity

            tdc
            sta.w   Entity._previousEntity
            lda.b   BaseEntity.next

            bne     Loop

EndLoop:
SkipTests:
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
        jsr     Entity.__ProcessEntityList

_{list}_Empty:
}

// Processes the entity lists.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// MODIFIES: DP
function __ProcessEntityLists {

    ldx.w   #lists.FIRST

    Repeat:
        lda.w   0,x
        beq     ListEmpty
            stx.w   _currentEntityList
            stx.w   _previousEntity

            tcd
            ldx.b   BaseEntity.functionTable
            beq     NoFunctionPointer
            jsr     (BaseEntityFunctionTable.Process,x)

            AfterJSR:
                // The entity process routine should eventually jump to
                // `Entity.GotoNextEntity` and therefore we should not
                // be here.

                // I am using JSR above so the engine can panic if an entity's Process
                // routine exits using an RTS instruction.

                // Get Process routine that failed, to make it easier to find the invalid entity.
                ldx.b   BaseEntity.functionTable
                lda.l   BaseEntityFunctionTable.Process,x

                // X = entity function table
                // A = entity process routine

                break(INVALID_PROCESS_ENTITY_RETURN)

    ListEmpty:
        inx
        inx
        cpx.w   #lists.LAST + 2
        bcc     Repeat

    // There are no entities
    rts

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
// z set if there are no more entities in the list
EntityListChanged:
    beq     NoMoreEntitiesInCurrentList

        tcd
        ldx.b   BaseEntity.functionTable
        beq     NoFunctionPointer
        jmp     (BaseEntityFunctionTable.Process,x)


NoMoreEntitiesInCurrentList:
    // Process the next entity list
    ldx.w   _currentEntityList
    Repeat:
        cpx.w   #lists.LAST
        bcs     NoMoreLists

        inx
        inx

        lda.w   0,x
        beq     Repeat

    // X = next list that is not empty
    // A = first entity in list
    stx.w   _currentEntityList
    stx.w   _previousEntity

    tcd
    ldx.b   BaseEntity.functionTable
    beq     NoFunctionPointer
    jmp     (BaseEntityFunctionTable.Process,x)


NoMoreLists:
    // The last entity has been processed.
    // Verify the stack is unchanged.
    // This should never happen, but I decided to test for it anyway.

    plx
    cpx.w   #__ProcessEntityLists.AfterJSR - 1
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


// Process the special entity lists.
//
// This macro:
//   * Activates entities in the `Entity.activateNextFrame` list
//   * Activates entities that enter the active window
//
// This macro should be invoked either:
//  * Before Processing entity hitboxes, or
//  * After `ProcessEntityLists`
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro ProcessSpecialLists() {
    assert16a()
    assert16i()

    // ::TODO move into Camera Module::
    jsr     Entity._UpdateActiveWindows

    Entity._ProcessActivateNextFrameList()

    Entity._ProcessActiveWindow()

    lda.w   #0
    tcd
}

// Process the entity lists.
//
// The macro will call the `BaseEntityFunctionTable.Process` routine on
// every entity in all entity lists.
//
// This macro should be invoked after the `MetaSprite.EntityHitbox` tests.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
macro ProcessEntityLists() {
    assert16a()
    assert16i()

    // ::TODO move into Camera Module::
    jsr     Entity._UpdateActiveWindows

    jsr     Entity.__ProcessEntityLists

    lda.w   #0
    tcd
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


    // Move entity to end of free list

    //  _previousEntity->next = dp->next
    //  dp->next = 0
    //
    //  if lastFreeEntity is not null:
    //      lastFreeEntity->next = dp
    //  else:
    //      lists[FREE] = dp
    //
    //  lastFreeEntity = dp
    //
    //  set A/n to _previousEntity->next
    //  Goto GotoNextEntity.EntityListChanged


    lda.b   BaseEntity.next
    ldx.w   _previousEntity
    sta.w   BaseEntity.next,x
    tax

    stz.b   BaseEntity.next

    tdc
    ldy.w   lastFreeEntity
    sta.w   lastFreeEntity              // n unchanged

    beq     FreeListEmpty
        sta.w   BaseEntity.next,y

        txa

        // A = next entity
        // z set if there are no more entities in the list

        // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER changing entity list
        jmp     GotoNextEntity.EntityListChanged


    FreeListEmpty:
        sta.w   lists.free

        txa

        // A = next entity
        // z set if there are no more entities in the list

        // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER changing entity list
        jmp     GotoNextEntity.EntityListChanged
}



// Test if the current entity is outside the inactive window.
// If it is delete it, otherwise jump to GotoNextEntity.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: DeleteAndGotoNextEntity or GotoNextEntity
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
// If it is delete it, otherwise jump to DrawAndGotoNextEntity.
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
function DeleteIfOutsideOrDrawAndGotoNextEntity {
    BranchEntityOutsideInactiveWindow(DeleteAndGotoNextEntity)

    bra     DrawAndGotoNextEntity
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

    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.Deactivated,x)

    __ChangeEntityList(deactivated)
    // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER a __ChangeEntityList() call

    bra     GotoNextEntity.EntityListChanged
}



// Test if the current entity is outside the inactive window.
// If it is deactivate it, otherwise jump to GotoNextEntity.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: DeactivateAndGotoNextEntity or GotoNextEntity
//
// @EndEntityProcess
a16()
i16()
code()
function DeactivateIfOutsideAndGotoNextEntity {
    BranchEntityOutsideInactiveWindow(DeactivateAndGotoNextEntity)

    bra     GotoNextEntity
}



// Test if the current entity is outside the inactive window.
// If it is deactivate it, otherwise jump to DrawAndGotoNextEntity.
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
function DeactivateIfOutsideOrDrawAndGotoNextEntity {
    BranchEntityOutsideInactiveWindow(DeactivateAndGotoNextEntity)

// DrawAndGotoNextEntity
FallThrough:
}



// Process MetaSprite Animations, draws Entity MetaSprites,
// then goto and process the next entity in the list.
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
function DrawAndGotoNextEntity {
    assert(pc() == DeactivateIfOutsideOrDrawAndGotoNextEntity.FallThrough)

    jsr     MetaSprite.Animation.Process

AfterAnimationProcessed:
    jsr     MetaSprite.Render.RenderEntity

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
    assert(pc() == DrawAndGotoNextEntity.FallThrough)

    __GotoNextEntity()
}



// Process MetaSprite Animations, deleting the entity if the animation ends,
// otherwise draw Entity MetaSprites and process the next entity in the list.
//
// MUST ONLY BE CALLED in the Entity's Process routine
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: DrawAndGotoNextEntity.AfterAnimationProcessed
// RETURN: DeleteAndGotoNextEntity
//
// @EndEntityProcess
a16()
i16()
code()
function DeleteIfAnimationEndsOrDrawAndGotoNextEntity {

    jsr     MetaSprite.Animation.Process

    // If animation.id is < 0xff then the animation is still going

    lda.b   BaseEntity.MetaSprite.Animation.id - 1
    cmp.w   #0xff00
    bcc     DrawAndGotoNextEntity.AfterAnimationProcessed

    jmp     DeleteAndGotoNextEntity
}



// Processes the MetaSprite Animation, draws entity MetaSprites,
// change the entity's list and then process the next entity.
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
function DrawAndChangeEntityListIdAndGotoNextEntity {
    rep     #$30
a16()
i16()
    pha

    jsr     MetaSprite.Animation.Process
    jsr     MetaSprite.Render.RenderEntity

    pla

// ChangeEntityListIdAndGotoNextEntity
FallThrough:
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
    assert(pc() == DrawAndChangeEntityListIdAndGotoNextEntity.FallThrough)

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

}

// vim: ft=bass-65816 ts=4 sw=4 et:

