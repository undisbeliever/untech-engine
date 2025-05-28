// entity/transitions.inc
// ======================
//
// Routines to assist in the transition between game-mode and menu-mode.
//
// SPDX-FileCopyrightText: © 2018 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2018 Marcus Rowe <undisbeliever@gmail.com>
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

