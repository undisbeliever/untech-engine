// entity/activewindow.inc
// =======================
//
// Tests if the entity is outside the active window.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Entity {

// Updates the active and inactive window variables
//
// REQUIRES: 16 bit A, DB = 0x7e
a16()
iu()
code()
function _UpdateActiveWindows {
    assert(INACTIVE_WINDOW_EXTEND_HORIZONTAL > ACTIVE_WINDOW_EXTEND_HORIZONTAL)
    assert(INACTIVE_WINDOW_EXTEND_VERTICAL > ACTIVE_WINDOW_EXTEND_VERTICAL)

    lda.w   Camera.xPos
    sec
    sbc.w   #ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   activeWindow.left

    sec
    sbc.w   #INACTIVE_WINDOW_EXTEND_HORIZONTAL - ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   inactiveWindow.left

    clc
    adc.w   #INACTIVE_WINDOW_EXTEND_HORIZONTAL + Camera.DISPLAY_WIDTH + ACTIVE_WINDOW_EXTEND_HORIZONTAL + 1
    sta.w   activeWindow.right

    clc
    adc.w   #INACTIVE_WINDOW_EXTEND_HORIZONTAL - ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   inactiveWindow.right


    lda.w   Camera.yPos
    sec
    sbc.w   #ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   activeWindow.top

    sec
    sbc.w   #INACTIVE_WINDOW_EXTEND_VERTICAL - ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   inactiveWindow.top

    clc
    adc.w   #INACTIVE_WINDOW_EXTEND_VERTICAL + Camera.DISPLAY_HEIGHT + ACTIVE_WINDOW_EXTEND_VERTICAL + 1
    sta.w   activeWindow.bottom

    clc
    adc.w   #INACTIVE_WINDOW_EXTEND_VERTICAL - ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   inactiveWindow.bottom

    rts
}


// Branches to the given label if the current entity is outside the active window
//
// REQUIRES: 16 bit A, DB = 0x7e
// MODIFIES: A
// DOES NOT MODIFY: X, Y
// INPUT: dp = entity
// BRANCHES: to label if entity is outside active window
macro BranchEntityOutsideActiveWindow(label) {
    assert16a()

    lda.b   BaseEntity.xPos.px
    cmp.w   Entity.activeWindow.left
    bcc     {label}
    cmp.w   Entity.activeWindow.right
    bcs     {label}

    lda.b   BaseEntity.yPos.px
    cmp.w   Entity.activeWindow.top
    bcc     {label}
    cmp.w   Entity.activeWindow.bottom
    bcs     {label}
}


// Branches to the given label if the current entity is outside the inactive window
//
// REQUIRES: 16 bit A, DB = 0x7e
// MODIFIES: A
// DOES NOT MODIFY: X, Y
// INPUT: dp = entity
// BRANCHES: to label if entity is outside active window
macro BranchEntityOutsideInactiveWindow(label) {
    assert16a()

    lda.b   BaseEntity.xPos.px
    cmp.w   Entity.inactiveWindow.left
    bcc     {label}
    cmp.w   Entity.inactiveWindow.right
    bcs     {label}

    lda.b   BaseEntity.yPos.px
    cmp.w   Entity.inactiveWindow.top
    bcc     {label}
    cmp.w   Entity.inactiveWindow.bottom
    bcs     {label}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

