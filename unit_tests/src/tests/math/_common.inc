// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace MathTest {

allocate(tableIndex, wram7e, 2)
allocate(tableData, wram7e, 16)


inline TableLoop(Table, evaluate size) {
    assert16a()
    assert16i()

    assert({size} > 1 && {size} <= 16)

    constant TableBank = {Table} >> 16

    ldx.w   #{Table}
    stx.w   tableIndex

    Loop:
        ldx.w   tableIndex
        cpx.w   #Table.End
        bcc     +
            // C set - return true
            rts
    +

        ldy.w   #tableData
        lda.w   #{size} - 1

        mvn     0x7e=TableBank

        stx.w   tableIndex
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

