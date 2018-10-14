// util/_dptmp.inc
// ===============
//
// Direct Page Temporary Variables.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


constant DP_TMP_COUNT = 20
evaluate _n = 0
while {_n} < DP_TMP_COUNT {
    allocate(dpTmp{_n}, dp, 2)
    evaluate _n = {_n} + 1
}


// Dirties all the dpTmp variables
//
// REQUIRES: none
code()
function _DirtyDpTmp {
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

    ldx.w   #DP_TMP_COUNT * 2 - 2
    -
        dec
        sta.b   dpTmp0,x

        dex
        dex
        bpl     -

    pld
    plp
    rts
}


// vim: ft=bass-65816 ts=4 sw=4 et:

