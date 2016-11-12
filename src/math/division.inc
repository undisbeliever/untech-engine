
arch snes.cpu

scope Math {

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
scope Divide.u16y_u8a: {
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
//  X: uint16 remainder (Always positive, Euclidian division)
au()
i16()
code()
scope Divide.u16y_s16x: {
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
//  X: uint16 remainder (Always positive, Euclidian division)
au()
i16()
code()
scope Divide.s16y_s16x: {
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
//  X: uint16 remainder (Always positive, Euclidian division)

// This one is here because it is the most common of the signed divisions
au()
i16()
code()
scope Divide.s16y_u16x: {
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
scope Divide.u16y_u16x: {
    // Inspiration: http://codebase64.org/doku.php?id=base:16bit_division_16-bit_result

    // if divisor < 256
    //  calculate using SNES registers
    // else
    //  remainder = 0
    //  repeat 16 times
    //      remainder << 1 | MSB(dividend)
    //      dividend << 1
    //      if (remainder >= divisor)
    //          remainder -= divisor
    //          result++

constant divisor(tmp1)

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
    // X > 256
    rep #$30
i16()
a16()
    // using DP to access divisor saves 2 cycles
    // and allows this scope to be called with DB anywhere

    assert((divisor & 0xff00) == ((divisor + 1) & 0xff00))

    lda.w   #divisor & 0xff00
    tcd

    stx.b   divisor
    ldx.w   #0          // Remainder

    macro repeat(evaluate n) {
        if {n} > 0 {
            tya         // Dividend / result
            asl
            tay
            txa         // Remainder
            rol
            tax

            sec
            sbc.b   divisor
            bcc     +
                // C set if positive
                tax
                iny     // Result
            +

            repeat({n} - 1)
        }
    }
    repeat(16)

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
//  remainder32: uint32 remainder (Always positive, Euclidian division)
//
// NOTES:
//  `result32` and `dividend32` share the same memory location
au()
iu()
code()
scope Divide.s32_s32: {
    php
    rep     #$30
a16()
i16()
    // code requires these variables to exist in the same page
    assert((dividend32 >> 8) == ((remainder32 + 3) >> 8))

    phd
    lda.w   #dividend32 & 0xff00
    tcd

    ldy.b   dividend32 + 2
    bpl     DividendPositive

        // negate dividend
        // y is high work of dividend
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
            // y is high work of divisor
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

    // divisor is negative
    // y is high work of divisor

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
scope Divide.u32_u32: {

    // Inspiration: http://codebase64.org/doku.php?id=base:16bit_division_16-bit_result

    // remainder = 0
    // Repeat 32 times:
    //  remainder << 1 | MSB(dividend)
    //  dividend << 1
    //  if (remainder >= divisor)
    //      remainder -= divisor
    //      result++

    php
    rep #$30
a16()
i16()
    // code requires these variables to exist in the same page
    assert((dividend32 >> 8) == ((remainder32 + 3) >> 8))

    phd
    lda.w   #dividend32 & 0xff00
    tcd

AfterRegisters:

    stz.b   remainder32
    stz.b   remainder32 + 2

    ldx.w   #32
    -
        asl.b   dividend32
        rol.b   dividend32 + 2
        rol.b   remainder32
        rol.b   remainder32 + 2

        lda.b   remainder32
        sec
        sbc.b   divisor32
        tay
        lda.b   remainder32 + 2
        sbc.b   divisor32 + 2

        bcc     +
            sty.b   remainder32
            sta.b   remainder32 + 2
            inc.b   result32        // result32 = dividend32, no overflow possible
        +

        dex
        bne     -

    pld
    plp
    rts
}



// uint32 / uint8 Integer Division
//
// REQUIRES: DB access shadow
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
scope Divide.u32_u8a: {
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

