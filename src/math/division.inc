// math/division.inc
// =================
//
// Division Routines.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Math {

code()


// uint16 / uint8 Integer Division
//
// REQUIRES: 8 bit A, 16 bit Index, DB anywhere
//
// INPUT:
//  Y: uint16 dividend
//  A: uint8 divisor
//
// OUTPUT:
//  Y: uint16 result
//  X: uint16 remainder
a8()
i16()
code()
function Divide.u16y_u8a {
    phd
    pea     WRDIV & 0xff00
    pld

    sty.b   WRDIV
    sta.b   WRDIVB  // Load to SNES division registers

    // Wait 16 Cycles
    phb             // 3
    plb             // 4
    phb             // 3
    plb             // 4

    ldy.b   RDDIV   // 1 from instruction fetch
    ldx.b   RDMPY

    pld
    rts
}



// uint16 / sint16 Integer Division
//
// REQUIRES: 16 bit Index, DB anywhere
//
// INPUT:
//  Y: uint16 dividend
//  X: sint16 divisor
//
// OUTPUT:
//  Y: sint16 result
//  X: uint16 remainder (Always positive, Euclidean division)
au()
i16()
code()
function Divide.u16y_s16x {
    cpx.w   #$8000
    bcc     Divide.u16y_u16x

    // X is negative
    php
    rep #$30
a16()

    txa
    neg.w()
    tax

    jsr     Divide.u16y_u16x

    // Result is negative
    tya
    neg.w()
    tay

    plp
    rts
}



// sint16 / sint16 Integer Division
//
// REQUIRES: 16 bit Index, DB anywhere
//
// INPUT:
//  Y: sint16 dividend
//  X: sint16 divisor
//
// OUTPUT:
//  Y: sint16 result
//  X: uint16 remainder (Always positive, Euclidean division)
au()
i16()
code()
function Divide.s16y_s16x {
    cpy.w   #$8000
    bcc     YPos

        php
        rep     #$30
a16()
        // dividend Negative
        tya
        neg.w()
        tay

        cpx.w   #$8000
        bcc     YNegXPos

            // divisor Negative
            txa
            neg.w()
            tax

            jsr     Divide.u16y_u16x

            // Result is positive
            plp
            rts

        YNegXPos:
            // divisor is positive
            jsr     Divide.u16y_u16x

            // Result is negative
            tya
            neg.w()
            tay

            plp
            rts

YPos:
    // Else - dividend is positive

    cpx.w   #$8000
    bcc     Divide.u16y_u16x

    // Y Positive X Negitive

    php
    rep     #$30
a16()
    txa
    neg.w()
    tax

    jsr     Divide.u16y_u16x

    // Result is negative
    tya
    neg.w()
    tay

    plp
    rts
}




// sint16 / uint16 Integer Division
//
// REQUIRES: 16 bit Index, DB anywhere
//
// INPUT:
//  Y: sint16 dividend
//  X: uint16 divisor
//
// OUTPUT:
//  Y: sint16 result
//  X: uint16 remainder (Always positive, Euclidean division)

// This one is here because it is the most common of the signed divisions
au()
i16()
code()
function Divide.s16y_u16x {
    cpy.w   #$8000
    bcc     Divide.u16y_u16x

    // Y is negative

    php
    rep     #$30
a16()

    tya
    neg.w()
    tay

    jsr     Divide.u16y_u16x

    // Result is negative
    tya
    neg.w()
    tay

    plp
    rts
}



// uint16 / uint16 Integer Division
//
// REQUIRES: 16 bit Index, DB anywhere
//
// INPUT:
//  Y: uint16 dividend
//  X: uint16 divisor
//
// OUTPUT:
//  Y: uint16 result
//  X: uint16 remainder

au()
i16()
code()
function Divide.u16y_u16x {
    //  if divisor < 256
    //      calculate using the SNES registers
    //  else
    //      calculate using the 16 bit division algorithm

constant divisor = tmp1

    phd
    php

    cpx.w   #$0100
    bcs     LargeX

        rep     #$30
i16()
a16()
        lda.w   #WRDIV & 0xff00
        tcd

        sty.b   WRDIV

        sep     #$30
i8()
a8()
        stx.b   WRDIVB

        // Wait 16 Cycles
        nop             // 2
        phd             // 4
        pld             // 5
        plp             // 4
i16()
        ldy.b   RDDIV   // 1 from instruction fetch
        ldx.b   RDMPY

        pld
        rts


LargeX:
    // divisor (X) > 256

    // using DP to access divisor saves 2 cycles
    // and allows this scope to be called with DB anywhere
    assertLowRam(divisor)
    assert((divisor & 0xff00) == ((divisor + 1) & 0xff00))

    // ::KUDOS codebase64 wiki for the original algorithm::
    // ::: http://codebase64.org/doku.php?id=base:16bit_division_16-bit_result ::

    // ::KUDOS Ben Eater for the improved division algorithm::
    // ::: Binary to decimal can’t be that hard, right? https://www.youtube.com/watch?v=v3-a-zqKfgA ::
    // (The improved algorithm uses less ROM space and is faster if 5 or more bits in the result are set)

    //  remainder = 0
    //  c = 0
    //
    //  repeat 16 times
    //      remainder << 1 | MSB(dividend)
    //      dividend << 1 | c
    //      c = bool (remainder >= divisor)
    //      if c set:
    //          remainder -= divisor
    //  dividend << 1 | c
    //
    //  result = dividend

    rep #$30
i16()
a16()

    lda.w   #divisor & 0xff00
    tcd

    stx.b   divisor
    ldx.w   #0
    // X = remainder
    clc

    evaluate n = 0
    while {n} < 16 {
        // Y = dividend
        tya
        rol
        tay

        // X = remainder
        txa
        rol
        tax

        sec
        sbc.b   divisor
        bcc     +
            tax
        +

        evaluate n = {n} + 1
    }
    tya
    rol
    tay

    plp
    pld
    rts
}



// sint32 / sint32 Integer Division
//
// INPUT:
//  dividend32: sint32 dividend
//  divisor32: sint32 divisor
//
// OUTPUT:
//  result32: sint32 result
//  remainder32: uint32 remainder (Always positive, Euclidean division)
//
// NOTES:
//  `result32` and `dividend32` share the same memory location
au()
iu()
code()
function Divide.s32_s32 {
    php
    rep     #$30
a16()
i16()
    // code requires these variables to exist in the same page and in bank 0
    assertLowRam(dividend32)
    assert((dividend32 >> 8) == ((remainder32 + 3) >> 8))

    phd
    lda.w   #dividend32 & 0xff00
    tcd

    ldy.b   dividend32 + 2
    bpl     DividendPositive

        // dividend is negative, negate it
        // y is high word of dividend
        lda.b   dividend32
        eor.w   #0xffff
        clc
        adc.w   #1
        sta.b   dividend32
        tya
        eor.w   #0xffff
        adc.w   #0
        sta.b   dividend32 + 2


        ldy.b   divisor32 + 2
        bpl     +
            // divisor is negative, negate it
            // y is high word of divisor
            lda.b   divisor32
            eor.w   #0xffff
            clc
            adc.w   #1
            sta.b   divisor32
            tya
            eor.w   #0xffff
            adc.w   #0
            sta.b   divisor32 + 2

            // result is positive
            bra     Divide.u32_u32.AfterRegisters
        +

        // Else, divisor is positive

        jsr     Divide.u32_u32

        // only 1 negative, result negative
        Negate32.b(result32)

        pld
        plp
        rts


DividendPositive:
    // dividend is positive

    ldy.b   divisor32 + 2
    bpl     Divide.u32_u32.AfterRegisters

    // divisor is negative, negate it
    // y is high word of divisor
    lda.b   divisor32
    eor.w   #0xffff
    clc
    adc.w   #1
    sta.b   divisor32
    tya
    eor.w   #0xffff
    adc.w   #0
    sta.b   divisor32 + 2

    jsr     Divide.u32_u32

    // only 1 negative, result negative
    Negate32.b(result32)

    pld
    plp
    rts
}


// uint32 / uint32 Integer Division
//
// REQUIRES: nothing
//
// INPUT:
//  dividend32: uint32 dividend
//  divisor32: uint32 divisor
//
// OUTPUT:
//  result32: uint32 result
//  remainder32: uint32 remainder
//
// NOTES:
//  `result32` and `dividend32` share the same memory location
au()
iu()
code()
function Divide.u32_u32 {

    // ::KUDOS codebase64 wiki for the original algorithm::
    // ::: http://codebase64.org/doku.php?id=base:16bit_division_16-bit_result ::

    // ::KUDOS Ben Eater for the improved division algorithm::
    // ::: Binary to decimal can’t be that hard, right? https://www.youtube.com/watch?v=v3-a-zqKfgA ::
    // (The improved division algorithm is faster if 3 or more bits in the result are set)

    //  remainder = 0
    //  c = 0
    //
    //  repeat 32 times
    //      remainder << 1 | MSB(dividend)
    //      dividend << 1 | c
    //      c = bool (remainder >= divisor)
    //      if c set:
    //          remainder -= divisor
    //  dividend << 1 | c
    //
    //  result = dividend


    php
    rep #$30
a16()
i16()
    // the code requires these variables to exist in the same page and in bank 0
    assertLowRam(dividend32)
    assert((dividend32 >> 8) == ((remainder32 + 3) >> 8))

    phd
    lda.w   #dividend32 & 0xff00
    tcd

AfterRegisters:

    stz.b   remainder32
    stz.b   remainder32 + 2
    clc

    ldx.w   #32
    -
        rol.b   dividend32
        rol.b   dividend32 + 2
        rol.b   remainder32
        rol.b   remainder32 + 2

        sec
        lda.b   remainder32
        sbc.b   divisor32
        tay
        lda.b   remainder32 + 2
        sbc.b   divisor32 + 2

        bcc     +
            sty.b   remainder32
            sta.b   remainder32 + 2
        +

        dex
        bne     -

    rol.b   dividend32
    rol.b   dividend32 + 2

    pld
    plp
    rts
}



// uint32 / uint8 Integer Division
//
// REQUIRES: DB access lowram
//
// INPUT:
//  dividend32: uint32 dividend
//  A: uint8 divisor
//
// OUTPUT:
//  result32: uint32 result
//  A: uint8 remainder
//
// NOTES:
//  `result32` and `dividend32` share the same memory location
au()
iu()
code()
function Divide.u32_u8a {
    php
    phd

    pea     WRDIV & 0xff00
    pld

    sep     #$30
a8()
i8()
    ldy.w   dividend32 + 3
    sty.b   WRDIVL
    stz.b   WRDIVH

    sta.b   WRDIVB

    // Wait 16 cycles
    phd                     // 4
    pld                     // 5
    nop                     // 2
    ldx.w   dividend32 + 2  // 4 - preload next byte
    ldy.b   RDDIV           // 1 from instruction fetch
    sty.w   result32 + 3


    ldy.b   RDMPY
    stx.b   WRDIVL
    sty.b   WRDIVH

    sta.b   WRDIVB

    // Wait 16 cycles
    phd                     // 4
    pld                     // 5
    nop                     // 2
    ldx.w   dividend32 + 1  // 4 - preload next byte
    ldy.b   RDDIV           // 1 from instruction fetch
    sty.w   result32 + 2


    ldy.b   RDMPY
    stx.b   WRDIVL
    sty.b   WRDIVH

    sta.b   WRDIVB

    // Wait 16 cycles
    phd                     // 4
    pld                     // 5
    nop                     // 2
    ldx.w   dividend32 + 0  // 4 - preload next byte
    ldy.b   RDDIV           // 1 from instruction fetch
    sty.w   result32 + 1


    ldy.b   RDMPY
    stx.b   WRDIVL
    sty.b   WRDIVH

    sta.b   WRDIVB

    // Wait 16 cycles
    phb                     // 3
    plb                     // 4
    phb                     // 3
    plb                     // 4
    ldy.b   RDDIV           // 2 from instruction fetch
    sty.w   result32 + 0

    lda.b   RDMPY           // remainder

    pld
    plp
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

