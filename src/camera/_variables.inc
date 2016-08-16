
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
    constant ACTIVE_WINDOW_EXTEND_HORIZONTAL(96)
    constant ACTIVE_WINDOW_EXTEND_VERTICAL(96)

    // Inactive window size (beyond camera display)
    // Active entities that leave the window will deactivate.
    constant INACTIVE_WINDOW_EXTEND_HORIZONTAL(128)
    constant INACTIVE_WINDOW_EXTEND_VERTICAL(128)

    // Camera position
    // (2x uint16)
    allocate(xPos, wram7e, 2)
    allocate(yPos, wram7e, 2)

    // Activate window
    // (4x uint16)
    scope activeWindow {
        allocate(top, wram7e, 2)
        allocate(bottom, wram7e, 2)
        allocate(left, wram7e, 2)
        allocate(right, wram7e, 2)
    }

    // Inactivate window
    // (4x uint16)
    scope inactiveWindow {
        allocate(top, wram7e, 2)
        allocate(bottom, wram7e, 2)
        allocate(left, wram7e, 2)
        allocate(right, wram7e, 2)
    }
}

// vim: ft=asm ts=4 sw=4 et:

