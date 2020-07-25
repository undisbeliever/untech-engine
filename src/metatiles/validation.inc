// metatiles/validation.inc
// ========================
//
// MetaTiles validation routine.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

RegisterBreakId(BROKEN_CRUMBLING_TILES_QUEUE)

namespace MetaTiles {


// Validates the Crumbling Tiles subsystem
//
// REGISTERS: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
//
// PARAM: ns = ChainA or ChainB
macro _ValidateCrumblingTiles(ns) {
    assert8a()
    assert8i()
}




// Validate the MetaTiles subsystem
//
// Breaks if the MetaTiles subsystem is invalid
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: room loaded into memory
a16()
i16()
code()
function Validate {
    sep     #$30
a8()
i8()
    _ValidateCrumblingTiles(ChainA)
    _ValidateCrumblingTiles(ChainB)

    rep     #$30
a16()
i16()
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

