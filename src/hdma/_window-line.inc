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

// The number of scanlines remaining in the DrawLine routine
// (uint8 - dp)
constant _count = dpTmp12

constant _tmp = dpTmp11

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
    //  dy2 = height * 2
    //
    //  slope = (x1 - x0)
    //  dx2 = abs(x1 - x0) * 2
    //
    //  err = dx2 / 2 - 1
    //  if err < 0:
    //      err = 0
    //  else if err < height:
    //      err = height
    //
    //  if skip > 0:
    //      scanlinesLeft -= skip
    //      err += dx2 * skip

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

    asl
    dec
    bpl     +
        lda.w   #0
        bra     ++
    cmp.w   _scanlinesLeft
    bcs     +
        lda.w   _scanlinesLeft
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
    //  if err >= dy2: goto ShiftXpos
    //
    //  while True:
    //      xClipped = bound(xPos, 0, 255)
    //
    //      repeat:
    //          count--
    //          if count == 0: goto EndLoop
    //
    //          buffer[x] = xClipped
    //          x += 2
    //
    //          err += dx2
    //      until err >= dy2
    //
    //    ShiftXPos:
    //      if slope positive:
    //          repeat:
    //              xPos++
    //              err -= dy2
    //          until err < dy2
    //      else:
    //          repeat:
    //              xPos--
    //              err -= dy2
    //          until err < dy2
    //  EndLoop:
    //
    //  if scanlinesLeft <= 0:
    //      xClipped = bound(x1, 0, 255)
    //
    //  buffer[x] = xClipped
    //  x += 2
    //  return x

    sta.b   _count
    eor.w   #0xffff
    sec
    adc.b   _scanlinesLeft
    sta.b   _scanlinesLeft

    ldy.b   _err
    tya
    cmp.b   _dy2
    bcs     ShiftXpos

    lda.b   _xPos

    Loop:
        // A = xPos
        // Y = err
        // X = bufferPos

        // clip xPos to between 0 and 255
        cmp.w   #0x8000
        bcc     +
            lda.w   #0
            bra     ++
        +
            cmp.w   #256
            bcc     +
                lda.w   #255
        +

        sta.b   _tmp

        -
            sep     #$20
        a8()
            lda.b   _tmp
            // A = xPos clipped
            dec.b   _count
            beq     EndLoop

            sta.w   0,x
            inx
            inx

            rep     #$31
        a16()
            tya
            // c clear
            adc.b   _dx2
            tay
            cmp.b   _dy2
            bcc     -


    ShiftXpos:
        // A = err
        stx.b   _tmp
        ldx.b   _xPos
        sec

        bit.b   _slope
        bmi     SlopeNeg
        SlopePos:
            -
                inx
                // c set
                tay
                sbc.b   _dy2
                bcs     -
            dex
            bra     EndIf

        SlopeNeg:
            -
                dex
                // c set
                tay
                sbc.b   _dy2
                bcs     -
            inx
        EndIf:

        stx.b   _xPos
        txa

        ldx.b   _tmp
        bra     Loop

    EndLoop:
a8()

    sty.b   _err

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

