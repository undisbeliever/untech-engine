
scope Camera {

// Initializes the Camera module
//
// This MUST be called before using this module
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
a16()
i16()
code()
scope Init: {
    // Reset the camera to origin

    lda.w   #ORIGIN
    sta.w   xPos
    sta.w   yPos

    lda.w   #ORIGIN - ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.activeWindow.left
    lda.w   #ORIGIN + DISPLAY_WIDTH + ACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.activeWindow.right

    lda.w   #ORIGIN - ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.activeWindow.top
    lda.w   #ORIGIN + DISPLAY_HEIGHT + ACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.activeWindow.bottom

    lda.w   #ORIGIN - INACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.inactiveWindow.left
    lda.w   #ORIGIN + DISPLAY_WIDTH + INACTIVE_WINDOW_EXTEND_HORIZONTAL
    sta.w   Camera.inactiveWindow.right

    lda.w   #ORIGIN - INACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.inactiveWindow.top
    lda.w   #ORIGIN + DISPLAY_HEIGHT + INACTIVE_WINDOW_EXTEND_VERTICAL
    sta.w   Camera.inactiveWindow.bottom

    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

