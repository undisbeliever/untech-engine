// entity/gameloop.inc
// ===================
//
// Entity Game-Loop routines.
//
// The Game-Loop exists as two macros, `ProcessSpecialLists` and
// `ProcessEntityList`, both of which need to be invoked by the
// subproject.
//
// The design of the entity loop and the `@EndEntityProcess` routines
// were inspired by cosmicchipsocket's Superfeather SNES Game Engine[1].
//
// [1]: https://gitlab.com/cosmicchipsocket/superfeather
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


RegisterBreakId(NO_FUNCTION_POINTER)
RegisterBreakId(INVALID_PROCESS_ENTITY_RETURN)
RegisterBreakId(STACK_SMASHED)


namespace Entity {


// Previous entity being processed
// Used in _ProcessActivateNextFrameList, _ProcessActiveWindow and __ProcessEntityList
// (word address)
allocateTmpWord(_previousEntity)


// Move the current entity to a given special list.
//
// ASSUMES: The system is in the middle of an entity loop.
// ASSUMES: The given list if not the currently looping list
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// PARAM: the special list to move the entity into
//
// INPUT: DP = The current entity being processed
//        _previousEntity = The address of the previous entity.
//                          If there is no previous entity, then
//                          `_previousEntity` is the address of the
//                          current entity list.
// OUTPUT: A = The address of the next entity of the original list
//         z set if the current entity has no next entity
//               (ie, there is no more entities in the current list)
//
// NOTE: You MUST NOT set the `_previousEntity` variable after invoking
// this macro.
macro __MoveEntityToSpecialList(list) {
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

    lda.w   Entity.specialLists.{list}
    sta.b   BaseEntity.next

    tdc
    sta.w   Entity.specialLists.{list}

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
// NOTE: You MUST NOT change the `_previousEntity` variable after invoking
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
a16()
i16()
code()
function _ProcessActivateNextFrameList {

    lda.w   Entity.specialLists.activateNextFrame
    beq     Return

    // this allows _previousEntity variable to work on first item
    assert(BaseEntity.next == 0)

    ldy.w   #Entity.specialLists.activateNextFrame
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

            rts


        InvalidEntityListId:
            // This should not happen
            break(INVALID_ENTITY_LIST_ID)


        ActivatedSuccessfully:
            // Entity is now Active
            //  move entity to current list

            lda.b   BaseEntity.listId
            and.w   #0xff
            cmp.w   #Entity.N_ENTITY_LISTS
            bcs     InvalidEntityListId

            asl
            tay

            Entity.__ChangeEntityList_Y()
            // A = next entity address
            // z set if there are no more entities in the current list
            // MUST NOT CHANGE _previousEntity as the entity is no longer in the activateNextFrame list

            bne     Loop

Return:
    rts
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


        lda.w   Entity.specialLists.deactivated
        beq     EndLoop

        ldy.w   #Entity.specialLists.deactivated
        sty.w   Entity._previousEntity

        Loop:
            tcd

            Entity.BranchEntityOutsideActiveWindow(Continue)

                // entity is now inside active window
                // try to activate it
                jsr     MetaSprite.Activate
                bcs     ActivatedSuccessfully

                    // Could not activate the entity.
                    // Try again on the next frame.
                    ldy.w   #Entity.specialLists.ACTIVATE_NEXT_FRAME_INDEX
                    bra     _ChangeEntityList_Y


                InvalidEntityListId:
                    // This should not happen
                    break(INVALID_ENTITY_LIST_ID)


                ActivatedSuccessfully:
                    // Entity is now Active
                    //  move entity to current list

                    lda.b   BaseEntity.listId
                    and.w   #0xff
                    cmp.w   #Entity.N_ENTITY_LISTS
                    bcs     InvalidEntityListId

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
// NOTE: This routine MUST NOT be called on an empty entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A - address of the first entity in the list (NOT NULL)
//        X - address of the entity list
function __ProcessEntityList {

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

        // Get the Process routine that failed to make it easier to find the invalid entity.
        ldx.b   BaseEntity.functionTable
        lda.l   BaseEntityFunctionTable.Process,x

    InvalidProcessEntityReturn:
        // X = entity function table
        // A = entity process routine
        break(INVALID_PROCESS_ENTITY_RETURN)

NoFunctionPointer:
    break(NO_FUNCTION_POINTER)
}

// Goto and process to the next entity.
//
// NOTE: If the entity is moved into a different list (ie, by using
// `__ChangeEntityList_Y`) then you MUST NOT change `_previousEntity`
// AND IMMEDIATELY ENTER this routine via the
// `GotoNextEntity.EntityListChanged` label.
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = current entity
//
// RETURN: invokes RTS or BRK
// MODIFIES: DP to next entity in list or 0 if no more entities in list
inline __GotoNextEntity() {
    assert16a()
    assert16i()

    tdc
    sta.w   _previousEntity

    lda.b   BaseEntity.next

// A = next entity
// z set if there are no more entities in the list
EntityListChanged:
    beq     NoMoreEntitiesInList

        tcd
        ldx.b   BaseEntity.functionTable
        beq     NoFunctionPointer
        jmp     (BaseEntityFunctionTable.Process,x)


NoMoreEntitiesInList:
    // The last entity has been processed.
    // Verify the stack is unchanged.
    // This should never happen, but I decided to test for it anyway.

    plx
    cpx.w   #__ProcessEntityList.AfterJSR - 1
    bne     StackSmashed

    // reset DP
    lda.w   #0
    tcd
// DP = 0

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
//  * After Processing entity lists
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro ProcessSpecialLists() {
    assert16a()
    assert16i()

    // ::TODO move into Camera Module::
    jsr     Entity._UpdateActiveWindows

    jsr     Entity._ProcessActivateNextFrameList

    Entity._ProcessActiveWindow()

    lda.w   #0
    tcd
}



// Process an entity list.
//
// The macro will call the `BaseEntityFunctionTable.Process` routine on
// every entity in the given entity list.
//
// This macro should be invoked after the `MetaSprite.EntityHitbox` tests.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// PARAM: listName - name of entity list to process
a16()
i16()
code()
macro ProcessEntityList(listName) {
    assert16a()
    assert16i()

    lda.w   {Entity.listAddress({listName})}
    beq     SkipList{#}

        ldx.w   #{Entity.listAddress({listName})}
        jsr     Entity.__ProcessEntityList

SkipList{#}:
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
    //  set A/z to _previousEntity->next
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
        sta.w   specialLists.free

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

    // If you change this section, also change `TemporarilyDeactivateEntities`
    jsr     MetaSprite.Deactivate

    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.Deactivated,x)


    __MoveEntityToSpecialList(deactivated)
    // MUST JUMP directly to `GotoNextEntity.EntityListChanged` IMMEDIATELY AFTER a __MoveEntityToSpecialList() call

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
// INTERNAL NOTE: If the entity is moved into a different list (ie, by
// using `__ChangeEntityList_Y`) then you MUST NOT change
// `_previousEntity` AND IMMEDIATELY ENTER this routine via the
// `EntityListChanged` label.
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

    cmp.b   #N_ENTITY_LISTS
    bcs     InvalidEntityListId

    sta.b   BaseEntity.listId

    assert(N_ENTITY_LISTS < 128)
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

InvalidEntityListId:
    break(INVALID_ENTITY_LIST_ID)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

