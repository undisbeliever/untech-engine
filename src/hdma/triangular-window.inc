// hdma/triangular-window.inc
// ==========================
//
// HDMA Table generator for rendering a triangle to the screen window.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Hdma {


// Builds and enables the HdmaTable for a triangular screen window
//
// NOTE: Only checks if the window is visible in the Y axis
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: all dpTmp variables unused
//
// INPUT: x0, y0, x1, y1, x2, y2 - triangle coordinates
//          (in dpTmp, will be modified by this routine)
//
// LIMITS: triangle height < 256
a16()
i16()
code()
function TriangularWindow {
// triangle coordinates
// (6 * sint16)
constant x0 = dpTmp0
constant y0 = dpTmp1
constant x1 = dpTmp2
constant y1 = dpTmp3
constant x2 = dpTmp4
constant y2 = dpTmp5

constant _tmp = dpTmp6

assert(_tmp < _WindowLine.LAST_DP_TMP)

allocateTmpWord(_bufferPos)

constant _skip = _WindowLine.skip
constant _nLines = _WindowLine.nLines

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
    sta.w   _WindowLine.SetupWindowBuffer.height

    lda.b   y0
    sta.b   _WindowLine.SetupWindowBuffer.yPos

    jsr     _WindowLine.SetupWindowBuffer
    bcc     NotVisible

    stx.w   _bufferPos


    // Determine if the longest line is on the left or right
    //
    //  xm = _WindowLine.CalcLineXpos(x0, x2, y2 - y0 + 1, y1 - y0 + 1)
    //
    //  if xm >= x1:
    //      bufferPosLong = bufferPos + 1
    //      bufferPosShort = bufferPos
    //  else:
    //      bufferPosLong = bufferPos
    //      bufferPosShort = bufferPos + 1

    lda.b   x0
    sta.b   _WindowLine.DrawLine.x0
    lda.b   x2
    sta.b   _WindowLine.DrawLine.x1

    // height set above

    lda.b   y1
    sec
    sbc.b   y0
    inc
    tay
    jsr     _WindowLine.CalcLineXpos

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
    //  _WindowLine.DrawLine(x0, x2, y2 - y0 + 1, skip, nLines, bufferPosLong)

    // X = bufferPosLong
    // x0, x1, height set above
    ldy.b   _skip
    lda.b   _nLines
    jsr     _WindowLine.DrawLine

    plx


    // Draw short lines
    //
    //  x = bufferPosShort
    //  height = y1 - y0 + 1
    //  nl = height - skip
    //  if nl > 0:
    //      x = _WindowLine.DrawLine(x0, x1, height, skip, nl, x)
    //  else:
    //      nl = 0
    //
    //  nl = nLines - nl
    //  if nl > 0:
    //      s = skip - height
    //      if s > 0:
    //          s = 0
    //      _WindowLine.DrawLine(x1, x2, y2 - y1 + 1, s, nl, x)

    // X = bufferPosShort

    lda.b   y1
    sec
    sbc.b   y0
    inc
    sta.b   _WindowLine.DrawLine.height
    sec
    sbc.b   _skip
    bcc     SkipTopShortLine
        sta.b   _tmp

        // a = number of lines to draw
        ldy.b   x0
        sty.b   _WindowLine.DrawLine.x0
        ldy.b   x1
        sty.b   _WindowLine.DrawLine.x1

        ldy.b   _skip
        jsr     _WindowLine.DrawLine

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
        sbc.b   _WindowLine.DrawLine.height
        bpl     +
            lda.w   #0
        +
        tay

        lda.b   y2
        sec
        sbc.b   y1
        inc
        sta.b   _WindowLine.DrawLine.height

        lda.b   x1
        sta.b   _WindowLine.DrawLine.x0
        lda.b   x2
        sta.b   _WindowLine.DrawLine.x1

        lda.b   _tmp
        jsr     _WindowLine.DrawLine

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
    jmp     _WindowLine.FinalizeBuffer
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

