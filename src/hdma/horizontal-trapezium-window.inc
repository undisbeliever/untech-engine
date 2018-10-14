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

// Builds and enables the HdmaTable for a trapezium with a horizontal base
//
// NOTE: Only checks if the window is visible in the Y axis
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: all utDpTmp variables unused
//
// INPUT:
//      yPos, height, topLeft, topRight, bottomLeft, bottomRight - trapezium
//          (in utDpTmp, may be modified by this routine)
//
// LIMITS: topLeft <= topRight
//         bottomLeft <= bottomRight
//         height >= 1
//         height < 256
a16()
i16()
code()
function HorizontalTrapeziumWindow {
// trapezium coordinates
// (6 * sint16)
constant yPos =         _WindowBuffer.SetupWindowBuffer.yPos
constant height =       _WindowBuffer.SetupWindowBuffer.height
constant topLeft =      utDpTmp0
constant topRight =     utDpTmp1
constant bottomLeft =   utDpTmp2
constant bottomRight =  utDpTmp3

constant _bufferPos =   utDpTmp4

assert(_bufferPos < _WindowBuffer.LAST_DP_TMP)

constant _skip = _WindowBuffer.skip
constant _nLines = _WindowBuffer.nLines

    //  bufferPos, skip, nLines = SetupWindowBuffer(height, yPos)
    //
    //  x = _WindowBuffer.DrawLine(topLeft, bottomLeft, height, skip, nLines, buffer)
    //  x = _WindowBuffer.DrawLine(topRight, bottomRight, height, skip, nLines, buffer + 1)
    //  finalize(x - 1)

    jsr     _WindowBuffer.SetupWindowBuffer
    bcc     NotVisible

    stx.b   _bufferPos

    // line height is shadowed by _WindowBuffer.SetupWindowBuffer.height

    lda.b   topLeft
    sta.b   _WindowBuffer.DrawLine.x0
    lda.b   bottomLeft
    sta.b   _WindowBuffer.DrawLine.x1
    ldy.b   _skip
    lda.b   _nLines
    // X = bufferPos
    jsr     _WindowBuffer.DrawLine

    lda.b   topRight
    sta.b   _WindowBuffer.DrawLine.x0
    lda.b   bottomRight
    sta.b   _WindowBuffer.DrawLine.x1
    ldy.b   _skip
    lda.b   _nLines
    ldx.b   _bufferPos
    inx
    jsr     _WindowBuffer.DrawLine
    dex

NotVisible:
    jmp     _WindowBuffer.FinalizeBuffer
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

