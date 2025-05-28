// hdma/_common.inc
// ================
//
// Common HDMA routines.
//
//
// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Hdma {

// Initializes the module
//
// REQUIRES: DB access lowram
macro Init() {
    stz.w   Hdma.ChannelSettings.enabled
}


// Deactivate the HDMA buffer
//
// REQUIRES: DB access lowram
macro Deactivate() {
    stz.w   Hdma.ChannelSettings.enabled
}


// Retrieve the next Hdma Table Buffer.
// The buffer returned alternates each function call.
//
// OUTPUT: X = address of Hdma Table Buffer
//
// REQUIRES: 8 bit A, 16 bit X, DB = $7E
a8()
i16()
farCode()
function GetBuffer {
    lda.w   currentBuffer
    bne     +
        dec.w   currentBuffer
        ldx.w   #buffer1
        rts
    +

    stz.w   currentBuffer
    ldx.w   #buffer2
    rts
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank_dp4300() {
    assert16a()
    assert8i()

    ldx.w   Hdma.ChannelSettings.enabled
    beq     Skip
        assert(Hdma.ChannelSettings.dmap + 1 == Hdma.ChannelSettings.register)
        lda.w   Hdma.ChannelSettings.dmap
        sta.b   DMAP7       // also sets BBAD7

        lda.w   Hdma.ChannelSettings.tableAddr
        sta.b   A1T7

        ldx.b   #Hdma.buffer1 >> 16
        stx.b   A1B7
        stx.b   DASB7

        ldx.b   #HDMAEN.dma7
        stx.w   HDMAEN
    Skip:
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

