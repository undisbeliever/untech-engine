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

namespace MathTest {

a16()
i16()
code()
Test.add("neg.b")
function neg.b {
    sep     #$20
a8()

    lda.b   #42
    Math.neg.b()

    cmp.b   #-42
    bne     Fail

    lda.b   #-120
    Math.neg.b()

    cmp.b   #120
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("neg.w")
function neg.w {
    lda.w   #0x1337
    Math.neg.w()

    cmp.w   #-0x1337
    bne     Fail

    lda.w   #-1024
    Math.neg.w()

    cmp.w   #1024
    bne     Fail

    sec
    rts

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Negate32xy")
function Negate32xy {
    macro test(evaluate num) {
        ldy.w   #{num} & 0xffff
        ldx.w   #{num} >> 16

        Math.Negate32xy()

        cpy.w   #(0-{num}) & 0xffff
        bne     Fail
        cpx.w   #(0-{num}) >> 16
        bne     Fail
    }

    test(0)
    test(-1)
    test(0x1337beef)

    sec
    rts

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Negate32.b")
function Negate32.b {
    macro test(evaluate num) {
        lda.w   #{num} & 0xffff
        sta.b   Test.zpTmp
        lda.w   #{num} >> 16
        sta.b   Test.zpTmp + 2

        lda.w   #0

        Math.Negate32.b(Test.zpTmp)

        lda.b   Test.zpTmp
        cmp.w   #(0-{num}) & 0xffff
        bne     Fail

        lda.b   Test.zpTmp + 2
        cmp.w   #(0-{num}) >> 16
        bne     Fail
    }

    test(0)
    test(42)
    test(0xdeadbeef)

    sec
    rts

Fail:
    clc
    rts
}



a16()
i16()
code()
Test.add("Negate32.w")
function Negate32.w {
    macro test(evaluate num) {
        lda.w   #{num} & 0xffff
        sta.w   Test.tmp
        lda.w   #{num} >> 16
        sta.w   Test.tmp + 2

        lda.w   #0

        Math.Negate32.w(Test.tmp)

        lda.w   Test.tmp
        cmp.w   #(0-{num}) & 0xffff
        bne     Fail

        lda.w   Test.tmp + 2
        cmp.w   #(0-{num}) >> 16
        bne     Fail
    }

    test(0)
    test(0xffffffff)
    test(123)

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

