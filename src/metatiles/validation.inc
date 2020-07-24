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


// Validate a single Crumbling Tiles linked list
//
// REGISTERS: 8 bit A, 8 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A = number of items in previous lists
// INPUT: X = first node index (can be null)
// OUTPUT: A = number of items in list + initial A value
// OUTPUT: X = last node in the list
inline __ValidateCrumblingTilesLinkedList(ns) {
    assert8a()
    assert8i()

    txy
    bmi     +
        -
            assert(CrumblingTiles.N_ELEMENTS * 2 < 0x80)
            inc
            bmi     Fail

            tyx
            ldy.w   CrumblingTiles.{ns}.Queue.next,x
            bpl     -
    +
    rts

Fail:
    break(BROKEN_CRUMBLING_TILES_QUEUE)
}
a8()
i8()
code()
function _ValidateCrumblingTilesLinkedList_ChainA {
    __ValidateCrumblingTilesLinkedList(ChainA)
}
a8()
i8()
code()
function _ValidateCrumblingTilesLinkedList_ChainB {
    __ValidateCrumblingTilesLinkedList(ChainB)
}



// Validates the Crumbling Tiles subsystem
//
// REGISTERS: 8 bit A, 8 bit Index, DB = 0x7e, DP = 0
//
// PARAM: ns = ChainA or ChainB
macro _ValidateCrumblingTiles(ns) {
    assert8a()
    assert8i()

    lda.b   #0

    ldx.w   CrumblingTiles.{ns}.free
    jsr     _ValidateCrumblingTilesLinkedList_{ns}

    ldx.w   CrumblingTiles.{ns}.firstHead
    jsr     _ValidateCrumblingTilesLinkedList_{ns}
    cpx.w   CrumblingTiles.{ns}.firstTail
    beq     +
        break(BROKEN_CRUMBLING_TILES_QUEUE)
    +

    ldx.w   CrumblingTiles.{ns}.secondHead
    jsr     _ValidateCrumblingTilesLinkedList_{ns}
    cpx.w   CrumblingTiles.{ns}.secondTail
    beq     +
        break(BROKEN_CRUMBLING_TILES_QUEUE)
    +

    ldx.w   CrumblingTiles.{ns}.thirdHead
    jsr     _ValidateCrumblingTilesLinkedList_{ns}
    cpx.w   CrumblingTiles.{ns}.thirdTail
    beq     +
        break(BROKEN_CRUMBLING_TILES_QUEUE)
    +

    cmp.b   #CrumblingTiles.N_ELEMENTS
    beq     +
        break(BROKEN_CRUMBLING_TILES_QUEUE)
    +
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

