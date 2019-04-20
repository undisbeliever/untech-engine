// hdma/triangular-window.inc
// ==========================
//
// HDMA Table generator for rendering a triangle to the screen window.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Hdma {


// Builds and enables the HdmaTable for a triangular screen window
//
// NOTE: Only checks if the window is visible in the Y axis
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: all utDpTmp variables unused
//
// INPUT: x0, y0, x1, y1, x2, y2 - triangle coordinates
//          (in utDpTmp, will be modified by this routine)
//
// LIMITS: triangle height < 256
//
// NOTE: If the triangle is too large then an overflow that corrupts the
//       window can occur. An overflow cannot occur if the triangle
//       width and height is < 256.
a16()
i16()
code()
function TriangularWindow {
// triangle coordinates
// (6 * sint16)
constant x0 = utDpTmp0
constant y0 = utDpTmp1
constant x1 = utDpTmp2
constant y1 = utDpTmp3
constant x2 = utDpTmp4
constant y2 = utDpTmp5

constant _tmp = utDpTmp6

assert(_tmp < _WindowBuffer.LAST_DP_TMP)

allocateTmpWord(_bufferPos)

constant _skip = _WindowBuffer.skip
constant _nLines = _WindowBuffer.nLines

    // Sort the triangle so p0 <= p1 <= p2
    //  if y1 < y0 { swap(y1, y0), swap(x1, x0) }
    //  if y2 < y0 { swap(y2, y0), swap(x2, x0) }
    //  if y2 < y1 { swap(y2, y1), swap(x2, x1) }

    lda.b   y1
    cmp.b   y0
    bpl     +
        ldx.b   y0
        sta.b   y0
        stx.b   y1

        ldy.b   x1
        ldx.b   x0
        sty.b   x0
        stx.b   x1
    +
    lda.b   y2
    cmp.b   y0
    bpl     +
        ldx.b   y0
        sta.b   y0
        stx.b   y2
        txa

        ldy.b   x2
        ldx.b   x0
        sty.b   x0
        stx.b   x2
    +
    // A = y2
    cmp.b   y1
    bpl     +
        ldx.b   y1
        sta.b   y1
        stx.b   y2
        txa

        ldy.b   x2
        ldx.b   x1
        sty.b   x1
        stx.b   x2
    +
    // A = y2


    // Initialize HDMA Window Buffer
    //
    // SetupWindowBuffer(y2 - y0 + 1, y0)

    // A = y2
    sec
    sbc.w   y0
    inc
    sta.w   _WindowBuffer.SetupWindowBuffer.height

    lda.b   y0
    sta.b   _WindowBuffer.SetupWindowBuffer.yPos

    jsr     _WindowBuffer.SetupWindowBuffer
    bcc     NotVisible

    stx.w   _bufferPos


    // Determine if the longest line is on the left or right
    //
    //  xm = _WindowBuffer.CalcLineXpos(x0, x2, y2 - y0 + 1, y1 - y0 + 1)
    //
    //  if xm >= x1:
    //      bufferPosLong = bufferPos + 1
    //      bufferPosShort = bufferPos
    //  else:
    //      bufferPosLong = bufferPos
    //      bufferPosShort = bufferPos + 1

    lda.b   x0
    sta.b   _WindowBuffer.DrawLine.x0
    lda.b   x2
    sta.b   _WindowBuffer.DrawLine.x1

    // height set above

    lda.b   y1
    sec
    sbc.b   y0
    inc
    tay
    jsr     _WindowBuffer.CalcLineXpos

    ldx.w   _bufferPos
    txy

    // A = xm
    cmp.b   x1
    bmi     +
        inx
        bra     ++
    +
        iny
    +

    phy

    // Draw long line
    //  _WindowBuffer.DrawLine(x0, x2, y2 - y0 + 1, skip, nLines, bufferPosLong)

    // X = bufferPosLong
    // x0, x1, height set above
    ldy.b   _skip
    lda.b   _nLines
    jsr     _WindowBuffer.DrawLine

    plx


    // Draw short lines
    //
    //  x = bufferPosShort
    //  height = y1 - y0 + 1
    //  nl = height - skip
    //  if nl > 0:
    //      x = _WindowBuffer.DrawLine(x0, x1, height, skip, nl, x)
    //  else:
    //      nl = 0
    //
    //  nl = nLines - nl
    //  if nl > 0:
    //      s = skip - height
    //      if s > 0:
    //          s = 0
    //      _WindowBuffer.DrawLine(x1, x2, y2 - y1 + 1, s, nl, x)

    // X = bufferPosShort

    lda.b   y1
    sec
    sbc.b   y0
    inc
    sta.b   _WindowBuffer.DrawLine.height
    sec
    sbc.b   _skip
    bcc     SkipTopShortLine
        sta.b   _tmp

        // a = number of lines to draw
        ldy.b   x0
        sty.b   _WindowBuffer.DrawLine.x0
        ldy.b   x1
        sty.b   _WindowBuffer.DrawLine.x1

        ldy.b   _skip
        jsr     _WindowBuffer.DrawLine

        bra     EndTopShortLine

SkipTopShortLine:
        stz.b   _tmp

EndTopShortLine:

    lda.b   _nLines
    sec
    sbc.b   _tmp
    bcc     SkipBottomShortLine
    beq     SkipBottomShortLine
        sta.b   _tmp

        lda.b   _skip
        sec
        sbc.b   _WindowBuffer.DrawLine.height
        bpl     +
            lda.w   #0
        +
        tay

        lda.b   y2
        sec
        sbc.b   y1
        inc
        sta.b   _WindowBuffer.DrawLine.height

        lda.b   x1
        sta.b   _WindowBuffer.DrawLine.x0
        lda.b   x2
        sta.b   _WindowBuffer.DrawLine.x1

        lda.b   _tmp
        jsr     _WindowBuffer.DrawLine

SkipBottomShortLine:


    // Calculate the end of the hdma buffer
    //  c = nLines > 127
    //  end = bufferPos + nLines * 2 + c

    lda.b   _nLines
    cmp.w   #127 + 1
    rol
    // c clear
    adc.w   _bufferPos
    tax


NotVisible:
    jmp     _WindowBuffer.FinalizeBuffer
}



// Builds and enables the HdmaTable for a screen window containing a
// equilateral triangle rotated by a given angle.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: all utDpTmp variables unused
// REQUIRES: not in Mode 7
//
// INPUT: xPos, yPos - location of the centre of the triangle
//        A - triangle angle (256 = 1 revolution)
//        Y - triangle diameter
//
// LIMITS: Y < 256
//
// NOTE: xPos/yPos are overwritten by this routine
a16()
i16()
code()
function RotatedTriangularWindow {

constant xPos = TriangularWindow.x0
constant yPos = TriangularWindow.y0

constant _angle = TriangularWindow._tmp
allocateTmpWord(_pos)

constant DEG_90  = SineTable.size / 4
constant DEG_120 = SineTable.size / 3

    sta.b   _angle

    // DP access registers
    lda.w   #$2100
    tcd

    sep     #$30
a8()
i8()
    sty.b   M7A
    stz.b   M7A

    rep     #$30
a16()
i16()

    // Calculate triangle coordinates

    lda.w   xPos
    sta.w   _pos

    lda.w   #DEG_90
    jsr     _CalcPos
    sta.w   TriangularWindow.x0

    lda.w   #DEG_120 + DEG_90
    jsr     _CalcPos
    sta.w   TriangularWindow.x1

    lda.w   #DEG_120 * 2 + DEG_90
    jsr     _CalcPos
    sta.w   TriangularWindow.x2


    lda.w   yPos
    sta.w   _pos

    lda.w   #0
    jsr     _CalcPos
    sta.w   TriangularWindow.y0

    lda.w   #DEG_120
    jsr     _CalcPos
    sta.w   TriangularWindow.y1

    lda.w   #DEG_120 * 2
    jsr     _CalcPos
    sta.w   TriangularWindow.y2


    // restore DP
    lda.w   #0
    tcd

    jmp     TriangularWindow


// INPUT:   A = angle offset
//     _angle = angle
//       _pos = position
//        M7A = diameter
// OUTPUT: sin(A + angle) * diameter + position
// DP access registers
a16()
i16()
_CalcPos:
    assert(SineTable.size == 256)

    sep     #$20
a8()
    clc
    adc.w   _angle
    tax

    lda.l   SineTable,x
    sta.b   M7B

    rep     #$31
a16()
    // c clear
    lda.b   MPYM
    adc.w   _pos
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

