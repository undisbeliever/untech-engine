// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
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



// Fades in the screen over 15 frames
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
au()
iu()
code()
function FadeIn {
    sep     #$20
a8()
    Loop:
        lda.w   inidispBuffer
        bpl     +
            lda.b   #-1
        +
        inc
        cmp.b   #INIDISP.brightness.mask + 1
        bcs     Break
        sta.w   inidispBuffer

        jsl     WaitFrame__far
        bra     Loop
    Break:

    rep     #$30
a16()
    rts
}


// Fades out the screen over 16 frames
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
au()
iu()
code()
function FadeOut {
    sep     #$20
a8()
    Loop:
        lda.w   inidispBuffer
        bmi     Break
        and.b   #0xf
        dec
        sta.w   inidispBuffer

        jsl     WaitFrame__far
        bra     Loop
    Break:

    rep     #$30
a16()
    rts
}


