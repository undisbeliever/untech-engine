// math/multiplication.inc
// =======================
//
// Multiplication Routines.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Math {

code()

// Multiply uint8 * uint8
//
// REQUIRES: nothing
//
// INPUT:
//  Y: unsigned integer (only low 8 bits are used)
//  X: unsigned integer (only low 8 bit are used)
//
// OUTPUT:
//  Y: result (8 or 16 bits depending on Index size)
au()
iu()
code()
function Multiply.u8y_u8x_y {
    php
    sep     #$20
a8()
    tya
    sta.l   WRMPYA

    txa
    sta.l   WRMPYB

    // wait 8 cycles
    rep     #$30    // 3
    nop             // 2
    lda.l   RDMPY   // 3 instruction fetch

    tay
    plp
    rts
}


// Multiply a 16 bit integer by uint8
//
// REQUIRES: 8 bit A, 16 bit Index
//
// INPUT:
//  Y: 16 bit integer
//  A: 8 bit unsigned integer
//
// OUTPUT:
//  Y: 16 bit product
a8()
i16()
code()
Multiply.s16y_u8a_s16y:
function Multiply.u16y_u8a_u16y {
    sta.l   WRMPYA
    tya
    sta.l   WRMPYB

    rep     #$30
a16()
    tya
    xba
    tax
    lda.l   RDMPY
    tay

    sep     #$20
a8()
    txa
    sta.l   WRMPYB

    // wait 8 cycles
    rep     #$31
    tya
    xba
    sep     #$20
a8()
    adc.l   RDMPY
    xba
    tay

    rts
}


// Multiply uint16 * uint8 = uint32
//
// REQUIRES: 8 bit A, 16 bit Index, DB access shadow
//
// INPUT:
//  Y: 16 bit unsigned integer
//  A: 8 bit unsigned integer
//
// OUTPUT:
//  XY: 32 bit unsigned product
//  product32: 32 bit unsigned product
a8()
i16()
code()
Multiply.u16y_u8a_u32:
function Multiply.u16y_u8a_u32xy {
    // DP faster than long addressing by 2 cycles

    phd
    pea     WRMPYA & 0xff00
    pld

    sta.b   WRMPYA
    tya
    sta.b   WRMPYB

    // wait 8 cycles
    sty.w   product32       // 5
    lda.w   product32 + 1   // 4

    ldy.b   RDMPY
    sty.w   product32


    // WRMPYA already set
    sta.b   WRMPYB

    // wait 8 cycles
    rep     #$31            // 3 - also clear carry
a16()
    lda.w   product32 + 1   // 5
    and.w   #$00ff
    adc.b   RDMPY

    sta.w   product32 + 1
    ldy.w   product32

    sep #$20
a8()
    lda.b   #0
    xba
    tax                     // 16 bits transfered
    stx.w   product32 + 2

    pld
    rts
}


// Multiply two 16 bit integers.
//
// The signs of the inputs and outputs are in the parameters.
//
// REQUIRES: nothing, 16 bit Index, DB access shadow
//
// INPUT:
//  Y: 16 bit factor
//  X: 16 bit factor
//
// OUTPUT:
//  Y: 16 bit product
au()
iu()
code()
Multiply.u16y_s16x_16y:
Multiply.s16y_u16x_16y:
Multiply.s16y_s16x_s16y:
function Multiply.u16y_u16x_u16y {
    //      y y
    // *    x x
    // -----------
    //      y*x
    // +  y*x
    // +  y*x

constant tmpY = tmp1
constant tmpX = tmp2

    php
    phd
    rep     #$30
a16()
i16()
    lda.w   #WRMPYA & 0xff00
    tcd

    sep     #$20
a8()
    txa
    sta.b   WRMPYA

    tya
    sta.b   WRMPYB

    // wait 8 cycles
    sty.w   tmpY        // 5
    stx.w   tmpX        // 5

    ldx.b   RDMPY       // yl*xl

    lda.w   tmpY + 1    // last use of tmpY to hold y
    sta.b   WRMPYB

    // wait 8 cycles
    stx.w   tmpY        // 5
    lda.w   tmpY + 1    // 4
    adc.b   RDMPY       // yh*xl    - no carry needed
    sta.w   tmpY + 1


    lda.w   tmpX + 1
    sta.b   WRMPYA

    tya
    sta.b   WRMPYB

    // wait 8 cycles
    clc                 // 2
    lda.w   tmpY + 1    // 4
    adc.b   RDMPY       // 2 - load address yl*xh
    sta.w   tmpY + 1

    ldy.w   tmpY

    pld
    plp
    rts
}


// Multiply uint16 * uint16 = uint32
//
// REQUIRES: 16 bit Index, DB access shadow
//
// INPUT:
//  Y: 16 bit factor
//  X: 16 bit factor
//
// OUTPUT:
//  XY: 32 bit product
//  product32: 32 bit product
au()
i16()
code()
function Multiply.u16y_u16x_u32xy {
    //       Y y
    //   *   X x
    // -----------
    //       y*x
    //     Y*x    -  no carry needed $FF * $FF + $FF < 0xffff
    // + c y*X
    // + Y*X

constant tmpY = tmp1
constant tmpX = tmp2

    php
    phd
    rep     #$30
a16()
i16()
    lda.w   #WRMPYA & 0xff00
    tcd

    sep     #$20
a8()

    txa
    sta.b   WRMPYA

    tya
    sta.b   WRMPYB

    // wait 8 cycles
    sty.w   tmpY            // 5
    stx.w   tmpX            // 5

    ldx.b   RDMPY           // xl * yl

    lda.w   tmpY + 1
    sta.b   WRMPYB

    // must not use y

    // wait 8 cycles
    stx.w   product32       // 5
    rep     #$31            // 3 clear c
a16()
    txa
    xba
    and.w   #$00ff          // a is bits 8-15 of x
    adc.b   RDMPY           // xl * yh
    sta.w   product32 + 1

    sep     #$20
a8()
    lda.w   tmpX + 1        // high byte of x
    sta.b   WRMPYA

    tya                     // y not set above
    sta.b   WRMPYB

    // wait 8 cycles
    rep     #$31            // 3 - clear c
a16()
    lda.w   product32 + 1   // 5
    adc.b   RDMPY           // xh * yl
    sta.w   product32 + 1

    sep     #$20
a8()
    lda.b   #0
    rol
    sta.w   product32 + 3

    lda.w   tmpY + 1
    sta.b   WRMPYB

    // wait 8 cycles
    rep     #$31            // 3 - clear c
a16()
    lda.w   product32 + 2   // 5
    adc.b   RDMPY           // xh * yh
    sta.w   product32 + 2
    tax
    ldy.w   product32

    pld
    plp
    rts
}


// Multiply sint16 * sint16 = sint32
//
// REQUIRES: 16 bit Index, DB access shadow
//
// INPUT:
//  Y: 16 bit factor
//  X: 16 bit factor
//
// OUTPUT:
//  XY: 32 bit product
//  product32: 32 bit product
au()
i16()
code()
function Multiply.s16y_s16x_s32xy {
    cpy.w   #$8000
    bcs     NegY


    // Y is Positive
    cpx.w   #$8000
    bcc     Multiply.u16y_u16x_u32xy


    // Y is Positive, X is Negative
    stx.w   factor32 + 0
    ldx.w   #0xffff
    stx.w   factor32 + 2
    ldx.w   #0x0000
    bra     Multiply.s32_s32xy_s32xy


NegY:
    // Y is Negative
    sty.w   factor32 + 0
    ldy.w   #0xffff
    sty.w   factor32 + 2

    txy
    bpl     NegYPosX


    // Y is Negative, X is Negative
    ldx.w   #0xffff
    bra     Multiply.s32_s32xy_s32xy


NegYPosX:
    // Y is Negative, X is Positive
    ldx.w   #0
    // Multiply.s32_s32xy_s32xy

FallThrough:
}



// Multiply a 32 bit integer by another 32 bit integer.
//
// REQUIRES: 16 bit Index, DB access shadow
//
// INPUT:
//  factor32: 32 bit factor
//  XY: 32 bit factor (Y = loword)
//
// OUTPUT:
//  XY: 32 bit product
//  product32: 32 bit product
au()
i16()
code()
Multiply.u32_s32xy_32xy:
Multiply.s32_u32xy_32xy:
Multiply.s32_s32xy_s32xy:
function Multiply.u32_u32xy_u32xy {
    //      f3 f2 f1 f0
    //  *   Xh Xl Yh Yl
    // ------------------
    //            f1*Yl
    // +       f1*Yl
    // +  c f1*Yl
    // + f1*Xl
    // +       f0*Yh
    // +    f1*Yh
    // + f2*Yh
    // +    f0*Xl
    // + f1*Xl
    // + f0*Xh

    assert(pc() == Multiply.s16y_s16x_s32xy.FallThrough)

constant tmpY = tmp1
constant mathTmp = tmp2
assert(mathTmp + 2 == tmp3)

    php
    phd

    rep     #$31
a16()
    lda.w   #WRMPYA & 0xff00
    tcd

    sep     #$20
a8()
    phx


    sty.w   tmpY
    tya                     // Yl
    bne +
        ldy.w   #0
        sty.w   product32 + 0
        sty.w   product32 + 2
        sep     #$30
        bra     SkipYl
    +

    sta.b   WRMPYA

    lda.w   factor32 + 0
    sta.b   WRMPYB

    // Wait 8 Cycles
    rep     #$21            // 3 - c clear
a16()
    ldy.w   factor32 + 1    // 5
    ldx.w   factor32 + 2
    lda.b   RDMPY           // f0*Yl

    sep     #$10
i8()
    sty.b   WRMPYB

    // Wait 8 cycles
    sta.w   product32 + 0   // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // f1*Yl

    stx.b   WRMPYB

    // Wait 8 cycles
    sta.w   product32 + 1   // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // f2*Yl


    ldx.w   factor32 + 3
    stx.b   WRMPYB

    // Wait 8 cycles
    sta.w   product32 + 2   // 5
    sep     #$20

a8()
    xba
    adc.b   RDMPY           // f3*Yl
    sta.w   product32 + 3


SkipYl:
    // This check is preformed here instead of at start
    // because I would have to load factor32 to XY which wastes cycles
    lda.w   tmpY + 1        // Yh
    beq     SkipYh

    sta.b   WRMPYA

    lda.w   factor32 + 0
    sta.b   WRMPYB

    // Wait 8 Cycles
    rep     #$21            // 3 - c clear
a16()
    ldy.w   factor32 + 1    // 5
    ldx.w   factor32 + 2
    lda.b   RDMPY           // f0*Yh

    sty.b   WRMPYB

    // Wait 8 cycles
    sta.w   mathTmp + 0     // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // f1*Yh

    stx.b   WRMPYB

    // Wait 8 cycles
    sta.w   mathTmp + 1     // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // f2*Yh
    tax

    // Add 2 halves together
    // Could not combine with previous, always caused an off by 1 error
    clc
    lda.w   mathTmp + 0
    adc.w   product32 + 1
    sta.w   product32 + 1

    sep #$20
a8()
    txa
    adc.w   product32 + 3
    sta.w   product32 + 3


SkipYh:
a8()
i8()
    ldy.w   factor32 + 0

    plx
    beq     SkipXl
    stx.b   WRMPYA          // Xl

    sty.b   WRMPYB

    // Wait 8 Cycles
    rep     #$21            // 3 - clear c
a16()

    ldx.w   factor32 + 1    // 5
    lda.b   RDMPY           // f0*Xl

    stx.b   WRMPYB

    // Wait 8 cycles
    sta.w   mathTmp + 0     // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // f1*Xl
    sta.w   mathTmp + 1

    lda.w   mathTmp + 0
    clc
    adc.w   product32 + 2
    sta.w   product32 + 2

SkipXl:
    plx
    beq     SkipXh
    stx.b   WRMPYA          // Xh
    sty.b   WRMPYB

    // Wait 8 cycles
    sep     #$20            // 3
a8()
    lda.w   product32 + 3   // 4
    clc                     // 2
    adc.b   RDMPYL          // f0*Xh
    sta.w   product32 + 3
SkipXh:

    pld
    plp
i16()
    ldy.w   product32
    ldx.w   product32 + 2
    rts
}


// Multiply a 32 bit integer by an 8 bit unsigned integer
//
// REQUIRES: 16 bit Index, DB access shadow
//
// INPUT:
//  XY: 32 bit factor (Y loword)
//  A: 8 bit unsigned factor
//
// OUTPUT:
//  XY: 32 bit product
//  product32: 32 bit product
a8()
i16()
code()
Multiply.s32xy_u8a_s32xy:
function Multiply.u32xy_u8a_u32xy {
    //      Xh Xl Yh Yl
    //  *             A
    // ------------------
    //             A*Yl
    // +        A*Yh
    // +     A*Xl
    // +  A*Xh

constant tmpY = tmp1
constant tmpX = tmp2

    phd

    pea     WRMPYA & 0xff00
    pld

    sta.b   WRMPYA
    tya
    sta.b   WRMPYB

    // Wait 8 Cycles
    rep     #$21            // 3 - c clear
a16()
    sty.w   tmpY            // 5
    stx.w   tmpX
    lda.b   RDMPY           // A*Yl

    sep     #$10
i8()
    ldy.w   tmpY + 1
    sty.b   WRMPYB

    // Wait 8 cycles
    sta.w   product32 + 0   // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // A*Yh

    stx.b   WRMPYB

    // Wait 8 cycles
    sta.w   product32 + 1   // 5
    xba                     // 3
    and.w   #0x00ff
    adc.b   RDMPY           // A*Xl


    ldx.w   tmpX + 1
    stx.b   WRMPYB

    // Wait 8 cycles
    sta.w   product32 + 2   // 5
    rep     #$30            // 3
    sep     #$20
a8()
i16()
    xba
    adc.b   RDMPY           // A*Xl
    sta.w   product32 + 3

    ldy.w   product32
    ldx.w   product32 + 2

    pld
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

