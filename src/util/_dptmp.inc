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
// SPDX-FileCopyrightText: © 2018 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2018 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


constant UT_DP_TMP_COUNT = 20
evaluate _n = 0
while {_n} < UT_DP_TMP_COUNT {
    // UnTech Engine Direct Page Temporary variable.
    // MUST NOT BE USED by the subproject.
    // (dp word)
    allocate(utDpTmp{_n}, zeropage, 2)
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

