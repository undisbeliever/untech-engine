// camera/_common.inc
// ==================
//
// Common routines for the Camera Module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

