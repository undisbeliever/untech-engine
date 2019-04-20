// entity/transitions.inc
// ======================
//
// Routines to assist in the transition between game-mode and menu-mode.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {


// Temporarily Deactivate all entities, moving them into the
// `activateNextFrame` special List.
//
// This routine is useful for when you wish to exit normal game mode
// and enter a menu mode.
//
// MUST NOT BE CALLED in the Entity GameLoop
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function TemporarilyDeactivateEntities {

    ldx.w   #0
    OuterLoop:
        lda.w   lists.FIRST,x
        beq     ListEmpty

            phx
            InnerLoop:
                tcd


                // The following MUST MATCH DeactivateAndGotoNextEntity
                jsr     MetaSprite.Deactivate

                ldx.b   BaseEntity.functionTable
                jsr     (BaseEntityFunctionTable.Deactivated,x)


                lda.b   BaseEntity.next
                bne     InnerLoop

            plx

            // Move the entity list to the activateNextFrame list

            // lastEntity->next = activateNextFrame.first
            // currentList->first = activateNextFrame.first
            // currentList = NULL

            lda.w   specialLists.activateNextFrame
            sta.b   BaseEntity.next

            lda.w   lists.FIRST,x
            sta.w   specialLists.activateNextFrame

            stz.w   lists.FIRST,x

    ListEmpty:
        // X = _currentEntityList
        inx
        inx
        cpx.w   #N_ENTITY_LISTS * 2
        bcc     OuterLoop


    lda.w   #0
    tdc

    rts
}



// Preloads the entity MetaSprites and activates the entities in the
// `activateNextFrame` special list.
//
// This routine is useful for preloading the entity MetaSprites when
// transitioning between menu-mode and game-mode.
//
// As this routine might not have enough DMA time to upload all entities in the
// `activateNextFrame` special list it is recommended you call this routine:
//      * When the display is enabled.
//      * Once per frame during the menu-mode fade-out transition effect.
//      * Just before the `WaitFrame` call.
//
// MUST NOT BE CALLED in the Entity GameLoop.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function PreloadEntityMetaSprites {
    phd

    jsr     Entity._ProcessActivateNextFrameList

    pld
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

