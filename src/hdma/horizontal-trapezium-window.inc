// hdma/horizontal-trapezium-window.inc
// ====================================
//
// HDMA Table generator for rendering a trapezium with a horizontal base
// to the screen window.
//
//
// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


namespace Hdma {

// Builds and enables the HdmaTable for a trapezium with a horizontal base
//
// NOTE: Only checks if the window is visible in the Y axis
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// REQUIRES: all utZpTmp variables unused
//
// INPUT:
//      yPos, height, topLeft, topRight, bottomLeft, bottomRight - trapezium
//          (in utZpTmp, may be modified by this routine)
//
// LIMITS: topLeft <= topRight
//         bottomLeft <= bottomRight
//         height >= 1
//         height < 256
a16()
i16()
farCode()
function HorizontalTrapeziumWindow {
// trapezium coordinates
// (6 * sint16)
constant yPos =         _WindowBuffer.SetupWindowBuffer.yPos
constant height =       _WindowBuffer.SetupWindowBuffer.height
constant topLeft =      utZpTmp0
constant topRight =     utZpTmp1
constant bottomLeft =   utZpTmp2
constant bottomRight =  utZpTmp3

constant _bufferPos =   utZpTmp4

assert(_bufferPos < _WindowBuffer.LAST_ZP_TMP)

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

