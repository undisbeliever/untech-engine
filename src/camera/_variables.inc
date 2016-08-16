
scope Camera {

    // The offset between the map origin (0,0) and the camera
    //
    // This offset is applied to the position of the camera and all of
    // the entities. It allows me to use unsigned math/comparisons
    // in the gameloop without worrying about signed overflows.
    constant ORIGIN(0x1000)

    // Size of the display area of the game field.
    // (Size of screen - GUI)
    constant DISPLAY_HEIGHT(224)
    constant DISPLAY_WIDTH(256)

    // Active window size (beyond camera display)
    // inactive entities inside this window will become active.
    constant ACTIVE_WINDOW_EXTEND(96)

    // Active window size (beyond camera display)
    // Active entities that leave the window will deactivate.
    constant INACTIVE_WINDOW_EXTEND(128)

    // Camera position
    // (2x uint16)
    allocate(xPos, wram7e, 2)
    allocate(yPos, wram7e, 2)
}

// vim: ft=asm ts=4 sw=4 et:

