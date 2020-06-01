// camera/_variables.inc
// =====================
//
// Variables used by the Camera module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Camera {

    // Size of the display area of the game field.
    // (Size of screen - GUI)
    constant DISPLAY_HEIGHT = 224
    constant DISPLAY_WIDTH = 256


    // Camera position
    // (2x uint16)
    allocate(xPos, wram7e, 2)
    allocate(yPos, wram7e, 2)


    // The maximum allowed camera position
    // (2x uint16)
    allocate(maxXPos, wram7e, 2)
    allocate(maxYPos, wram7e, 2)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

