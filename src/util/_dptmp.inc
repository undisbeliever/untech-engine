// util/_dptmp.inc
// ===============
//
// UnTech Engine Direct Page Temporary Variables.
//
// These variables may be used by the system to store state that crosses
// call boundaries and MUST NOT BE USED by the subproject.
//
// It is expected that the subproject create its own set of direct page
// temporary variables in order to ensure that the utDpTmp variables are
// not accidentally overwritten.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


constant UT_DP_TMP_COUNT = 20
evaluate _n = 0
while {_n} < UT_DP_TMP_COUNT {
    // UnTech Engine Direct Page Temporary variable.
    // MUST NOT BE USED by the subproject.
    // (dp word)
    allocate(utDpTmp{_n}, dp, 2)
    evaluate _n = {_n} + 1
}


// Dirties all the utDpTmp variables
//
// REQUIRES: none
code()
function _DirtyUtDpTmp {
    php
    phd

    rep     #$30
i16()
a16()

    lda.w   #0
    tcd

    lda.l   frameCounter
    eor.l   JOY1
    eor.l   JOY2
    eor.w   #0xffff

    ldx.w   #UT_DP_TMP_COUNT * 2 - 2
    -
        dec
        sta.b   utDpTmp0,x

        dex
        dex
        bpl     -

    pld
    plp
    rts
}


// vim: ft=bass-65816 ts=4 sw=4 et:

