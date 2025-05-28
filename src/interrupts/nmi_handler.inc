// interrupts/nmi_handler.inc
// ==========================
//
// NMI Interrupt Service Routine.
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


au()
iu()
code(interrupt_code)
function NmiHandler {
    // ensure NmiHandler is accessible by interrupt vectors
    assert((pc() & 0x3fffff) >= 0x008000 && (pc() & 0x3fffff) < 0x00ffff)

// Number of NMIs processed since the counter was last reset
// (byte)
allocate(counter, lowram, 1)

    // Store registers
    sep     #$20
a8()
    pha

    lda.l   counter
    inc
    sta.l   counter

    lda.l   RDNMI

    pla
    rti
}

// vim: ft=bass-65816 ts=4 sw=4 et:

