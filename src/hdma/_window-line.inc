// hdma/_window-line.inc
// =====================
//
// HDMA Window Line drawing algorithm.
//
// This code is based on Bresenham's line drawing algorithm
//
// NOTE: This code uses dpTmp11 to dpTmp19 to store state. Ensure that all
// calling code does not use these variables.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Hdma {
namespace _WindowLine {

// current x coordinate of the line
// (sint16 - dp)
constant _xPos = dpTmp19

// final x coordinate of the line
// (sint16 - dp)
constant _x1 = dpTmp18

// Number of scanlines left to draw until the end of the line
// (uint16 - dp)
constant _scanlinesLeft = dpTmp17

// Parameters for Bresenham's line drawing algorithm
// (3 x uint16 - dp)
constant _dx2 = dpTmp16
constant _dy2 = dpTmp15
constant _err = dpTmp14

// if MSB is set then the slope is negative
// (msb flag word - dp)
constant _slope = dpTmp13

// Remaining height for the current xPos value
//
// If 0 then the height will be calculated at the start of a DrawLine
// without changing the current xPos.
//
// MUST BE set to > 0x8000 if the line is vertical.
//
// (uint16 - dp)
constant _remainingHeight = dpTmp12

// The number of scanlines remaining in the DrawLine routine
// (uint8 - dp)
constant _count = dpTmp11

constant LAST_DP_TMP = dpTmp11


// Initializes the HDMA Window line drawing algorithm
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
//
// INPUT: SetupLine.x0 - top x coordinate of the line
//        SetupLine.x1 - bottom x coordinate of the line
//        A - height of the line
//        X - number of scanlines to skip
//
// LIMITS: abs(x0 - x1) < 0x8000
//         abs(x0 - x1) * skip < 0x10000
//         skip < 256
a16()
i16()
code()
function SetupLine {
// Reuse variables to reduce load/stores
constant x0 = _xPos
constant x1 = _x1

    //  xPos = x0
    //  x1 = x1
    //  scanlinesLeft = height
    //
    //  remainingHeight = 0
    //
    //  dy2 = height * 2
    //
    //  slope = (x1 - x0)
    //  dx2 = abs(x1 - x0) * 2
    //
    //  if dx2 == 0:
    //      remainingHeight = 0xffff
    //
    //  err = dx2 / 2 - 1
    //  if err < height:    // (signed comparison)
    //      err = height
    //
    //  if skip > 0:
    //      scanlinesLeft -= skip
    //      err += dx2 * skip

    stz.b   _remainingHeight

    sta.b   _scanlinesLeft
    asl
    sta.b   _dy2

    lda.b   _x1
    sec
    sbc.b   x0
    sta.b   _slope
    bpl     +
        eor.w   #0xffff
        inc
    +
    asl
    sta.b   _dx2

    bne     +
        dec
        sta.b   _remainingHeight
        inc
    +

    lsr
    dec
    bmi     +
    cmp.b   _scanlinesLeft
    bcs     ++
    +
        lda.b   _scanlinesLeft
    +
    sta.b   _err

    cpx.w   #0
    beq     +
        txa
        eor.w   #0xffff
        sec
        adc.b   _scanlinesLeft
        sta.b   _scanlinesLeft

        sep     #$20
    a8()
        ldy.b   _dx2
        txa
        jsr     Math.Multiply.u16y_u8a_u16y

        rep     #$20
    a16()
        tya
        clc
        adc.b   _err
        sta.b   _err
    +

    rts
}


// Draws a single line to a single side of the HDMA window buffer.
//
// This routine can be called sequentially.
//
// The HDMA window buffer MUST:
//  * contain both the left and right window registers
//  * be in continuous mode with the length already set
//
// This routine MUST NOT CROSS a continuous HDMA boundary.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: A - number of scanlines to draw (MUST BE <= 0x7f)
//        X - address of HDMA buffer
//
// LIMITS: A <= 0x7f
// ASSUMES: A <= _scanlinesLeft
//
// OUTPUT: X - address HDMA buffer after
a16()
i16()
code()
function DrawLine {

    //  scanlinesLeft -= count
    //  sx = slope > 0 ? 1 : -1
    //
    //  h = remainingHeight
    //  if h != 0: goto Resume
    //
    //  if err >= dy2: goto ShiftXpos
    //
    //  while True:
    //      h = 0
    //      repeat:
    //          h++
    //          err += dx2
    //      until err >= dy2
    //
    //    Resume:
    //      xClipped = bound(xPos, 0, 255)
    //
    //      repeat:
    //          count--
    //          if count == 0: goto EndLoop
    //
    //          buffer[x] = xClipped
    //          x += 2
    //
    //          h--
    //      until h == 0
    //
    //    ShiftXPos:
    //      repeat:
    //          xPos += sx
    //          err -= dy2
    //      until err < dy2
    //  EndLoop:
    //
    //  remainingHeight = h
    //
    //  if scanlinesLeft <= 0:
    //      xClipped = bound(x1, 0, 255)
    //
    //  buffer[x] = xClipped
    //  x += 2
    //  return x

    inline _Loop(sx) {
        assert16a()
        assert16i()

        if {sx} > 0 {
            define l = PostitiveSlope_
            define addSXtoY = iny
            define takeSXfromY = dey
        } else {
            define l = NegativeSlope_
            define addSXtoY = dey
            define takeSXfromY = iny
        }

        ldy.b   _remainingHeight
        bne     {l}Resume

        lda.b   _err
        cmp.b   _dy2
        bcs     {l}ShiftXpos

        {l}Loop:
            // A = err
            // X = bufferPos

            // do not have to worry about _dx2 == 0
            // when _dx2 == 0 _remainingHeight is large and this loop is skipped
            ldy.w   #0
            clc
            -
                iny
                // c clear
                adc.b   _dx2
                cmp.b   _dy2
                bcc     -
            sta.b   _err

        {l}Resume:
            // Y = height of xPos segment
            // clip xPos to between 0 and 255
            lda.w   _xPos
            bpl     +
                lda.w   #0
                bra     ++
            +
                cmp.w   #256
                bcc     +
                    lda.w   #255
            +

            sep     #$20
        a8()
            -
                dec.b   _count
                beq     EndLoop

                sta.w   0,x
                inx
                inx

                dey
                bne     -

            rep     #$30
        a16()

            lda.b   _err

        {l}ShiftXpos:
            // A = err
            ldy.b   _xPos
            sec
            -
                {addSXtoY}
                // c set
                sbc.b   _dy2
                bcs     -
            {takeSXfromY}
            // c clear
            adc.b   _dy2

            sty.b   _xPos
            bra     {l}Loop
    }


    sta.b   _count
    eor.w   #0xffff
    sec
    adc.b   _scanlinesLeft
    sta.b   _scanlinesLeft

    lda.b   _slope
    bmi     NegativeSlope
        _Loop(1)

    NegativeSlope:
        _Loop(-1)

    EndLoop:
a8()
    // y = remaining height for the current xPos
    sty.b   _remainingHeight

    // A = xClipped
    ldy.b   _scanlinesLeft
    beq     LastScanline
    bpl     NotLastScanline
    LastScanline:
        // clip x1
        ldy.b   _x1
        bpl     +
            lda.b   #0
            bra     ++
        +
            tya
            cpy.w   #256
            bcc     +
                lda.b   #255
        +
    NotLastScanline:

    sta.w   0,x
    inx
    inx

    rep     #$30
a16()
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

