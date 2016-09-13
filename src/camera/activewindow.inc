
scope Camera {

// Updates the active and inactive window variables
// REQUIRES: 16 bit A, DB = 0x7e
macro _UpdateWindows() {
    assert16a()

    assert(Camera.INACTIVE_WINDOW_EXTEND_HORIZONTAL > Camera.ACTIVE_WINDOW_EXTEND_HORIZONTAL)
    assert(Camera.INACTIVE_WINDOW_EXTEND_VERTICAL > Camera.ACTIVE_WINDOW_EXTEND_VERTICAL)

    lda.w   Camera.xPos
    sec
    sbc.w   #Camera.ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.activeWindow.left

    sec
    sbc.w   #Camera.INACTIVE_WINDOW_EXTEND_HORIZONTAL - Camera.ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.inactiveWindow.left

    clc
    adc.w   #Camera.INACTIVE_WINDOW_EXTEND_HORIZONTAL + Camera.DISPLAY_WIDTH + Camera.ACTIVE_WINDOW_EXTEND_HORIZONTAL + 1
    sta.w   Camera.activeWindow.right

    clc
    adc.w   #Camera.INACTIVE_WINDOW_EXTEND_HORIZONTAL - Camera.ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.inactiveWindow.right


    lda.w   Camera.yPos
    sec
    sbc.w   #Camera.ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.activeWindow.top

    sec
    sbc.w   #Camera.INACTIVE_WINDOW_EXTEND_VERTICAL - Camera.ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.inactiveWindow.top

    clc
    adc.w   #Camera.INACTIVE_WINDOW_EXTEND_VERTICAL + Camera.DISPLAY_HEIGHT + Camera.ACTIVE_WINDOW_EXTEND_VERTICAL + 1
    sta.w   Camera.activeWindow.bottom

    clc
    adc.w   #Camera.INACTIVE_WINDOW_EXTEND_VERTICAL - Camera.ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.inactiveWindow.bottom
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
    cmp.w   Camera.activeWindow.left
    bcc     {label}
    cmp.w   Camera.activeWindow.right
    bcs     {label}

    lda.b   BaseEntity.yPos.px
    cmp.w   Camera.activeWindow.top
    bcc     {label}
    cmp.w   Camera.activeWindow.bottom
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
    cmp.w   Camera.inactiveWindow.left
    bcc     {label}
    cmp.w   Camera.inactiveWindow.right
    bcs     {label}

    lda.b   BaseEntity.yPos.px
    cmp.w   Camera.inactiveWindow.top
    bcc     {label}
    cmp.w   Camera.inactiveWindow.bottom
    bcs     {label}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

