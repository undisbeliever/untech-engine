// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace MathTest {

a16()
i16()
code()
Test.add("Multiply.u8y_u8x_y")
function Multiply.u8y_u8x_y {
    TableLoop(Table, 4)

        ldx.w   tableData
        ldy.w   tableData + 1

        jsr     Math.Multiply.u8y_u8x_y

        cpy.w   tableData + 2
        beq     Loop

    // fail
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        db  {a}, {b}
        dw  {a} * {b}
    }

    row(0, 0)
    row(0xff, 0xff)
    row(10, 10)
    row(100, 100)
Table.End:
}



a16()
i16()
code()
Test.add("Multiply.16y_u8a_16y")
function Multiply.16y_u8a_16y {
    TableLoop(Table, 5)

        sep     #$20
    a8()

        ldy.w   tableData
        lda.w   tableData + 2

        jsl     Math.Multiply.u16y_u8a_u16y__far

        rep     #$20
    a16()

        cpy.w   tableData + 3
        beq     Loop

    // fail
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dw  {a}
        db  {b}
        dw  {a} * {b}
    }

assert(Math.Multiply.s16y_u8a_s16y__far == Math.Multiply.u16y_u8a_u16y__far)

    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
    }

    row(0, 0)
    row(0xeeee, 0xee)
    manyRows(1000, 100)
    manyRows(0x7777, 0x77)
    manyRows(128, 33)
    manyRows(666, 128)  // overflow test
Table.End:
}



a16()
i16()
code()
Test.add("Multiply.u16y_u8a_u32xy")
function Multiply.u16y_u8a_u32xy {
    TableLoop(Table, 7)

        sep     #$20
    a8()

        ldy.w   tableData
        lda.w   tableData + 2

        jsr     Math.Multiply.u16y_u8a_u32xy

        rep     #$20
    a16()

        cpy.w   Math.product32
        bne     Fail

        cpx.w   Math.product32 + 2
        bne     Fail

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
        dd  {a} * {b}
    }

    row(0, 0)
    row(0xffff, 0xff)
    row(10, 56)
    row(100, 56)
    row(1000, 56)
    row(10000, 56)
Table.End:
}



a16()
i16()
code()
Test.add("Multiply.16y_16x_16y")
function Multiply.16y_16x_16y {
    TableLoop(Table, 6)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Multiply.u16y_u16x_u16y

        cpy.w   tableData + 4
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dw  {a}, {b}, {a} * {b}
    }

assert(Math.Multiply.u16y_s16x_16y == Math.Multiply.u16y_u16x_u16y)
assert(Math.Multiply.s16y_u16x_16y == Math.Multiply.u16y_u16x_u16y)
assert(Math.Multiply.s16y_s16x_s16y == Math.Multiply.u16y_u16x_u16y)

    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
        row({a}, -{b})
        row(-{a}, -{b})
    }

    row(0, 0)
    row(0xffff, 0xffff)
    row(0xdead, 0xbeef)
    manyRows(10, 10)
    manyRows(100, 100)
    manyRows(1000, 1000)
    manyRows(1024, 1024)
    manyRows(42, 42)
    manyRows(0x1337, 0x1337)
Table.End:
}




a16()
i16()
code()
Test.add("Multiply.u16y_u16x_u32xy")
function Multiply.u16y_u16x_u32xy {
    TableLoop(Table, 8)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Multiply.u16y_u16x_u32xy

        cpy.w   Math.product32
        bne     Fail

        cpx.w   Math.product32 + 2
        bne     Fail

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
        dd  {a} * {b}
    }

    row(0, 0)
    row(10, 10)
    row(100, 100)
    row(1000, 1000)
    row(10000, 10000)
    row(0xffff, 0xffff)
Table.End:
}




a16()
i16()
code()
Test.add("Multiply.s16y_s16x_s32xy")
function Multiply.s16y_s16x_s32xy {

    TableLoop(Table, 8)

        ldy.w   tableData
        ldx.w   tableData + 2

        jsr     Math.Multiply.s16y_s16x_s32xy

        cpy.w   Math.product32
        bne     Fail

        cpx.w   Math.product32 + 2
        bne     Fail

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
        dd  {a} * {b}
    }

    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
        row({a}, -{b})
        row(-{a}, -{b})
    }

    row(0, 0)
    manyRows(10, 10)
    manyRows(100, 100)
    manyRows(1000, 1000)
    manyRows(1024, 1024)
    manyRows(0x1337, 0x1337)
Table.End:
}




a16()
i16()
code()
Test.add("Multiply.u32_32xy_32xy")
function Multiply.u32_32xy_32xy {

    TableLoop(Table, 12)

        ldy.w   tableData
        sty.w   Math.factor32
        ldx.w   tableData + 2
        stx.w   Math.factor32 + 2

        ldy.w   tableData + 4
        ldx.w   tableData + 6

        jsr     Math.Multiply.u32_u32xy_u32xy

        cpy.w   Math.product32
        bne     Fail

        cpx.w   Math.product32 + 2
        bne     Fail

        cpy.w   tableData + 8
        bne     Fail

        cpx.w   tableData + 10
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dd  {a}, {b}, {a} * {b}
    }

assert(Math.Multiply.u32_s32xy_32xy == Math.Multiply.u32_u32xy_u32xy)
assert(Math.Multiply.s32_u32xy_32xy == Math.Multiply.u32_u32xy_u32xy)
assert(Math.Multiply.s32_s32xy_s32xy == Math.Multiply.u32_u32xy_u32xy)

    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
        row({a}, -{b})
        row(-{a}, -{b})
    }

    row(0, 0)
    row(0xffffffff, 0xffffffff)
    row(0xdeadbeef, 0xdeadbeef)

    manyRows(10, 10)
    manyRows(100, 100)
    manyRows(1000, 1000)
    manyRows(10000, 10000)
    manyRows(100000, 100000)
    manyRows(1000000, 1000000)
    manyRows(0xffffff, 0xffffff)
    manyRows(0x1337, 0x1337)
Table.End:
}




a16()
i16()
code()
Test.add("Multiply.32xy_u8a_32xy")
function Multiply.32_u8a_32xy {

    TableLoop(Table, 9)

        sep     #$20
    a8()

        ldy.w   tableData + 0
        ldx.w   tableData + 2
        lda.w   tableData + 4

        jsr     Math.Multiply.u32xy_u8a_u32xy

        rep     #$20
    a16()

        cpy.w   Math.product32
        bne     Fail

        cpx.w   Math.product32 + 2
        bne     Fail

        cpy.w   tableData + 5
        bne     Fail

        cpx.w   tableData + 7
        beq     Loop

Fail:
    clc
    rts

rodata(rom0)
Table:
    macro row(evaluate a, evaluate b) {
        dd  {a}
        db  {b}
        dd  {a} * {b}
    }

assert(Math.Multiply.s32xy_u8a_s32xy == Math.Multiply.u32xy_u8a_u32xy)

    macro manyRows(evaluate a, evaluate b) {
        row({a}, {b})
        row(-{a}, {b})
    }

    row(0, 0)
    row(0xffffff, 0xff)
    row(0xffffffff, 0xff)
    row(0xdeadbeef, 32)

    manyRows(10, 10)
    manyRows(100, 10)
    manyRows(1000, 10)
    manyRows(10000, 10)
    manyRows(100000, 10)
    manyRows(1000000, 10)
    manyRows(0x1337, 0x80)
    manyRows(0x1337, 0x25)
    manyRows(0x13371337, 64)
Table.End:
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

