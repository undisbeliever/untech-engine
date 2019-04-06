// camera/_variables.inc
// =====================
//
// Variables used by the Camera module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Camera {

    // The offset between the map origin (0,0) and the camera
    //
    // This offset is applied to the position of the camera and all of
    // the entities. It allows me to use unsigned math/comparisons
    // in the gameloop without worrying about signed overflows.
    constant ORIGIN = 0x1000

    // Size of the display area of the game field.
    // (Size of screen - GUI)
    constant DISPLAY_HEIGHT = 224
    constant DISPLAY_WIDTH = 256

    // Camera position
    // (2x uint16, project editable)
    allocate(xPos, wram7e, 2)
    allocate(yPos, wram7e, 2)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

