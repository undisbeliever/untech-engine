// hdma/_window-line.inc
// =====================
//
// HDMA Window Line drawing algorithm.
//
// This code is based on Bresenham's line drawing algorithm
//
// NOTE: This code uses dpTmp8 to dpTmp19 to store state. Ensure that all
// calling code does not use these variables.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Hdma {
namespace _WindowLine {

// Current HDMA window buffer
//   First `_HdmaIndirectTableOffset` bytes = window settings buffer
//      odd indexes = left window settings
//      even indexes = right window settings
//
//   Rest of buffer = HDMA Indirect Table
//
// (word ptr - dp)
constant _hdmaBuffer = dpTmp19

// Offset (in bytes) between the window buffer the HDMA Indirect Table
constant _HdmaIndirectTableOffset = (Camera.DISPLAY_HEIGHT + 2) * 2

// Number of lines from the top of the window to skip (offscreen)
// (uint16 - dp)
constant skip = dpTmp18

// Number of visible lines in the window
// This value is always <= 244.
// (uint16 - dp)
constant nLines = dpTmp17



// current x coordinate of the line
// (sint16 - dp)
constant _xPos = dpTmp16

// final x coordinate of the line
// (sint16 - dp)
constant _x1 = dpTmp15

// Height of the line or window
// (uint16 - dp)
constant _height = dpTmp14

// Parameters for Bresenham's line drawing algorithm
// (3 x uint16 - dp)
constant _dx2 = dpTmp13
constant _dy2 = dpTmp12
constant _err = dpTmp11

// The last scanline of the current DrawLine call
// (uint16 - dp)
constant _lastScanline = dpTmp10

// Address in the buffer of the last scanline of the current line
// (word ptr - dp)
constant _bufferLast = dpTmp9

// Temporary variable
// (word)
constant _tmp = dpTmp8

constant LAST_DP_TMP = dpTmp8


// Window settings when no line is drawn
// (word, bank 0x7e)
allocate(noLineWindow, wram7e, 2)
assert(noLineWindow >> 16 == buffer1 >> 16)
assert(noLineWindow >> 16 == buffer2 >> 16)


namespace SetupWindowBuffer {
NotVisible:
    //  iTable[0] = 1
    //  iTable[1 + 2] = noLineWindow
    //  iTable[4] = 0
    //
    //  return hdmaBuffer, false

    // X = hdmaBuffer

    lda.w   #1
    sta.w   _HdmaIndirectTableOffset + 0,x
    lda.w   #noLineWindow
    sta.w   _HdmaIndirectTableOffset + 1,x
    lda.w   #0
    sta.w   _HdmaIndirectTableOffset + 3,x

    clc
    rts
}


// Setup and initialize a continuous HDMA window buffer.
//
// NOTE: This routine does not enable the buffer.
//       You must call `FinalizeBuffer` to enable the buffer during the
//       next VBlank, even if there is nothing on screen.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: SetupWindowBuffer.height - height of the effect
//        SetupWindowBuffer.yPos - topmost position of the window
//
// OUTPUT: carry set if window visible on screen
//         X - HDMA window buffer
//         nLines - number of lines to draw
//         skip - number of lines to skip when drawing the window
//         DrawLine.height = height
//
// NOTE: SetupWindowBuffer.yPos is temporary and overridden in DrawLine
a16()
i16()
code()
function SetupWindowBuffer {
assert(BUFFER_SIZE > _HdmaIndirectTableOffset + 20)

constant height = _height
constant yPos = _lastScanline   // reuse variable, not used in setup

    //  hdmaBuffer = GetBuffer()
    //
    //  iTable = hdmaBuffer + _HdmaIndirectTableOffset
    //
    //  // store no empty line window offsets
    //  noLineWindow[0] = 0xff
    //  noLineWindow[1] = 0
    //
    //  if nLines > Camera.DISPLAY_HEIGHT:
    //      nLines = Camera.DISPLAY_HEIGHT
    //
    //  if yPos < 0:
    //      skip = -yPos
    //      if skip > height:
    //          goto NotVisible
    //  else:
    //      skip = 0
    //
    //      if yPos > DISPLAY_HEIGHT:
    //          goto NotVisible
    //
    //      if yPos != 0:
    //          ySkip = yPos
    //          if ySkip > 0x7f
    //              iTable[0] = ySkip - 0x7f
    //              iTable[1 + 2] = noLineWindow
    //              iTable += 3
    //              ySkip = 0x7f
    //
    //          iTable[0] = ySkip
    //          iTable[1 + 2] = noLineWindow
    //          iTable += 3
    //
    //
    //  if height + yPos >= DISPLAY_HEIGHT:
    //      nLines = DISPLAY_HEIGHT - yPos - skip
    //  else:
    //      nLines = height - skip
    //
    //
    //  if nLines > 0x7f:
    //      iTable[0] = 0xff
    //      iTable[1 + 2] = hdmaBuffer
    //      iTable[3] = nLines + 1 // == (nLines - 0x7f) | 0x80
    //      iTable[4 + 5] = hdmaBuffer + 0x7f * 2
    //      iTable += 6
    //  else:
    //      iTable[0] = nLines | 0x80
    //      iTable[1 + 2] = hdmaBuffer
    //      iTable += 3
    //
    //  iTable[0] = 1
    //  iTable[1 + 2] = noLineWindow
    //  iTable[4] = 0
    //
    //  return hdmaBuffer, true
    //

    sep     #$20
a8()
    jsr     GetBuffer
    rep     #$20
a16()
    stx.b   _hdmaBuffer
    txy

    lda.w   #0x00ff
    sta.w   noLineWindow


    lda.b   nLines
    cmp.w   #Camera.DISPLAY_HEIGHT + 1
    bcc     +
        lda.w   #Camera.DISPLAY_HEIGHT
    +


    // X = hdmaBuffer
    // Y = indirect table offset
    // KEEP: X/Y

    lda.b   yPos
    bpl     TopLineInsideScreen
        // Top line above screen
        eor.w   #0xffff
        inc

        cmp.b   height
        bcs     NotVisible

        sta.b   skip
        bra     End_TopVisibleIf


    TopLineInsideScreen:
        stz.b   skip
        beq     End_TopVisibleIf

        cmp.w   #Camera.DISPLAY_HEIGHT
        bcs     NotVisible

        cmp.w   #0x7f + 1
        bcc     +
            // c set
            sbc.w   #0x7f
            sta.w   _HdmaIndirectTableOffset + 0,y
            lda.w   #noLineWindow
            sta.w   _HdmaIndirectTableOffset + 1,y
            iny
            iny
            iny

            lda.w   #0x7f
        +
        sta.w   _HdmaIndirectTableOffset + 0,y
        lda.w   #noLineWindow
        sta.w   _HdmaIndirectTableOffset + 1,y
        iny
        iny
        iny
    End_TopVisibleIf:


    // height test

    lda.b   height
    clc
    adc.b   yPos
    cmp.w   #Camera.DISPLAY_HEIGHT
    bcc     +
        lda.w   #Camera.DISPLAY_HEIGHT
        sec
        sbc.b   yPos
        bra     ++
    +
        lda.b   height
    +
    sec
    sbc.b   skip
    sta.b   nLines


    // A = nLines
    // X = hdmaBuffer
    // Y = indirect table offset

    cmp.w   #0x7f + 1
    bcc     +
        inc
        sta.w   _HdmaIndirectTableOffset + 3,y

        lda.w   #0xff
        sta.w   _HdmaIndirectTableOffset + 0,y

        txa
        sta.w   _HdmaIndirectTableOffset + 1,y
        clc
        adc.w   #0x7f * 2
        sta.w   _HdmaIndirectTableOffset + 4,y

        iny
        iny
        iny

        bra     ++
    +
        // else nLines <= 0x7f
        ora.w   #0x80
        sta.w   _HdmaIndirectTableOffset + 0,y
        txa
        sta.w   _HdmaIndirectTableOffset + 1,y
    +

    lda.w   #1
    sta.w   _HdmaIndirectTableOffset + 3,y
    lda.w   #noLineWindow
    sta.w   _HdmaIndirectTableOffset + 4,y
    lda.w   #0
    sta.w   _HdmaIndirectTableOffset + 6,y

    sec
    rts
}


// Finalize and enable the HDMA buffer on the next VBlank.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
a16()
i16()
code()
function FinalizeBuffer {
    // Setup HDMA Channel

    lda.w   _hdmaBuffer
    clc
    adc.w   #_HdmaIndirectTableOffset
    sta.w   ChannelSettings.tableAddr

    assert(ChannelSettings.dmap + 1 == ChannelSettings.register)

    lda.w   #DMAP.direction.toPpu | DMAP.addressing.indirect | DMAP.transfer.two | (WH0 << 8)
    sta.w   ChannelSettings.dmap

    lda.w   #1
    tsb.w   ChannelSettings.enabled

    rts
}


// Calculates the X position within a line for a given yPos
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: DrawLine.x0 - top x coordinate of the line
//        DrawLine.x1 - bottom x coordinate of the line
//        DrawLine.height - height of the line
//        Y = y pos within the line to calculate the xPos for
//
// KEEP: This routine does not modify the line coordinates
//
// LIMITS: height > 0
//         height < 256
//         Y < 256
//         abs(x0 - x1) * (Y + 1) + height / 2 < 0x10000
//
// OUTPUT: A - x coordinate within the line at Y
a16()
i16()
code()
function CalcLineXpos {
    //  dx = abs(x1 - x0)
    //
    //  err = dx * 2
    //  if err < height:
    //      err = height
    //
    //  err /= 2
    //  err += dx * skip
    //  div = err / height
    //
    //  if xPos < x1:
    //      return xPos + div
    //  else:
    //      return xPos - div

    lda.b   _x1
    sec
    sbc.b   _xPos
    bpl     +
        eor.w   #0xffff
        inc
    +
    tax

    asl
    // this MUST MATCH DrawLine
    // A = dx * 2
    cmp.b   _height
    bcs     +
        lda.b   _height
    +


    // Have to do the /2 here to minimize the risk of an unsigned
    // overflow in the multiplication
    lsr
    sta.b   _err


    sep     #$20
a8()
    tya
    txy
    jsr     Math.Multiply.u16y_u8a_u16y

    rep     #$20
a16()
    tya
    clc
    adc.b   _err

    sta.l   WRDIV

    sep     #$20
a8()
    lda.b   _height
    sta.l   WRDIVB      // Wait 16 cycles

    rep     #$30        // 3
a16()
    lda.b   _xPos       // 4
    cmp.b   _x1         // 4
    bpl     +           // 2
        clc             // 2
        adc.l   RDDIV   // 1
        rts

    +
        sec
        sbc.l   RDDIV
        rts
}



// Draws a single line to a single side of the HDMA window buffer.
//
// The HDMA window buffer MUST:
//  * contain both the left and right window registers
//
// ASSUMES: buffer is the one created by `SetupWindowBuffer` call
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: DrawLine.x0 - top x coordinate of the line
//        DrawLine.x1 - bottom x coordinate of the line
//        DrawLine.height - height of the line
//        Y - number of scanlines to skip
//        A - number of scanlines to draw
//        X - address of the HDMA window buffer
//
// OUTPUT: X - address of the HDMA window buffer after final scanline
//
// KEEP: height is not modified by this routine
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
    //  err = dx2
    //  if err < height:
    //      err = height
    //
    //  if skip != 0:
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

    // err MUST MATCH CalcLineXpos
    // A = _dx2
    cmp.b   _height
    bcs     +
        lda.b   _height
    +
    sta.b   _err

    tya
    beq     +
        clc
        adc.b   _lastScanline
        sta.b   _lastScanline
        stx.b   _tmp

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

        ldx.b   _tmp
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
// INPUT: X - address of the HDMA window buffer
// OUTPUT: X - address of the HDMA window buffer after final scanline
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
    //      xClipped = xPos
    //      if xClipped < 0:
    //          xClipped = 0
    //          if sx < 0: goto EarlyExit
    //      else if xClipped > 255:
    //          xClipped = 255
    //          if sx > 0: goto EarlyExit
    //
    //      repeat h times:
    //          if buffer >= bufferLast: goto EndLoop
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
    //
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
                if {sx} < 0 {
                    bra     EarlyExit
                } else {
                    bra     ++
                }
            +
                cmp.w   #256
                bcc     +
                    lda.w   #255
                    if {sx} > 0 {
                        bra     EarlyExit
                    }
            +

            sep     #$20
        a8()
            -
                cpx.b   _bufferLast
                bcs     EndLoop

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


// xClipped is outside the screen and will never re-enter it.
//
// A = xClipped
// X = buffer address
a16()
i16()
EarlyExit:
    //  while buffer < bufferLast:
    //      *buffer = xClipped
    //      buffer += 2
    //
    //  *buffer = xClipped
    //  buffer += 2
    //
    //  return buffer

    sep     #$20
a8()
    cpx.b   _bufferLast
    bcs     +
    -
        sta.w   0,x
        inx
        inx
        cpx.b   _bufferLast
        bcc     -
    +

    sta.w   0,x
    inx
    inx

    rep     #$30
    rts
}



// Draw a shallow (dx2 >= dy2) Line to the HDMA Buffer
//
// INTERNAL: called by DrawLine
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X - address of the HDMA window buffer
// OUTPUT: X - address of the HDMA window buffer after final scanline
a16()
i16()
code()
function _DrawShallowLine {

constant EarlyExit = _DrawSteepLine.EarlyExit

    //  sx = x1 >= xPos ? 1 : -1
    //
    //  while True:
    //      if buffer >= bufferLast: goto EndLoop
    //
    //      xClipped = xPos
    //      if xClipped < 0:
    //          xClipped = 0
    //          if sx < 0: goto EarlyExit
    //      else if xClipped > 255:
    //          xClipped = 255
    //          if sx > 0: goto EarlyExit
    //
    //      *buffer = xClipped
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
    //  buffer += 2;
    //
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

            cpx.b   _bufferLast
            bcs     EndLoop

            // clip xPos to between 0 and 255
            lda.b   _xPos
            bpl     +
                lda.w   #0
                if {sx} < 0 {
                    bra     EarlyExit
                } else {
                    bra     ++
                }
            +
                cmp.w   #256
                bcc     +
                    lda.w   #255
                    if {sx} > 0 {
                        bra     EarlyExit
                    }
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

