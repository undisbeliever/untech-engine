// room/maps.inc
// =============
//
// Room map routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Room {
namespace Maps {


// Initialize the Room Subsystem
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
macro _Init() {
    assert16a()
    assert16i()
}



// Load and verify the Map and Submap data
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
//
// OUTPUT: X = address after map data
macro _Load() {
    assert16a()
    assert16i()

    MetaTiles._LoadMap()

    lda.w   MetaTiles.map.dataSize
    clc
    adc.w   #roomData + Format.Header.size
    tax


    // ::TODO submaps::
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

