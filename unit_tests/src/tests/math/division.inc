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
Test.add("Divide.u16y_u8a")
function Divide.u16y_u8a {
    TableLoop(Table, 7)

        sep     #$20
    a8()

        ldy.w   tableData
        lda.w   tableData + 2

        jsr     Math.Divide.u16y_u8a

        rep     #$20
    a16()

        cpy.w   tableData + 3
        bne     Fail

        cpx.w   tableData + 5
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dw  {a}
        db  {b}
        dw  {a} / {b}
        dw  {a} % {b}
    }

    row(100, 10)
    row(10000, 100)
    row(0x1337, 5)
    row(0xffff, 0xff)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.u16y_s16x")
function Divide.u16y_s16x {
    TableLoop(Table, 8)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Divide.u16y_s16x

        cpy.w   tableData + 4
        bne     Fail

        cpx.w   tableData + 6
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        evaluate rem = {a} % {b}
        if ({rem} < 0) {
            evaluate rem = 0-{rem}
        }

        dw  {a}, {b}
        dw  {a} / {b}
        dw  {rem}
    }
    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row({a}, -{b})
    }

    manyRows(100, 10)
    manyRows(10000, 1)
    manyRows(10000, 1000)
    manyRows(0xdead, 0x1337)
    manyRows(0x1337, 0x7777)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.s16y_s16x")
function Divide.s16y_s16x {
    TableLoop(Table, 8)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Divide.s16y_s16x

        cpy.w   tableData + 4
        bne     Fail

        cpx.w   tableData + 6
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        evaluate rem = {a} % {b}
        if ({rem} < 0) {
            evaluate rem = 0-{rem}
        }

        dw  {a}, {b}
        dw  {a} / {b}
        dw  {rem}
    }
    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
        row({a}, -{b})
        row(-{a}, -{b})
    }

    manyRows(100, 10)
    manyRows(10000, 1)
    manyRows(10000, 1000)
    manyRows(0x7777, 0x1337)
    manyRows(0x1337, 0x7777)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.s16y_u16x")
function Divide.s16y_u16x {
    TableLoop(Table, 8)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Divide.s16y_u16x

        cpy.w   tableData + 4
        bne     Fail

        cpx.w   tableData + 6
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        evaluate rem = {a} % {b}
        if ({rem} < 0) {
            evaluate rem = 0-{rem}
        }

        dw  {a}, {b}
        dw  {a} / {b}
        dw  {rem}
    }
    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
    }

    manyRows(100, 10)
    manyRows(10000, 1)
    manyRows(10000, 1000)
    manyRows(0x1337, 0xdead)
    manyRows(0x7777, 0x1337)
    manyRows(0x1337, 0x7777)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.u16y_u16x")
function Divide.u16y_u16x {
    TableLoop(Table, 8)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Divide.u16y_u16x

        cpy.w   tableData + 4
        bne     Fail

        cpx.w   tableData + 6
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dw  {a}, {b}
        dw  {a} / {b}
        dw  {a} % {b}
    }

    row(100, 10)
    row(10000, 1)
    row(10000, 1000)
    row(0x1337, 0xdead)
    row(0x7777, 0x1337)
    row(0xffff, 0xff)
    row(0xff, 0xffff)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.s32_s32")
function Divide.s32_s32 {
    TableLoop(Table, 16)

        ldy.w   tableData
        sty.w   Math.dividend32
        ldx.w   tableData + 2
        stx.w   Math.dividend32 + 2

        ldy.w   tableData + 4
        sty.w   Math.divisor32
        ldx.w   tableData + 6
        stx.w   Math.divisor32 + 2

        jsr     Math.Divide.s32_s32

        ldy.w   Math.result32
        cpy.w   tableData + 8
        bne     Fail

        ldx.w   Math.result32 + 2
        cpx.w   tableData + 10
        bne     Fail

        ldy.w   Math.remainder32
        cpy.w   tableData + 12
        bne     Fail

        ldx.w   Math.remainder32 + 2
        cpx.w   tableData + 14
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        evaluate rem = {a} % {b}
        if ({rem} < 0) {
            evaluate rem = 0-{rem}
        }

        dd  {a}, {b}
        dd  {a} / {b}
        dd  {rem}
    }
    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
        row({a}, -{b})
        row(-{a}, -{b})
    }

    manyRows(100, 10)
    manyRows(10000, 1)
    manyRows(10000, 1000)
    manyRows(0x7777, 0x1337)
    manyRows(0x1337, 0x7777)
    manyRows(0x77777777, 0x13371337)
    manyRows(0x13371337, 0x77777777)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.u32_u32")
function Divide.u32_u32 {
    TableLoop(Table, 16)

        ldy.w   tableData
        sty.w   Math.dividend32
        ldx.w   tableData + 2
        stx.w   Math.dividend32 + 2

        ldy.w   tableData + 4
        sty.w   Math.divisor32
        ldx.w   tableData + 6
        stx.w   Math.divisor32 + 2

        jsr     Math.Divide.u32_u32

        ldy.w   Math.result32
        cpy.w   tableData + 8
        bne     Fail

        ldx.w   Math.result32 + 2
        cpx.w   tableData + 10
        bne     Fail

        ldy.w   Math.remainder32
        cpy.w   tableData + 12
        bne     Fail

        ldx.w   Math.remainder32 + 2
        cpx.w   tableData + 14
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dd  {a}, {b}
        dd  {a} / {b}
        dd  {a} % {b}
    }

    row(100, 10)
    row(10000, 1)
    row(10000, 1000)
    row(0x7777, 0x1337)
    row(0x1337, 0x7777)
    row(0x77777777, 0x13371337)
    row(0x13371337, 0x77777777)
    row(0xffffffff, 0xffff)
Table.End:
}



a16()
i16()
code()
Test.add("Divide.u32_u8a")
function Divide.u32_u8a {
    TableLoop(Table, 10)

        sep     #$20
    a8()

        ldy.w   tableData
        sty.w   Math.dividend32
        ldx.w   tableData + 2
        stx.w   Math.dividend32 + 2

        lda.w   tableData + 4

        jsr     Math.Divide.u32_u8a

        ldy.w   Math.result32
        cpy.w   tableData + 5
        bne     Fail

        ldx.w   Math.result32 + 2
        cpx.w   tableData + 7
        bne     Fail

        // remainder
        cmp.w   tableData + 9
        bne     Fail

        rep     #$20
    a16()
        bra     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dd  {a}
        db  {b}
        dd  {a} / {b}
        db  {a} % {b}
    }

    row(10, 10)
    row(100, 10)
    row(1000, 10)
    row(100000, 100)
    row(0x1337, 200)
    row(0x7777, 100)
    row(0x13371337, 56)
    row(0x77777777, 42)
    row(0xffffffff, 0xff)
Table.End:
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

