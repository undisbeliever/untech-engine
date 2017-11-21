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
// REQUIRES: all dpTmp variables unused
//
// INPUT:
//      yPos, height, topLeft, topRight, bottomLeft, bottomRight - trapezium
//          (in dpTmp, may be modified by this routine)
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
constant yPos =         _WindowLine.SetupWindowBuffer.yPos
constant height =       _WindowLine.SetupWindowBuffer.height
constant topLeft =      dpTmp0
constant topRight =     dpTmp1
constant bottomLeft =   dpTmp2
constant bottomRight =  dpTmp3

constant _bufferPos =   dpTmp4

assert(_bufferPos < _WindowLine.LAST_DP_TMP)

constant _skip = _WindowLine.skip
constant _nLines = _WindowLine.nLines

    //  bufferPos, skip, nLines = SetupWindowBuffer(height, yPos)
    //
    //  x = _WindowLine.DrawLine(topLeft, bottomLeft, height, skip, nLines, buffer)
    //  x = _WindowLine.DrawLine(topRight, bottomRight, height, skip, nLines, buffer + 1)
    //  finalize(x - 1)

    jsr     _WindowLine.SetupWindowBuffer
    bcc     NotVisible

    stx.b   _bufferPos

    // line height is shadowed by _WindowLine.SetupWindowBuffer.height

    lda.b   topLeft
    sta.b   _WindowLine.DrawLine.x0
    lda.b   bottomLeft
    sta.b   _WindowLine.DrawLine.x1
    ldy.b   _skip
    lda.b   _nLines
    // X = bufferPos
    jsr     _WindowLine.DrawLine

    lda.b   topRight
    sta.b   _WindowLine.DrawLine.x0
    lda.b   bottomRight
    sta.b   _WindowLine.DrawLine.x1
    ldy.b   _skip
    lda.b   _nLines
    ldx.b   _bufferPos
    inx
    jsr     _WindowLine.DrawLine
    dex

NotVisible:
    jmp     _WindowLine.FinalizeBuffer
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

