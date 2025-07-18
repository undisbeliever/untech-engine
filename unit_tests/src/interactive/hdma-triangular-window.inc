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

namespace InteractiveTests {
namespace HdmaTriangularWindowTest {

// Tests:
//  * Hdma.TriangularWindow


constant x0 = Test.zpTmp + 0
constant y0 = Test.zpTmp + 2
constant x1 = Test.zpTmp + 4
constant y1 = Test.zpTmp + 6
constant x2 = Test.zpTmp + 8
constant y2 = Test.zpTmp + 10

constant cursor = Test.zpTmp + 12

constant FIRST_POINT_X = x0
constant FIRST_POINT_Y = y0
constant N_POINTS = 3


// Prints the positions on the screen
macro PrintWindowInput() {
    assert16a()
    assert16i()

    constant textXPos = 0
    constant textYPos = 4

    macro PrintVar(line, cursorId, name) {
        Text.Console.SetCursor(textXPos, textYPos + {line})

        ldx.b   cursor
        cpx.w   #{cursorId}
        bne     +
            ldx.w   #CursorString
            bra     ++
        +
            ldx.w   #NoCursorString
        +
        lda.w   #CursorString >> 16
        jsr     Text.Console.PrintString

        Text.Console.PrintConstString("{name}:")

        Text.Console.SetCursor(textXPos + 15, textYPos + {line})
        ldy.b   {name}
        jsr     Text.Console.PrintHex16Y
    }
    PrintVar(0, 0, x0)
    PrintVar(1, 0, y0)
    PrintVar(3, 1, x1)
    PrintVar(4, 1, y1)
    PrintVar(6, 2, x2)
    PrintVar(7, 2, y2)
}

rodata(rom0)
CursorString:
    db  " >", 0

NoCursorString:
    db  "  ", 0


// Update the variables with the controller
macro UpdateVariables() {
    assert16a()
    assert16i()

    ldx.b   cursor

    lda.w   Controller.Joy1.pressed
    and.w   #JOY.shoulder
    beq     CursorUnchanged

        and.w   #JOY.r
        beq     LPressed
            // R Pressed
            inx
            cpx.w   #N_POINTS
            bcc     +
                ldx.w   #0
            +
            bra     StoreCursor

        LPressed:
            dex
            bpl     +
                ldx.w   #N_POINTS
            +
    StoreCursor:
        stx.w   cursor

    CursorUnchanged:

    // X = cursor
    txa
    asl
    asl
    tax

    lda.w   Controller.Joy1.current
    and.w   #JOY.left
    beq     TestRight
        lda.b   FIRST_POINT_X,x
        dec
        cmp.w   #-16
        bmi     +
            sta.b   FIRST_POINT_X,x
        +

TestRight:
    lda.w   Controller.Joy1.current
    and.w   #JOY.right
    beq     TestUp
        lda.b   FIRST_POINT_X,x
        inc
        cmp.w   #Camera.DISPLAY_WIDTH + 16
        bpl     +
            sta.b   FIRST_POINT_X,x
        +

TestUp:
    lda.w   Controller.Joy1.current
    and.w   #JOY.up
    beq     TestDown
        lda.b   FIRST_POINT_Y,x
        dec
        cmp.w   #-16
        bmi     +
            sta.b   FIRST_POINT_Y,x
        +

TestDown:
    lda.w   Controller.Joy1.current
    and.w   #JOY.down
    beq     EndTests
        lda.b   FIRST_POINT_Y,x
        inc
        cmp.w   #Camera.DISPLAY_HEIGHT + 16
        bpl     +
            sta.b   FIRST_POINT_Y,x
        +

EndTests:
}


// The Test
a16()
i16()
code()
Test.add("HDMA Triangular Window Test")
function Test {
    macro SetVar(name, val) {
        lda.w   #{val}
        sta.b   {name}
    }

    // These value represent a former bug that occurred when the middle point
    // is 0x7e pixels below the top point.
    SetVar(x0, 0x22)
    SetVar(y0, 0x9d)
    SetVar(x1, 0x80)
    SetVar(y1, 0x1f)
    SetVar(x2, 0xdd)
    SetVar(y2, 0xb6)

    SetVar(cursor, 0)


    Text.Console.SetCursor(0, 21)
    Text.Console.PrintString(UnitTestMessage)


    // Setup Screen and color window
    // -----------------------------
    sep     #$20
a8()

    pea     (0x7e << 8) | REGISTER_DB
    plb

    jsl     ForceBlank__far

    lda.b   #(WSEL.win1.enable | WSEL.win1.outside) << WOBJSEL.color.shift
    sta.w   WOBJSEL


    lda.b   #CGWSEL.clip.outside | CGWSEL.prevent.inside
    sta.w   CGWSEL

    lda.b   #CGADSUB.color.subtract | CGADSUB.enable.backdrop
    sta.w   CGADSUB


    lda.b   #COLDATA.plane.all | (24 & COLDATA.intensity.mask)
    sta.w   COLDATA
    lda.b   #COLDATA.plane.red | (6 & COLDATA.intensity.mask)
    sta.w   COLDATA

    jsr     EnableDisplay_Full

    plb

    Hdma.Init()

    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        UpdateVariables()

        PrintWindowInput()

        jsl     _DrawHdma__far


        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Hdma.Deactivate()

    jmp     Test.ResetPpuState
}


a16()
i16()
farCode()
function _DrawHdma__far {
    macro CopyVar(name) {
        lda.b   {name}
        sta.b   Hdma.TriangularWindow.{name}
    }
    CopyVar(x0)
    CopyVar(y0)
    CopyVar(x1)
    CopyVar(y1)
    CopyVar(x2)
    CopyVar(y2)
    jsr     Hdma.TriangularWindow

    rtl
}


rodata(rom0)
UnitTestMessage:
    db  "Use L / R to select variable"
    db  "Use D-PAD to adjust value\n", 0
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

