// hdma/_window-line.inc
// =====================
//
// HDMA Window Line drawing algorithm.
//
// This code is based on Bresenham's line drawing algorithm
//
// NOTE: This code uses dpTmp10 to dpTmp19 to store state. Ensure that all
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

// Height of the line
// (uint16 - dp)
constant _height = dpTmp17

// Parameters for Bresenham's line drawing algorithm
// (3 x uint16 - dp)
constant _dx2 = dpTmp16
constant _dy2 = dpTmp15
constant _err = dpTmp14

// The last scanline that is drawn
// (uint16 - dp)
constant _lastScanline = dpTmp13

// Address in the buffer of the last scanline of the current line
// (word ptr - dp)
constant _bufferLast = dpTmp12

// Address of the second HDMA count byte in the buffer
// (word ptr - dp)
constant _bufferSecondCount = dpTmp11

// Address of the next break in the line drawing loop
// (word ptr - dp)
constant _bufferTest = dpTmp10

constant LAST_DP_TMP = dpTmp10


// Draws a single line to a single side of the HDMA window buffer.
//
// The HDMA window buffer MUST:
//  * contain both the left and right window registers
//  * be in continuous mode
//
// This routine will skip over the second HDMA count byte.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: DrawLine.x0 - top x coordinate of the line
//        DrawLine.x1 - bottom x coordinate of the line
//        DrawLine.height - height of the line
//        DrawLine.bufferSecondCount - address of the second HDMA count byte
//                                     (`start of line buffer + 127 * 2`)
//        Y - number of scanlines to skip
//        A - number of scanlines to draw
//        X - address of the HDMA buffer
//
// OUTPUT: X - address of the HDMA buffer after final scanline
//
// KEEP: height is not modified by this routine
//       bufferSecondCount is not modified by this routine
//
// LIMITS: abs(x0 - x1) < 0x8000
//         abs(x0 - x1) * skip < 0x10000
//         height > 0
//         height < 256
//         skip < 256
//         nScanlines > 0
//         nScanlines + skip <= height
a16()
i16()
code()
function DrawLine {
// Reuse variables to reduce load/stores
constant x0 = _xPos
constant x1 = _x1
constant height = _height
constant bufferSecondCount = _bufferSecondCount

    //  xPos = x0
    //  x1 = x1
    //  height = height
    //
    //  lastScanline = nScanlines
    //  bufferLast = buffer + (nScanlines - 1) * 2
    //
    //  dy2 = height * 2
    //  dx2 = abs(x1 - x0) * 2
    //
    //  err = dx2 / 2 - 1
    //  if err < height:    // (signed comparison)
    //      err = height
    //
    //  if skip > 0:
    //      lastScanline += skip
    //      err += dx2 * skip
    //
    //  if err >= dy2:
    //    err = err % dy2
    //    div = err / dy2
    //
    //    if xPos < x1:
    //        xPos += div
    //    else:
    //        xPos -= div
    //
    //  if buffer < bufferSecondCount and bufferSecondCount < bufferLast:
    //      bufferTest = bufferSecondCount
    //  else:
    //      bufferTest = bufferLast
    //
    //  if dx2 >= dy2:
    //      return _DrawShallowLine()
    //  else:
    //      return _DrawSteepLine()


    sta.b   _lastScanline

    dec
    asl
    sta.b   _bufferLast
    txa
    clc
    adc.b   _bufferLast
    sta.b   _bufferLast

    lda.b   _height
    asl
    sta.b   _dy2


    lda.b   _x1
    sec
    sbc.b   x0
    bpl     +
        eor.w   #0xffff
        inc
    +
    asl
    sta.b   _dx2

    lsr
    dec
    bmi     +
    cmp.b   _height
    bcs     ++
    +
        lda.b   _height
    +
    sta.b   _err

    tya
    beq     +
        clc
        adc.b   _lastScanline
        sta.b   _lastScanline

        stx.b   _bufferTest

        sep     #$20
    a8()
        tya
        ldy.b   _dx2
        jsr     Math.Multiply.u16y_u8a_u16y

        rep     #$20
    a16()
        tya
        clc
        adc.b   _err
        sta.b   _err

        ldx.b   _bufferTest
    +


    lda.b   _err
    cmp.b   _dy2
    bcc     SkipDy2Division
        // since dy2 is equal to `height * 2` and height < 256
        // we can optimize the code into a single 16 by 8 bit division
        //
        //      c = A & 1
        //      err = ((A / 2) % height) * 2 + c
        //      div = (A / 2) / height

        lsr                 // MUST NOT edit carry until rol
        sta.l   WRDIV

        sep     #$20
    a8()
        lda.b   _height
        sta.l   WRDIVB      // Wait 16 cycles
        nop                 // 2
        nop                 // 2
        nop                 // 2
        ldy.b   _xPos       // 4

        rep     #$30        // 3
    a16()
        lda.l   RDMPY       // 3
        rol                 // A * 2 + c
        sta.b   _err

        tya
        cmp.b   _x1
        bpl     +
            clc
            adc.l   RDDIV
            bra     ++
        +
            sec
            sbc.l   RDDIV
        +
        sta.b   _xPos
    SkipDy2Division:


    cpx.b   _bufferSecondCount
    bcs     +
        lda.b   _bufferSecondCount
        cmp.b   _bufferLast
        bcc     ++
    +
        lda.b   _bufferLast
    +
    sta.b   _bufferTest


    lda.b   _dx2
    cmp.b   _dy2
    bcc     +
        jmp     _DrawShallowLine
    +

_DrawSteepLine_Followthrough:
}


// Draw a steep Line (dx2 < dy2) to the HDMA Buffer
//
// INTERNAL: called by DrawLine
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X - address of the HDMA buffer
// OUTPUT: X - address of the HDMA buffer after final scanline
a16()
i16()
code()
function _DrawSteepLine {
    assert(pc() == DrawLine._DrawSteepLine_Followthrough)

    //  sx = x1 >= xPos ? 1 : -1
    //
    //  if dx == 0:
    //      h = 0xffff
    //      goto SetXPos
    //
    //  while True:
    //      h = 0
    //      repeat:
    //          h++
    //          err += dx2
    //      until err >= dy2
    //      err -= dy2
    //
    //    SetXPos:
    //      xClipped = bound(xPos, 0, 255)
    //
    //      repeat h times:
    //          if buffer >= bufferTest:
    //              if buffer >= bufferLast:
    //                  goto EndLoop
    //              else:
    //                  buffer++
    //                  bufferTest = bufferLast
    //
    //          *buffer = xClipped
    //          buffer += 2
    //
    //      xPos += sx
    //
    //  EndLoop:
    //
    //  if lastScanline == height:
    //      xClipped = bound(x1, 0, 255)
    //
    //  *buffer = xClipped
    //  buffer += 2
    //  return buffer

    inline _ProcessSx(sx) {
        assert16a()
        assert16i()

        if {sx} > 0 {
            define l = PostitiveSlope_
            define addSxToXpos = inc.b  _xPos
        } else {
            define l = NegativeSlope_
            define addSxToXpos = dec.b  _xPos
        }

        lda.b   _dx2
        bne     +
            ldy.w   #0xffff
            bra     {l}SetXPos
        +

        lda.b   _err

        {l}Loop:
            // A = err
            // X = bufferPos

            // This is skipped when dx == 0
            ldy.w   #0
            clc
            -
                iny
                // c clear
                adc.b   _dx2
                cmp.b   _dy2
                bcc     -
            // c set
            sbc.b   _dy2
            sta.b   _err

        {l}SetXPos:
            // Y = height of xPos segment

            lda.b   _xPos
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
                cpx.b   _bufferTest
                bcs     {l}TestBufferPos
            {l}ContinueLoop:

                sta.w   0,x
                inx
                inx

                dey
                bne     -

            rep     #$30
        a16()
            {addSxToXpos}

            lda.b   _err

            bra     {l}Loop


    {l}TestBufferPos:
    a8()
        cpx.b   _bufferLast
        bcs     EndLoop

        // skip over second count byte
        inx

        phx
        ldx.b   _bufferLast
        stx.b   _bufferTest
        plx

        bra     {l}ContinueLoop
    }


    lda.b   _x1
    cmp.b   _xPos
    bmi     NegativeSlope
        _ProcessSx(1)

    NegativeSlope:
    a16()
        _ProcessSx(-1)

    EndLoop:
a8()
    // A = xClipped
    ldy.b   _lastScanline
    cpy.b   _height
    bcc     NotEndOfLine
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
    NotEndOfLine:

    sta.w   0,x
    inx
    inx

    rep     #$30
a16()
    rts
}


// Draw a shallow (dx2 >= dy2) Line to the HDMA Buffer
//
// INTERNAL: called by DrawLine
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X - address of the HDMA buffer
// OUTPUT: X - address of the HDMA buffer after final scanline
a16()
i16()
code()
function _DrawShallowLine {
    //  sx = x1 >= xPos ? 1 : -1
    //
    //  while True:
    //      if buffer >= bufferTest:
    //          if buffer >= bufferLast:
    //              goto EndLoop
    //          else:
    //              buffer++
    //              bufferTest = bufferLast
    //
    //      *buffer = bound(xPos, 0, 255)
    //      buffer += 2
    //
    //      err += dx2
    //
    //      repeat:
    //          xPos += sx
    //          err -= dy2
    //      until err < dy2
    //
    //  EndLoop:
    //
    //  if lastScanline == height:
    //      *buffer = bound(x1, 0, 255)
    //  else:
    //      *buffer = bound(xPos, 0, 255)
    //
    //  buffer += 2
    //  return buffer

    inline _ProcessSx(sx) {
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

        ldy.b   _err

        {l}Loop:
            // Y = err
            // X = bufferPos

            cpx.b   _bufferTest
            bcs     {l}TestBufferPos
        {l}ContinueLoop:

            // clip xPos to between 0 and 255
            lda.b   _xPos
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
            sta.w   0,x
            inx
            inx


            rep     #$31
        a16()
            tya
            // c clear
            adc.b   _dx2

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
            tay

            bra     {l}Loop


    {l}TestBufferPos:
    a8()
        cpx.b   _bufferLast
        bcs     EndLoop

        // skip over second count byte
        inx

        phx
        ldx.b   _bufferLast
        stx.b   _bufferTest
        plx

        bra     {l}ContinueLoop
    }


    lda.b   _x1
    cmp.b   _xPos
    bmi     NegativeSlope
        _ProcessSx(1)

    NegativeSlope:
    a16()
        _ProcessSx(-1)

    EndLoop:
a16()
    ldy.b   _lastScanline
    cpy.b   _height
    bcs     ClipX1
        lda.b   _xPos
        bra     ClipA

    ClipX1:
        lda.b   _x1

    ClipA:
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

