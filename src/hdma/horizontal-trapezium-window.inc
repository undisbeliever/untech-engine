// hdma/horizontal-trapezium-window.inc
// ====================================
//
// HDMA Table generator for rendering a trapezium with a horizontal base
// to the screen window.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Hdma {

namespace HorizontalTrapeziumWindow {
NotVisible:
    // X = _hdmaBuffer
    jmp     AddHdmaTerminator
}

// Builds and enables the HdmaTable for a trapezium with a horizontal base
//
// NOTE: Only checks if the window is visible in the Y axis
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: all dpTmp variables unused
//
// INPUT:
//      yPos, height, topLeft, topRight, bottomLeft, bottomRight - trapezium (in dpTmp)
//
// LIMITS: topLeft <= topRight
//         bottomLeft <= bottomRight
//         height >= 1
//         yPos > -256
a16()
i16()
code()
function HorizontalTrapeziumWindow {
// trapezium coordinates
// (6 * sint16)
constant yPos =         dpTmp0
constant height =       dpTmp1
constant topLeft =      dpTmp2
constant topRight =     dpTmp3
constant bottomLeft =   dpTmp4
constant bottomRight =  dpTmp5

constant _skip =        dpTmp6
constant _hdmaBuffer =  dpTmp7
constant _count =       dpTmp8
constant _bufferPos =   dpTmp9

assert(_bufferPos < _WindowLine.LAST_DP_TMP)

assert(BUFFER_SIZE >= 256 * 2 + 3)

    sep     #$20
a8()
    jsr     GetBuffer
    rep     #$20
a16()
    stx.b   _hdmaBuffer

    //  if yPos < 0:
    //      skip = -yPos
    //      if skip > height: goto NotVisible
    //
    //  else:
    //      skip = 0
    //      if yPos > DISPLAY_HEIGHT: goto NotVisible
    //
    //      if yPos != 0:
    //          if yPos > 0x7f
    //              buffer[0] = yPos - 0x7f
    //              buffer[1] = 0xff
    //              buffer[2] = 0
    //              buffer[0] = 0x7f
    //              buffer[1] = 0xff
    //              buffer[2] = 0
    //              buffer += 6
    //          buffer[0] = yPos
    //          buffer[1] = 0xff
    //          buffer[2] = 0
    //          buffer += 3

    lda.b   yPos
    bpl     TopLineInsideScreen
        // Top line above screen
        eor.w   #0xffff
        inc

        cmp.b   height
        bcs     NotVisible

        sta.b   _skip
        bra     End_TopVisibleIf

    TopLineInsideScreen:
        stz.b   _skip
        beq     End_TopVisibleIf

        cmp.w   #Camera.DISPLAY_HEIGHT
        bcs     NotVisible

        cmp.w   #0x7f + 1
        bcc     +
            sec
            sbc.w   #0x7f
            sta.w   0,x
            lda.w   #0xff
            sta.w   1,x
            inx
            inx
            inx

            lda.w   #0x7f
        +
        sta.w   0,x
        lda.w   #0xff
        sta.w   1,x
        inx
        inx
        inx
    End_TopVisibleIf:


    //  if height + yPos >= DISPLAY_HEIGHT:
    //      count = DISPLAY_HEIGHT - yPos - skip
    //  else:
    //      count = height - skip

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
    sbc.b   _skip
    sta.b   _count


    // Setup HDMA count bytes

    //  if (count) > 0x7f:
    //      buffer[0x7f * 2 + 1] = 0x80 | (count - 0x7f)
    //      buffer[0] = 0x80 | 0x7f
    //  else:
    //      buffer[0] = 0x80 | count

    cmp.w   #0x7f + 1
    bcc     +
        tay
        sec
        sbc.w   #0x7f
        ora.w   #0x80
        sta.w   0x7f * 2 + 1,x

        lda.w   #0x7f
    +
    ora.w   #0x80
    sta.w   0,x
    inx
    stx.b   _bufferPos


    // Draw Left Line

    //  _WindowLine.SetupLine(topLeft, bottomLeft, height, skip)
    //  x = buffer
    //  if count > 0x7f:
    //      x = _WindowLine.DrawLine(0x7f, x)
    //      x = _WindowLine.DrawLine(c - 0x7f, x + 1)
    //  else:
    //      x = _WindowLine.DrawLine(0x7f, x)

    lda.b   topLeft
    sta.b   _WindowLine.SetupLine.x0
    lda.b   bottomLeft
    sta.b   _WindowLine.SetupLine.x1
    lda.b   height
    ldx.b   _skip
    jsr     _WindowLine.SetupLine

    ldx.b   _bufferPos
    lda.b   _count
    cmp.w   #0x7f + 1
    bcc     +
        lda.w   #0x7f
        jsr     _WindowLine.DrawLine
        lda.b   _count
        sec
        sbc.w   #0x7f
        inx
    +
    jsr     _WindowLine.DrawLine


    // Draw Right Line

    //  _WindowLine.SetupLine(topRight, bottomRight, height, skip)
    //  x = buffer + 1
    //  if count > 0x7f:
    //      x = _WindowLine.DrawLine(0x7f, x)
    //      x = _WindowLine.DrawLine(c - 0x7f, x + 1)
    //  else:
    //      x = _WindowLine.DrawLine(0x7f, x)
    //  buffer = x - 1

    lda.b   topRight
    sta.b   _WindowLine.SetupLine.x0
    lda.b   bottomRight
    sta.b   _WindowLine.SetupLine.x1
    lda.b   height
    ldx.b   _skip
    jsr     _WindowLine.SetupLine

    ldx.b   _bufferPos
    inx
    lda.b   _count
    cmp.w   #0x7f + 1
    bcc     +
        lda.w   #0x7f
        jsr     _WindowLine.DrawLine
        lda.b   _count
        sec
        sbc.w   #0x7f
        inx
    +
    jsr     _WindowLine.DrawLine

    dex


    // Add terminator to HDMA buffer
    //
    //  x = bufferPos
    //
    //  buffer[0] = 1
    //  buffer[1] = 0xff
    //  buffer[2] = 0
    //  buffer[3] = 0

AddHdmaTerminator:
    lda.w   #0xff01
    sta.w   0,x
    stz.w   2,x


    // Setup HDMA Channel
    ldx.w   _hdmaBuffer
    sep     #$20
a8()
    stx.w   ChannelSettings.tableAddr

    assert(ChannelSettings.dmap + 1 == ChannelSettings.register)

    ldx.w   #DMAP.direction.toPpu | DMAP.addressing.absolute | DMAP.transfer.two | (WH0 << 8)
    stx.w   ChannelSettings.dmap

    lda.b   #1
    sta.w   ChannelSettings.enabled

    rep     #$30
a16()
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

