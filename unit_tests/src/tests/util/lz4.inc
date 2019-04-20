// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace LZ4Test {

constant BUFFER_SIZE = 1024
allocate(buffer, wram7e, BUFFER_SIZE)

macro _Decompress(label, evaluate expectedSize) {
    assert16a()
    assert16i()
    assert({expectedSize} <= BUFFER_SIZE)

    lda.w   #{label}
    sta.b   LZ4.ptr
    lda.w   #{label} >> 8
    sta.b   LZ4.ptr + 1

    ldy.w   #BUFFER_SIZE
    ldx.w   #buffer
    lda.w   #buffer >> 16

    jsr     LZ4.Decompress

    cpx.w   #{label} + {label}.size
    bne     Fail
    cpy.w   #buffer + {expectedSize}
    bne     Fail
}

a16()
i16()
code()
Test.add("LZ4.Decompress (FontFixed)")
function Decompress_FontFixed {
    _Decompress(FontFixed_LZ4, Text.Font.FixedTiles.size)

    sep     #$20
a8()

    ldx.w   #Text.Font.FixedTiles.size - 1
    Loop:
        lda.w   buffer,x
        cmp.l   Text.Font.FixedTiles,x
        bne     Fail

        dex
        bpl     Loop

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("LZ4.Decompress (1kzeros)")
function Decompress_OneKZeros {
    _Decompress(OneKZeros_LZ4, 1024)

    ldx.w   #1024 - 2
    lda.w   #0
    Loop:
        cmp.w   buffer,x
        bne     Fail

        dex
        dex
        bpl     Loop

    sec
    rts

Fail:
    clc
    rts
}


a16()
i16()
code()
Test.add("LZ4.Decompress (incompressible)")
function Decompress_Incompressible {
    _Decompress(Incompressable_LZ4, 256 * 4)

    ldx.w   #(256 - 1) * 4
    lda.w   #0xffff
    Loop:
        cmp.w   buffer,x
        bne     Fail

        cmp.w   buffer + 2,x
        bne     Fail

        sec
        sbc.w   #0x0101

        dex
        dex
        dex
        dex
        bpl     Loop

    sec
    rts

Fail:
    clc
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

